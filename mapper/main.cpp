#include <libtoqm/ToqmMapper.hpp>
#include <libtoqm/QasmObject.hpp>
#include <libtoqm/CouplingMapParser.hpp>
#include <libtoqm/Node.hpp>
#include <libtoqm/CostFunc/CXFrontier.hpp>
#include <libtoqm/CostFunc/CXFull.hpp>
#include <libtoqm/CostFunc/SimpleCost.hpp>
#include <libtoqm/Expander/DefaultExpander.hpp>
#include <libtoqm/Expander/GreedyTopK.hpp>
#include <libtoqm/Expander/NoSwaps.hpp>
#include <libtoqm/Filter/HashFilter.hpp>
#include <libtoqm/Filter/HashFilter2.hpp>
#include <libtoqm/Latency/Latency_1.hpp>
#include <libtoqm/Latency/Latency_1_2_6.hpp>
#include <libtoqm/Latency/Latency_1_3.hpp>
#include <libtoqm/Latency/Table.hpp>
#include <libtoqm/NodeMod/GreedyMapper.hpp>
#include <libtoqm/Queue/DefaultQueue.hpp>
#include <libtoqm/Queue/TrimSlowNodes.hpp>

#include <cassert>
#include <iostream>
#include <memory>
#include <vector>
#include <functional>

using namespace std;

namespace toqm {
bool _verbose = false;
}

template<typename T>
using FactoryFunc = function<unique_ptr<T>()>;

template<typename T>
struct UserOption {
    string name;
    string description;
    function<FactoryFunc<T>()> fromStdin;
    function<FactoryFunc<T>(char**, int&)> fromArg;
};

template<typename TBase, typename TDerived>
UserOption<TBase> NoArgsOption(string name, string description) {
    return UserOption<TBase> {
        name,
        description,
        []() { return []() { return unique_ptr<TBase>(new TDerived());}; },
        [](char**, int&) { return []() { return unique_ptr<TBase>(new TDerived()); }; }
    };
}

const int NUMCOSTFUNCTIONS = 3;
UserOption<toqm::CostFunc> costFunctions[NUMCOSTFUNCTIONS] = {
    NoArgsOption<toqm::CostFunc, toqm::CXFrontier>(
        "CXFrontier",
        "Calculates lower-bound cost, including swaps to enable gates in the CX frontier"
    ),
    NoArgsOption<toqm::CostFunc, toqm::CXFull>(
        "CXFull",
        "Calculates lower-bound cost, including swaps to enable CX gates in remaining circuit"
    ),
    NoArgsOption<toqm::CostFunc, toqm::SimpleCost>(
        "SimpleCost",
        "Calculates lower-bound cost, assuming no more swaps will be inserted"
    ),
};

const int NUMEXPANDERS = 3;
UserOption<toqm::Expander> expanders[NUMEXPANDERS] = {
    NoArgsOption<toqm::Expander, toqm::DefaultExpander>(
        "DefaultExpander",
        "The default expander. Includes acyclic swap and dependent state optimizations."
    ),
    {
        "GreedyTopK",
        "Keep only top K nodes and schedule original gates ASAP [non-optimal!]",
        []() {
            unsigned int k;
            std::cerr << "Enter value of K for top-k expander: K=";
            std::cin >> k;

            return [=]() {
                return unique_ptr<toqm::Expander>(new toqm::GreedyTopK(k));
            };
        },
        [](char**argv, int& iter) {
            iter += 1;
            auto k = atoi(argv[0]);

            return [=]() {
                return unique_ptr<toqm::Expander>(new toqm::GreedyTopK(k));
            };
        },
    },
    NoArgsOption<toqm::Expander, toqm::NoSwaps>(
        "NoSwaps",
        "An expander that tries various possible initial mappings, and cannot insert swaps."
    ),
};

const int NUMFILTERS = 2;
UserOption<toqm::Filter> FILTERS[NUMFILTERS] = {
        NoArgsOption<toqm::Filter, toqm::HashFilter>(
            "HashFilter",
            "using hash, this tries to filter out worse nodes."
        ),
        NoArgsOption<toqm::Filter, toqm::HashFilter2>(
            "HashFilter2",
            "using hash, this tries to filter out worse nodes, or mark old nodes as dead if a new node is strictly-better."
        ),
};

