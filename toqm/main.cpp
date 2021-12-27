//#include "QASMparser.h"

#include <Toqm.hpp>
#include <myParser.hpp>
#include <Node.hpp>
#include <CostFunc/CXFrontier.hpp>
#include <CostFunc/CXFull.hpp>
#include <CostFunc/SimpleCost.hpp>
#include <Expander/DefaultExpander.hpp>
#include <Expander/GreedyTopK.hpp>
#include <Expander/NoSwaps.hpp>
#include <Filter/HashFilter.hpp>
#include <Filter/HashFilter2.hpp>
#include <Latency/Latency_1.hpp>
#include <Latency/Latency_1_2_6.hpp>
#include <Latency/Latency_1_3.hpp>
#include <Latency/Table.hpp>
#include <NodeMod/GreedyMapper.hpp>
#include <Queue/DefaultQueue.hpp>
#include <Queue/TrimSlowNodes.hpp>

#include <cassert>
#include <iostream>
#include <vector>

using namespace std;

namespace toqm {
bool _verbose = false;
}

const int NUMCOSTFUNCTIONS = 3;
tuple<toqm::CostFunc*, string, string> costFunctions[NUMCOSTFUNCTIONS] = {
        make_tuple(new toqm::CXFrontier(),
                   "CXFrontier",
                   "Calculates lower-bound cost, including swaps to enable gates in the CX frontier"),
        make_tuple(new toqm::CXFull(),
                   "CXFull",
                   "Calculates lower-bound cost, including swaps to enable CX gates in remaining circuit"),
        make_tuple(new toqm::SimpleCost(),
                   "SimpleCost",
                   "Calculates lower-bound cost, assuming no more swaps will be inserted"),
};

const int NUMEXPANDERS = 3;
tuple<toqm::Expander*, string, string> expanders[NUMEXPANDERS] = {
        make_tuple(new toqm::DefaultExpander(),
                   "DefaultExpander",
                   "The default expander. Includes acyclic swap and dependent state optimizations."),
        make_tuple(new toqm::GreedyTopK(),
                   "GreedyTopK",
                   "Keep only top K nodes and schedule original gates ASAP [non-optimal!]"),
        make_tuple(new toqm::NoSwaps(),
                   "NoSwaps",
                   "An expander that tries various possible initial mappings, and cannot insert swaps."),
};

const int NUMFILTERS = 2;
tuple<toqm::Filter*, string, string> FILTERS[NUMFILTERS] = {
        make_tuple(new toqm::HashFilter(),
                   "HashFilter",
                   "using hash, this tries to filter out worse nodes."),
        make_tuple(new toqm::HashFilter2(),
                   "HashFilter2",
                   "using hash, this tries to filter out worse nodes, or mark old nodes as dead if a new node is strictly-better."),
};

const int NUMLATENCIES = 4;
tuple<toqm::Latency*, string, string> latencies[NUMLATENCIES] = {
        make_tuple(new toqm::Latency_1_2_6(),
                   "Latency_1_2_6",
                   "swap cost 6, 2-bit gate cost 2, 1-bit gate cost 1."),
        make_tuple(new toqm::Latency_1_3(),
                   "Latency_1_3",
                   "swap cost 3, all else cost 1."),
        make_tuple(new toqm::Latency_1(),
                   "Latency_1",
                   "every gate takes 1 cycle."),
        make_tuple(new toqm::Table(),
                   "Table",
                   "gets latencies from specified latency-table file"),
};

const int NUMNODEMODS = 1;
tuple<toqm::NodeMod*, string, string> nodeMods[NUMNODEMODS] = {
        make_tuple(new toqm::GreedyMapper(),
                   "GreedyMapper",
                   "Deletes default initial mapping, and greedily maps qubits in ready CX gates"),
};

