#include "ToqmMapper.hpp"

#include <cassert>
#include <iostream>
#include <stack>
#include <utility>
#include <vector>
#include <sstream>

#include "CostFunc.hpp"
#include "GateNode.hpp"
#include "Latency.hpp"
#include "Environment.hpp"
#include "Expander.hpp"
#include "ScheduledGate.hpp"
#include "LinkedStack.hpp"
#include "Node.hpp"
#include "ToqmResult.hpp"

namespace toqm {

namespace {
//set each node's distance to furthest leaf node
//while we're at it, record the next 2-bit gate (cnot) from each gate node
int setCriticality(GateNode ** lastGatePerQubit, int numQubits) {
	GateNode ** gates = new GateNode * [numQubits];
	for(int x = 0; x < numQubits; x++) {
		gates[x] = lastGatePerQubit[x];
		if(gates[x]) {
			gates[x]->nextTargetCNOT = nullptr;
			gates[x]->nextControlCNOT = nullptr;
			gates[x]->criticality = 0;
		}
	}
	
	int maxCrit = 0;
	
	bool done = false;
	while(!done) {
		done = true;
		for(int x = 0; x < numQubits; x++) {
			GateNode * g = gates[x];
			if(g) {
				done = false;
			} else {
				continue;
			}
			
			//mark ready if gate is 1-qubit or appears twice in gates array
			bool ready = (g->control < 0) || (gates[g->control] == gates[g->target]);
			
			if(ready) {
				int crit = g->criticality + g->optimisticLatency;
				if(crit > maxCrit) {
					maxCrit = crit;
				}
				
				GateNode * parentT = g->targetParent;
				GateNode * parentC = g->controlParent;
				if(parentT) {
					//set parent's criticality
					if(crit > parentT->criticality) {
						parentT->criticality = crit;
					}
					
					//set parent's next 2-bit gate
					GateNode * nextCX;
					if(g->control >= 0) {
						nextCX = g;
					} else {
						nextCX = g->nextTargetCNOT;
					}
					if(parentT->target == g->target) {
						parentT->nextTargetCNOT = nextCX;
					} else if(g->control >= 0 && parentT->target == g->control) {
						parentT->nextTargetCNOT = nextCX;
					}
					if(parentT->control == g->target) {
						parentT->nextControlCNOT = nextCX;
					} else if(g->control >= 0 && parentT->control == g->control) {
						parentT->nextControlCNOT = nextCX;
					}
				}
				if(parentC) {
					//set parent's criticality
					if(crit > parentC->criticality) {
						parentC->criticality = crit;
					}
					
					//set parent's next 2-bit gate
					GateNode * nextCX;
					if(g->control >= 0) {
						nextCX = g;
					} else {
						nextCX = g->nextTargetCNOT;
					}
					if(parentC->target == g->target) {
						parentC->nextTargetCNOT = nextCX;
					} else if(g->control >= 0 && parentC->target == g->control) {
						parentC->nextTargetCNOT = nextCX;
					}
					if(parentC->control == g->target) {
						parentC->nextControlCNOT = nextCX;
					} else if(g->control >= 0 && parentC->control == g->control) {
						parentC->nextControlCNOT = nextCX;
					}
				}
				
				//adjust gates array
				assert(gates[g->target] == g);
				gates[g->target] = parentT;
				if(g->control >= 0) {
					assert(gates[g->control] == g);
					gates[g->control] = parentC;
				}
			}
		}
	}
	
	delete[] gates;
	
	return maxCrit;
}

//build dependence graph, put root gates into firstGates:
void
buildDependencyGraph(const std::vector<GateOp> & gates, std::size_t maxQubits, const Latency & lat,
					 set<GateNode *> & firstGates, int & numQubits, Environment * env,
					 int & idealCycles) {
	numQubits = 0;
	
	env->numGates = gates.size();
	env->firstCXPerQubit = new GateNode * [maxQubits];
	for(int x = 0; x < maxQubits; x++) {
		env->firstCXPerQubit[x] = 0;
	}
	
	//build dependence graph
	GateNode ** lastGatePerQubit = new GateNode * [maxQubits];
	for(int x = 0; x < maxQubits; x++) {
		lastGatePerQubit[x] = 0;
	}
	for(const auto & gate: gates) {
		GateNode * v = new GateNode;
		v->control = gate.control;
		v->target = gate.target;
		v->name = gate.type;
		v->criticality = 0;
		v->optimisticLatency = lat.getLatency(v->name, (v->control >= 0 ? 2 : 1), -1, -1);
		v->controlChild = 0;
		v->targetChild = 0;
		v->controlParent = 0;
		v->targetParent = 0;
		
		if(v->control >= 0) {
			if(!env->firstCXPerQubit[v->control]) {
				env->firstCXPerQubit[v->control] = v;
			}
			if(!env->firstCXPerQubit[v->target]) {
				env->firstCXPerQubit[v->target] = v;
			}
		}
		
		if(v->control >= numQubits) {
			numQubits = v->control + 1;
		}
		if(v->target >= numQubits) {
			numQubits = v->target + 1;
		}
		
		assert(v->control != v->target);
		
		//set parents, and adjust lastGatePerQubit
		if(v->control >= 0) {
			v->controlParent = lastGatePerQubit[v->control];
			if(v->controlParent) {
				if(lastGatePerQubit[v->control]->control == v->control) {
					lastGatePerQubit[v->control]->controlChild = v;
				} else {
					lastGatePerQubit[v->control]->targetChild = v;
				}
			}
			lastGatePerQubit[v->control] = v;
		}
		if(v->target >= 0) {
			v->targetParent = lastGatePerQubit[v->target];
			if(v->targetParent) {
				if(lastGatePerQubit[v->target]->control == v->target) {
					lastGatePerQubit[v->target]->controlChild = v;
				} else {
					lastGatePerQubit[v->target]->targetChild = v;
				}
			}
			lastGatePerQubit[v->target] = v;
		}
		
		//if v is a root gate, add it to firstGates
		if(!v->controlParent && !v->targetParent) {
			firstGates.insert(v);
		}
	}
	
	assert(numQubits <= maxQubits);
	
	//set critical path lengths starting from each gate
	idealCycles = setCriticality(lastGatePerQubit, numQubits);
	
	delete[] lastGatePerQubit;
}

////parseQasm2 coupling map, producing a list of edges and number of physical qubits
//void buildCouplingMap(const string& filename, set<pair<int, int> > &edges, int &numPhysicalQubits) {
//    std::fstream myfile(filename, std::ios_base::in);
//    unsigned int numEdges;
//
//    myfile >> numPhysicalQubits;
//    myfile >> numEdges;
//    for (unsigned int x = 0; x < numEdges; x++) {
//        int a, b;
//        myfile >> a;
//        myfile >> b;
//        pair<int, int> edge = make_pair(a, b);
//        edges.insert(edge);
//    }
//}

//Calculate minimum distance between each pair of physical qubits
//ToDo replace this with something more efficient?
void calcDistances(int * distances, int numQubits) {
	bool done = false;
	while(!done) {
		done = true;
		for(int x = 0; x < numQubits; x++) {
			for(int y = 0; y < numQubits; y++) {
				if(x == y) {
					continue;
				}
				for(int z = 0; z < numQubits; z++) {
					if(x == z || y == z) {
						continue;
					}
					
					if(distances[x * numQubits + y] + distances[y * numQubits + z] <
					   distances[x * numQubits + z]) {
						done = false;
						distances[x * numQubits + z] =
								distances[x * numQubits + y] + distances[y * numQubits + z];
						distances[z * numQubits + x] = distances[x * numQubits + z];
					}
				}
			}
		}
	}
}

}

enum InitMapping {
	None,
	QAL,
	LAQ
};

struct ToqmMapper::Impl {
	QueueFactory nodes_queue;
	std::unique_ptr<Expander> expander;
	std::unique_ptr<CostFunc> cost_func;
	std::unique_ptr<Latency> latency;
	std::vector<std::unique_ptr<NodeMod>> node_mods;
	std::vector<std::unique_ptr<Filter>> filters;
	int initialSearchCycles;
	int retainPopped;
	InitMapping init_mapping;
	const char * init_qal;
	const char * init_laq;
	bool verbose;
	