const int NUMLATENCIES = 4;
UserOption<toqm::Latency> latencies[NUMLATENCIES] = {
    NoArgsOption<toqm::Latency, toqm::Latency_1_2_6>(
        "Latency_1_2_6",
        "swap cost 6, 2-bit gate cost 2, 1-bit gate cost 1."
    ),
    NoArgsOption<toqm::Latency, toqm::Latency_1_3>(
        "Latency_1_3",
        "swap cost 3, all else cost 1."
    ),
    NoArgsOption<toqm::Latency, toqm::Latency_1>(
        "Latency_1",
        "every gate takes 1 cycle."
    ),
    {
        "Table",
        "gets latencies from specified latency-table file",
        []() {
            string filename;
            std::cin >> filename;

            return [=]() {
                std::ifstream infile(filename);
                return unique_ptr<toqm::Latency>(new toqm::Table(infile));
            };
        },
        [](char**argv, int& iter) {
            iter += 1;
            char *filename = argv[0];

            return [=]() {
                std::ifstream infile(filename);
                return unique_ptr<toqm::Latency>(new toqm::Table(infile));
            };
        }
    }
};

const int NUMNODEMODS = 1;
UserOption<toqm::NodeMod> nodeMods[NUMNODEMODS] = {
        NoArgsOption<toqm::NodeMod, toqm::GreedyMapper>(
            "GreedyMapper",
            "Deletes default initial mapping, and greedily maps qubits in ready CX gates"
        ),
};