const int NUMQUEUES = 2;
tuple<toqm::Queue*, string, string> queues[NUMQUEUES] = {
        make_tuple(new toqm::DefaultQueue(),
                   "DefaultQueue",
                   "uses std priority_queue."),
        make_tuple(new toqm::TrimSlowNodes(),
                   "TrimSlowNodes",
                   "Takes 2 params; when reaching max # nodes it removes slowest until it reaches target # nodes."),
};

//string comparison
int caseInsensitiveCompare(const char * c1, const char * c2) {
	for(int x = 0;; x++) {
		if(toupper(c1[x]) == toupper(c2[x])) {
			if(!c1[x]) {
				return 0;
			}
		} else {
			return toupper(c1[x]) - toupper(c2[x]);
		}
	}
}

//string comparison
int caseInsensitiveCompare(string str, const char * c2) {
	const char * c1 = str.c_str();
	return caseInsensitiveCompare(c1, c2);
}
int caseInsensitiveCompare(const char * c1, string str) {
	const char * c2 = str.c_str();
	return caseInsensitiveCompare(c1, c2);
}

int main(int argc, char** argv) {
	char * qasmFileName = NULL;
	char * couplingMapFileName = NULL;

    toqm::Expander * ex = NULL;
    toqm::CostFunc * cf = NULL;
    toqm::Latency * lat = NULL;
    toqm::Queue * nodes = NULL;
    vector<toqm::NodeMod *> mods {};
    vector<toqm::Filter *> filters {};

	unsigned int retainPopped = 0;
	
	int choice = -1;
	//bool printNumQubitsAndQuit = false;
	
	//variables used to indicate we'll spend some cycles searching for initial mapping:
	int initialSearchCycles = 0;
	
	//variables for user-specified initial mapping:
	char init_qal[toqm::MAX_QUBITS];
	char init_laq[toqm::MAX_QUBITS];
	int use_specified_init_mapping = 0;
	
	//Parse command-line arguments:
	for(int iter = 1; iter < argc; iter++) {
		if(!caseInsensitiveCompare(argv[iter], "-retain") || !caseInsensitiveCompare(argv[iter], "-retainPopped")) {
			retainPopped = atoi(argv[++iter]);
		} else if(!caseInsensitiveCompare(argv[iter], "-qal")) {
			++iter;
			use_specified_init_mapping = 1;
			int c = 0;
			int i = 0;
			while(argv[iter][c]) {
				if(argv[iter][c] < '0' || argv[iter][c] > '9') {
					if(argv[iter][c] != '-') {
						c++;
						continue;
					}
				}
				init_qal[i++] = atoi(&(argv[iter][c]));
			}
			for(; i < toqm::MAX_QUBITS; i++) {
				init_qal[i] = -1;
			}
		} else if(!caseInsensitiveCompare(argv[iter], "-laq")) {
			++iter;
			use_specified_init_mapping = 2;
			int c = 0;
			int i = 0;
			while(argv[iter][c]) {
				if(argv[iter][c] < '0' || argv[iter][c] > '9') {
					if(argv[iter][c] != '-') {
						c++;
						continue;
					}
				}
				init_laq[i++] = atoi(&(argv[iter][c]));
			}
			for(; i < toqm::MAX_QUBITS; i++) {
				init_laq[i] = -1;
			}
		} else if(!caseInsensitiveCompare(argv[iter], "-default") || !caseInsensitiveCompare(argv[iter], "-defaults")) {
			if(!ex) ex = std::get<0>(expanders[0]);
			if(!cf) cf = std::get<0>(costFunctions[0]);
			if(!lat) lat = std::get<0>(latencies[0]);
			if(!nodes) nodes = std::get<0>(queues[0]);
		} else if(!caseInsensitiveCompare(argv[iter], "-expander")) {
			char * choiceStr = argv[++iter];
			bool found = false;
			for(int x = 0; x < NUMEXPANDERS; x++) {
				if(!caseInsensitiveCompare(std::get<1>(expanders[x]), choiceStr)) {
					found = true;
					ex = std::get<0>(expanders[x]);
					iter += ex->setArgs(argv + (iter+1));
					break;
				}
			}
			assert(found);
		} else if(!caseInsensitiveCompare(argv[iter], "-pureSwapDiameter") || !caseInsensitiveCompare(argv[iter], "-rewindD")) {
			initialSearchCycles = -1;//a later part of the code detects the nonsensical -1 value and sets this appropriately
		} else if(!caseInsensitiveCompare(argv[iter], "-pureSwaps") || !caseInsensitiveCompare(argv[iter], "-rewindCycles")) {
			char * choiceStr = argv[++iter];
			initialSearchCycles = atoi(choiceStr);
		} else if(!caseInsensitiveCompare(argv[iter], "-nodemod")) {
			char * choiceStr = argv[++iter];
			bool found = false;
			for(int x = 0; x < NUMNODEMODS; x++) {
				if(!caseInsensitiveCompare(std::get<1>(nodeMods[x]), choiceStr)) {
					found = true;
                    toqm::NodeMod * nm = std::get<0>(nodeMods[x]);
					mods.push_back(nm);
					iter += nm->setArgs(argv + (iter+1));
					break;
				}
			}
			assert(found);
		} else if(!caseInsensitiveCompare(argv[iter], "-costfunction") || !caseInsensitiveCompare(argv[iter], "-costfunc") || !caseInsensitiveCompare(argv[iter], "-cost")) {
			char * choiceStr = argv[++iter];
			bool found = false;
			for(int x = 0; x < NUMCOSTFUNCTIONS; x++) {
				if(!caseInsensitiveCompare(std::get<1>(costFunctions[x]), choiceStr)) {
					found = true;
					cf = std::get<0>(costFunctions[x]);
					iter += cf->setArgs(argv + (iter+1));
					break;
				}
			}
			assert(found);
		} else if(!caseInsensitiveCompare(argv[iter], "-latency")) {
			char * choiceStr = argv[++iter];
			bool found = false;
			for(int x = 0; x < NUMLATENCIES; x++) {
				if(!caseInsensitiveCompare(std::get<1>(latencies[x]), choiceStr)) {
					found = true;
					lat = std::get<0>(latencies[x]);
					iter += lat->setArgs(argv + (iter+1));
					break;
				}
			}
			assert(found);
		} else if(!caseInsensitiveCompare(argv[iter], "-filter")) {
			char * choiceStr = argv[++iter];
			bool found = false;
			for(int x = 0; x < NUMFILTERS; x++) {
				if(!caseInsensitiveCompare(std::get<1>(FILTERS[x]), choiceStr)) {
					found = true;
                    toqm::Filter * fil = std::get<0>(FILTERS[x]);
					filters.push_back(fil);
					iter += fil->setArgs(argv + (iter+1));
					break;
				}
			}
			assert(found);
		} else if(!caseInsensitiveCompare(argv[iter], "-queue")) {
			char * choiceStr = argv[++iter];
			bool found = false;
			for(int x = 0; x < NUMQUEUES; x++) {
				if(!caseInsensitiveCompare(std::get<1>(queues[x]), choiceStr)) {
					found = true;
					nodes = std::get<0>(queues[x]);
					iter += nodes->setArgs(argv + (iter+1));
					break;
				}
			}
			assert(found);
		} else if(!caseInsensitiveCompare(argv[iter], "-v")) {
            toqm::_verbose = true;
		} else if(!qasmFileName) {
			qasmFileName = argv[iter];
		} else if(!couplingMapFileName) {
			couplingMapFileName = argv[iter];
		} else {
			assert(false);
		}
	}
	
	bool userChoices = false;
	
	if(!ex) {
		userChoices = true;
		choice = -1;
		cerr << "Select an expander.\n";
		for(int x = 0; x < NUMEXPANDERS; x++) {
			cerr << " " << x << ": " << std::get<1>(expanders[x]) << ": " << std::get<2>(expanders[x]) << "\n";
		}
		cin >> choice;
		assert(choice >= 0 && choice < NUMEXPANDERS);
		ex = std::get<0>(expanders[choice]);
		ex->setArgs();
	}
	
	if(!cf) {
		userChoices = true;
		choice = -1;
		cerr << "Select a cost function.\n";
		for(int x = 0; x < NUMCOSTFUNCTIONS; x++) {
			cerr << " " << x << ": " << std::get<1>(costFunctions[x]) << ": " << std::get<2>(costFunctions[x]) << "\n";
		}
		cin >> choice;
		assert(choice >= 0 && choice < NUMCOSTFUNCTIONS);
		cf = std::get<0>(costFunctions[choice]);
		cf->setArgs();
	}
	
	if(!lat) {
		userChoices = true;
		choice = -1;
		cerr << "Select a latency setting.\n";
		for(int x = 0; x < NUMLATENCIES; x++) {
			cerr << " " << x << ": " << std::get<1>(latencies[x]) << ": " << std::get<2>(latencies[x]) << "\n";
		}
		cin >> choice;
		assert(choice >= 0 && choice < NUMLATENCIES);
		lat = std::get<0>(latencies[choice]);
		lat->setArgs();
	}
	
	if(!nodes) {
		userChoices = true;
		choice = -1;
		cerr << "Select a queue structure.\n";
		for(int x = 0; x < NUMQUEUES; x++) {
			cerr << " " << x << ": " << std::get<1>(queues[x]) << ": " << std::get<2>(queues[x]) << "\n";
		}
		cin >> choice;
		assert(choice >= 0 && choice < NUMQUEUES);
		nodes = std::get<0>(queues[choice]);
		nodes->setArgs();
	}
	
	if(userChoices) {
		int numselected;
		
		bool filtersOn[NUMFILTERS];
		for(int x = 0; x < NUMFILTERS; x++) {
			filtersOn[x] = false;
		}
		numselected = 0;
		while(numselected < NUMFILTERS) {
			choice = -1;
			cerr << "Select a filter, or -1 for no additional filters.\n";
			cerr << " " << -1 << ": " << "no more filters\n";
			for(int x = 0; x < NUMFILTERS; x++) {
				if(!filtersOn[x]) {
					if(x < 10) cerr << " ";
					cerr << " " << x << ": " << std::get<1>(FILTERS[x]) << ": " << std::get<2>(FILTERS[x]) << "\n";
				}
			}
			cin >> choice;
			if(choice < 0 || choice >= NUMFILTERS) {
				break;
			}
			if(!filtersOn[choice]) {
				numselected++;
				filtersOn[choice] = true;
                toqm::Filter * fil = std::get<0>(FILTERS[choice]);
				filters.push_back(fil);
				fil->setArgs();
			}
		}
		
		numselected = 0;
		bool nodeModsOn[NUMNODEMODS];
		for(int x = 0; x < NUMNODEMODS; x++) {
			nodeModsOn[x] = false;
		}
		while(numselected < NUMNODEMODS) {
			choice = -1;
			cerr << "Select a node modifier, or -1 for no additional mods.\n";
			cerr << " " << -1 << ": " << "no more node mods\n";
			for(int x = 0; x < NUMNODEMODS; x++) {
				if(!nodeModsOn[x]) {
					if(x < 10) cerr << " ";
					cerr << " " << x << ": " << std::get<1>(nodeMods[x]) << ": " << std::get<2>(nodeMods[x]) << "\n";
				}
			}
			cin >> choice;
			if(choice < 0 || choice >= NUMNODEMODS) {
				break;
			}
			if(!nodeModsOn[choice]) {
				numselected++;
				nodeModsOn[choice] = true;
                toqm::NodeMod * nm = std::get<0>(nodeMods[choice]);
				mods.push_back(nm);
				nm->setArgs();
			}
		}
	}

    toqm::run(qasmFileName,
        couplingMapFileName,
        ex,
        cf,
        lat,
        nodes,
        mods,
        filters,
        retainPopped,
        initialSearchCycles,
        use_specified_init_mapping,
        init_qal,
        init_laq);
	
	return 0;
}