	std::unique_ptr<ToqmResult>
	run(const std::vector<GateOp> & gate_ops, std::size_t num_qubits, const CouplingMap & coupling_map) const {
		auto nodes = nodes_queue();
		
		// create fresh deep copy of filters for run
		std::vector<std::unique_ptr<Filter>> run_filters;
		run_filters.reserve(filters.size());
		for(auto & filter: filters) {
			run_filters.push_back(filter->createEmptyCopy());
		}
		
		auto env = new Environment(*cost_func, *latency, node_mods);
		env->swapCost = latency->getLatency("swp", 2, -1, -1);
		env->filters = std::move(run_filters);
		env->couplings = coupling_map.edges;
		env->numPhysicalQubits = coupling_map.numPhysicalQubits;
		
		set<GateNode *> firstGates;
		int idealCycles = -1;
		buildDependencyGraph(gate_ops, num_qubits, *latency, firstGates, env->numLogicalQubits, env, idealCycles);
		
		assert(env->numPhysicalQubits >= env->numLogicalQubits);
		
		//Calculate distances between physical qubits in coupling map (min 1, max numPhysicalQubits-1)
		env->couplingDistances = new int[env->numPhysicalQubits * env->numPhysicalQubits];
		for(int x = 0; x < env->numPhysicalQubits * env->numPhysicalQubits; x++) {
			env->couplingDistances[x] = env->numPhysicalQubits - 1;
		}
		for(auto iter = env->couplings.begin(); iter != env->couplings.end(); iter++) {
			int x = (*iter).first;
			int y = (*iter).second;
			env->couplingDistances[x * env->numPhysicalQubits + y] = 1;
			env->couplingDistances[y * env->numPhysicalQubits + x] = 1;
		}
		calcDistances(env->couplingDistances, env->numPhysicalQubits);
		
		int initialSearchCycles = this->initialSearchCycles;
		if(initialSearchCycles < 0) {
			int diameter = 0;
			for(int x = 0; x < env->numPhysicalQubits - 1; x++) {
				for(int y = x + 1; y < env->numPhysicalQubits; y++) {
					if(env->couplingDistances[x * env->numPhysicalQubits + y] > diameter) {
						diameter = env->couplingDistances[x * env->numPhysicalQubits + y];
					}
				}
			}
			initialSearchCycles = diameter;
		}
		
		//Prepare list of gates corresponding to possible swaps
		//ToDo: make it so this won't cause redundancies when given directed coupling map
		//might need to adjust parts of code that infer its size from coupling's size
		env->possibleSwaps = new GateNode * [env->couplings.size()];
		auto iter = env->couplings.begin();
		int x = 0;
		while(iter != env->couplings.end()) {
			GateNode * g = new GateNode();
			g->control = (*iter).first;
			g->target = (*iter).second;
			g->name = "swp";
			g->optimisticLatency = latency->getLatency("swp", 2, g->target, g->control);
			env->possibleSwaps[x] = g;
			x++;
			iter++;
		}
		
		//Set up root node (for cycle -1, before any gates are scheduled):
		auto root = std::shared_ptr<Node>(new Node());
		for(int x = env->numLogicalQubits; x < env->numPhysicalQubits; x++) {
			root->laq[x] = -1;
			root->qal[x] = -1;
		}
		if(init_mapping == QAL) {
			for(int x = 0; x < env->numPhysicalQubits; x++) {
				root->qal[x] = init_qal[x];
				if(init_qal[x] >= 0) {
					root->laq[(int) init_qal[x]] = x;
				}
			}
		} else if(init_mapping == LAQ) {
			for(int x = 0; x < env->numPhysicalQubits; x++) {
				root->laq[x] = init_laq[x];
				if(init_laq[x] >= 0) {
					root->qal[(int) init_laq[x]] = x;
				}
			}
		}
		root->parent = nullptr;
		root->numUnscheduledGates = env->numGates;
		root->env = env;
		root->cycle = -1;
		if(initialSearchCycles) {
			//std::cerr << "//Note: making attempt to find better initial mapping.\n";
			root->cycle -= initialSearchCycles;
		}
		root->readyGates = firstGates;
		root->scheduled = new LinkedStack<ScheduledGate *>;
		root->cost = cost_func->getCost(*root);
		nodes->push(root);

//    // TODO: is this needed? why delete filters that the user didn't select?
//    //Cleanup filters before I start messing things up:
//    for (int x = 0; x < NUMFILTERS; x++) {
//        bool del = true;
//        for (unsigned int y = 0; y < env->filters.size(); y++) {
//            if (env->filters[y] == std::get<0>(FILTERS[x])) {
//                del = false;
//                break;
//            }
//        }
//        if (del) {
//            delete std::get<0>(FILTERS[x]);
//        }
//    }
		env->resetFilters();
		
		//Pop nodes from the queue until we're done:
		bool notDone = true;
		std::vector<std::shared_ptr<Node>> tempNodes;
		int numPopped = 0;
		int counter = 0;
		std::deque<std::shared_ptr<Node>> oldNodes;
		while(notDone) {
			assert(nodes->size() > 0);
			
			while(retainPopped && oldNodes.size() > retainPopped) {
				auto pop = oldNodes.front();
				oldNodes.pop_front();
				if(pop == nodes->getBestFinalNode()) {
					oldNodes.push_back(pop);
				} else {
					env->deleteRecord(*pop);
				}
			}
			
			auto n = nodes->pop();
			n->expanded = true;
			
			if(n->dead) {
				if(n == nodes->getBestFinalNode()) {
					oldNodes.push_back(n);
				} else {
					env->deleteRecord(*n);
				}
				continue;
			}
			
			oldNodes.push_back(n);
			
			/*
			if(n->parent && n->parent->dead) {
				std::cerr << "skipping child of dead node:\n";
				printNode(std::cerr, lastNode->scheduled);
				n->dead = true;
				continue;
			}
			*/
			
			numPopped++;
			
			//In verbose mode, we pause after popping some number of nodes:
			if(verbose && counter <= 0) {
				cerr << "cycle " << n->cycle << "\n";
				cerr << "cost " << n->cost << "\n";
				cerr << "unscheduled " << n->numUnscheduledGates << " from this node\n";
				std::cerr << "mapping (logical qubit at each location): ";
				for(int x = 0; x < env->numPhysicalQubits; x++) {
					std::cerr << (int) n->qal[x] << ", ";
				}
				std::cerr << "\n";
				std::cerr << "mapping (location of each logical qubit): ";
				for(int x = 0; x < env->numPhysicalQubits; x++) {
					std::cerr << (int) n->laq[x] << ", ";
				}
				std::cerr << "\n";
				std::cerr << "//" << (numPopped - 1) << " nodes popped from queue so far.\n";
				std::cerr << "//" << nodes->size() << " nodes remain in queue.\n";
				env->printFilterStats(std::cerr);
				//printNode(std::cerr, n->scheduled);
				//cf->getCost(n);
				for(GateNode * ready: n->readyGates) {
					std::cerr << "ready: ";
					int control = (ready->control >= 0) ? n->laq[ready->control] : -1;
					int target = (ready->target >= 0) ? n->laq[ready->target] : -1;
					std::cerr << ready->name << " ";
					if(ready->control >= 0) {
						std::cerr << "q[" << control << "],";
					}
					std::cerr << "q[" << target << "]";
					std::cerr << ";";
					
					target = ready->target;
					control = ready->control;
					std::cerr << " //" << ready->name << " ";
					if(control >= 0) {
						std::cerr << "q[" << control << "],";
					}
					std::cerr << "q[" << target << "]";
					std::cerr << "\n";
				}
				
				cin >> counter;//pause the program after (counter) steps
				if(counter < 0) exit(1);
			}
			
			notDone = expander->expand(*nodes, *n);
			
			counter--;
		}
		
		auto finalNode = nodes->getBestFinalNode();
		
		//Figure out what the initial mapping must have been
		LinkedStack<ScheduledGate *> * sg = finalNode->scheduled;
		std::vector<char> inferredQal(env->numPhysicalQubits);
		std::vector<char> inferredLaq(env->numPhysicalQubits);
		
		for(int x = 0; x < env->numPhysicalQubits; x++) {
			inferredQal[x] = finalNode->qal[x];
			inferredLaq[x] = finalNode->laq[x];
		}
		/*
		std::cerr << "//Note: qubit mapping at end (location of each logical qubit): ";
		for(int x = 0; x < env->numLogicalQubits; x++) {
			std::cerr << (int)inferredLaq[x] << ", ";
		}
		std::cerr << "\n";
		std::cerr << "//Note: qubit mapping at end (logical qubit at each location): ";
		for(int x = 0; x < env->numPhysicalQubits; x++) {
			std::cerr << (int)inferredQal[x] << ", ";
		}
		std::cerr << "\n";
		*/
		while(sg->size > 0) {
			if(sg->value->gate->control >= 0) {
				if((!sg->value->gate->name.compare("swp")) || (!sg->value->gate->name.compare("SWP"))) {
					
					if(inferredQal[sg->value->physicalControl] >= 0 &&
					   inferredQal[sg->value->physicalTarget] >= 0) {
						std::swap(inferredLaq[(int) inferredQal[sg->value->physicalControl]],
								  inferredLaq[(int) inferredQal[sg->value->physicalTarget]]);
					} else if(inferredQal[sg->value->physicalControl] >= 0) {
						inferredLaq[(int) inferredQal[sg->value->physicalControl]] = sg->value->physicalTarget;
					} else if(inferredQal[sg->value->physicalTarget] >= 0) {
						inferredLaq[(int) inferredQal[sg->value->physicalTarget]] = sg->value->physicalControl;
					}
					
					std::swap(inferredQal[sg->value->physicalTarget], inferredQal[sg->value->physicalControl]);
				}
			} else {
				if(sg->value->physicalTarget < 0) {
					sg->value->physicalTarget = inferredLaq[sg->value->gate->target];
				}
				
				//in case this qubit's assignment is arbitrary:
				if(sg->value->physicalTarget < 0) {
					for(int x = 0; x < env->numPhysicalQubits; x++) {
						if(inferredQal[x] < 0) {
							inferredQal[x] = sg->value->gate->target;
							inferredLaq[sg->value->gate->target] = x;
							sg->value->physicalTarget = x;
							break;
						}
					}
				}
			}
			sg = sg->next;
		}
		
		stringstream filterStats;
		env->printFilterStats(filterStats);
		
		auto result = unique_ptr<ToqmResult>(new ToqmResult{
				finalNode,
				std::move(nodes),
				env->numPhysicalQubits,
				env->numLogicalQubits,
				std::move(inferredQal),
				std::move(inferredLaq),
				idealCycles,
				numPopped,
				filterStats.str()
		});
		
		return result;
	}
};

ToqmMapper::ToqmMapper(QueueFactory node_queue,
					   std::unique_ptr<Expander> expander,
					   std::unique_ptr<CostFunc> cost_func,
					   std::unique_ptr<Latency> latency,
					   std::vector<std::unique_ptr<NodeMod>> node_mods,
					   std::vector<std::unique_ptr<Filter>> filters)
		: impl(new Impl{std::move(node_queue), std::move(expander), std::move(cost_func), std::move(latency),
						std::move(node_mods), std::move(filters), 0, 0, None,
						nullptr, nullptr, false}) {}

ToqmMapper::~ToqmMapper() = default;

void ToqmMapper::setInitialSearchCycles(int initial_search_cycles) {
	this->impl->initialSearchCycles = initial_search_cycles;
}

void ToqmMapper::setRetainPopped(int retainPopped) {
	this->impl->retainPopped = retainPopped;
}

void ToqmMapper::setInitialMappingQal(const char * init_qal) {
	this->impl->init_mapping = QAL;
	this->impl->init_qal = init_qal;
	this->impl->init_laq = nullptr;
}

void ToqmMapper::setInitialMappingLaq(const char * init_laq) {
	this->impl->init_mapping = LAQ;
	this->impl->init_laq = init_laq;
	this->impl->init_qal = nullptr;
}

void ToqmMapper::clearInitialMapping() {
	this->impl->init_mapping = None;
	this->impl->init_laq = nullptr;
	this->impl->init_qal = nullptr;
}

void ToqmMapper::setVerbose(bool verbose) {
	this->impl->verbose = verbose;
}

std::unique_ptr<ToqmResult> ToqmMapper::run(const std::vector<GateOp> & gate_ops, std::size_t num_qubits,
											const CouplingMap & coupling_map) const {
	return impl->run(gate_ops, num_qubits, coupling_map);
}

}