const int NUMQUEUES = 2;
UserOption<toqm::Queue> queues[NUMQUEUES] = {
        NoArgsOption<toqm::Queue, toqm::DefaultQueue>(
            "DefaultQueue",
            "uses std priority_queue."
        ),
        {
            "TrimSlowNodes",
            "Takes 2 params; when reaching max # nodes it removes slowest until it reaches target # nodes.",
            []() {
                unsigned int maxSize, targetSize;
                std::cerr << "Enter max size and then target size for queue:\n";
                std::cin >> maxSize;
                std::cin >> targetSize;

                return [=]() {
                    return unique_ptr<toqm::Queue>(new toqm::TrimSlowNodes(maxSize, targetSize));
                };
            },
            [](char**argv, int& iter) {
                iter += 2;
                unsigned int maxSize = atoi(argv[0]);
                unsigned int targetSize = atoi(argv[1]);

                return [=]() {
                    return unique_ptr<toqm::Queue>(new toqm::TrimSlowNodes(maxSize, targetSize));
                };
            },
        }
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

    FactoryFunc<toqm::Queue> nodes;
    unique_ptr<toqm::Expander> ex;
    unique_ptr<toqm::CostFunc> cf;
    unique_ptr<toqm::Latency> lat;
    vector<unique_ptr<toqm::NodeMod>> mods {};
    vector<unique_ptr<toqm::Filter>> filters {};

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
			if(!ex) ex = expanders[0].fromStdin()();
			if(!cf) cf = costFunctions[0].fromStdin()();
			if(!lat) lat = latencies[0].fromStdin()();
			if(!nodes) nodes = queues[0].fromStdin();
		} else if(!caseInsensitiveCompare(argv[iter], "-expander")) {
			char * choiceStr = argv[++iter];
			bool found = false;
			for(int x = 0; x < NUMEXPANDERS; x++) {
				if(!caseInsensitiveCompare(expanders[x].name, choiceStr)) {
					found = true;
                    ex = expanders->fromArg(argv + (iter+1), iter)();
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
				if(!caseInsensitiveCompare(nodeMods[x].name, choiceStr)) {
					found = true;
                    auto nm = nodeMods[x].fromArg(argv + (iter+1), iter)();
					mods.push_back(move(nm));
					break;
				}
			}
			assert(found);
		} else if(!caseInsensitiveCompare(argv[iter], "-costfunction") || !caseInsensitiveCompare(argv[iter], "-costfunc") || !caseInsensitiveCompare(argv[iter], "-cost")) {
			char * choiceStr = argv[++iter];
			bool found = false;
			for(int x = 0; x < NUMCOSTFUNCTIONS; x++) {
				if(!caseInsensitiveCompare(costFunctions[x].name, choiceStr)) {
					found = true;
                    cf = costFunctions[x].fromArg(argv + (iter+1), iter)();
					break;
				}
			}
			assert(found);
		} else if(!caseInsensitiveCompare(argv[iter], "-latency")) {
			char * choiceStr = argv[++iter];
			bool found = false;
			for(int x = 0; x < NUMLATENCIES; x++) {
				if(!caseInsensitiveCompare(latencies[x].name, choiceStr)) {
					found = true;
                    lat = latencies[x].fromArg(argv + (iter+1), iter)();
					break;
				}
			}
			assert(found);
		} else if(!caseInsensitiveCompare(argv[iter], "-filter")) {
			char * choiceStr = argv[++iter];
			bool found = false;
			for(int x = 0; x < NUMFILTERS; x++) {
				if(!caseInsensitiveCompare(FILTERS[x].name, choiceStr)) {
					found = true;
                    auto fil = FILTERS[x].fromArg(argv + (iter+1), iter)();
					filters.push_back(move(fil));
					break;
				}
			}
			assert(found);
		} else if(!caseInsensitiveCompare(argv[iter], "-queue")) {
			char * choiceStr = argv[++iter];
			bool found = false;
			for(int x = 0; x < NUMQUEUES; x++) {
				if(!caseInsensitiveCompare(queues[x].name, choiceStr)) {
					found = true;
                    nodes = queues[x].fromArg(argv + (iter+1), iter);
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
			cerr << " " << x << ": " << expanders[x].name << ": " << expanders[x].description << "\n";
		}
		cin >> choice;
		assert(choice >= 0 && choice < NUMEXPANDERS);
		ex = expanders[choice].fromStdin()();
	}
	
	if(!cf) {
		userChoices = true;
		choice = -1;
		cerr << "Select a cost function.\n";
		for(int x = 0; x < NUMCOSTFUNCTIONS; x++) {
			cerr << " " << x << ": " << costFunctions[x].name << ": " << costFunctions[x].description << "\n";
		}
		cin >> choice;
		assert(choice >= 0 && choice < NUMCOSTFUNCTIONS);
        cf = costFunctions[choice].fromStdin()();
	}
	
	if(!lat) {
		userChoices = true;
		choice = -1;
		cerr << "Select a latency setting.\n";
		for(int x = 0; x < NUMLATENCIES; x++) {
			cerr << " " << x << ": " << latencies[x].name << ": " << latencies[x].description << "\n";
		}
		cin >> choice;
		assert(choice >= 0 && choice < NUMLATENCIES);
        lat = latencies[choice].fromStdin()();
	}
	
	if(!nodes) {
		userChoices = true;
		choice = -1;
		cerr << "Select a queue structure.\n";
		for(int x = 0; x < NUMQUEUES; x++) {
			cerr << " " << x << ": " << queues[x].name << ": " << queues[x].description << "\n";
		}
		cin >> choice;
		assert(choice >= 0 && choice < NUMQUEUES);
		nodes = queues[choice].fromStdin();
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
					cerr << " " << x << ": " << FILTERS[x].name << ": " << FILTERS[x].description << "\n";
				}
			}
			cin >> choice;
			if(choice < 0 || choice >= NUMFILTERS) {
				break;
			}
			if(!filtersOn[choice]) {
				numselected++;
				filtersOn[choice] = true;
                auto fil = FILTERS[choice].fromStdin()();
				filters.push_back(move(fil));
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
					cerr << " " << x << ": " << nodeMods[x].name << ": " << nodeMods[x].description << "\n";
				}
			}
			cin >> choice;
			if(choice < 0 || choice >= NUMNODEMODS) {
				break;
			}
			if(!nodeModsOn[choice]) {
				numselected++;
				nodeModsOn[choice] = true;
                auto nm = nodeMods[choice].fromStdin()();
				mods.push_back(move(nm));
			}
		}
	}

    auto mapper = std::make_unique<toqm::ToqmMapper>(
        nodes,
        move(ex),
        move(cf),
        move(lat),
        move(mods),
        move(filters));

    mapper->setRetainPopped(retainPopped);
    mapper->setInitialSearchCycles(initialSearchCycles);
    mapper->setVerbose(toqm::_verbose);

    if (use_specified_init_mapping == 1) {
        mapper->setInitialMappingQal(init_qal);
    } else if (use_specified_init_mapping == 2) {
        mapper->setInitialMappingLaq(init_laq);
    }

    auto qasmFile = ifstream(qasmFileName);
    auto couplingMapFile = ifstream(couplingMapFileName);

    auto qasm = toqm::QasmObject::fromQasm2(qasmFile);
    auto couplingMap = toqm::parseCouplingMap(couplingMapFile);

    // invoke TOQM algo
    auto result = mapper->run(qasm->gateOperations(), qasm->numQubits(), couplingMap);

    // write new qasm to std::cout
    qasm->toQasm2(cout, *result);
	
	return 0;
}