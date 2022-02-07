#include "ToqmMapper.hpp"

#include <cassert>
#include <iostream>
#include <stack>
#include <utility>
#include <vector>
#include <sstream>

#include "CostFunc.hpp"
#include "Environment.hpp"
#include "Expander.hpp"
#include "GateNode.hpp"
#include "Latency.hpp"
#include "Node.hpp"
#include "Queue.hpp"

namespace toqm {

bool _verbose = false;

namespace {

//set each node's distance to furthest leaf node
//while we're at it, record the next 2-bit gate (cnot) from each gate node
int setCriticality(const std::vector<GateNode*>& lastGatePerQubit, int numQubits) {
	std::vector<GateNode*> gates(numQubits);
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
			auto g = gates[x];
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
				
				auto parentT = g->targetParent;
				auto parentC = g->controlParent;
				if(parentT) {
					//set parent's criticality
					if(crit > parentT->criticality) {
						parentT->criticality = crit;
					}
					
					//set parent's next 2-bit gate
					GateNode* nextCX;
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
	
	return maxCrit;
}

// Create DAG from topological ordering of gates.
// Put root gates into firstGates.
// Calculate num logical qubits.
// Calculate ideal cycles.
// TODO: we don't need maxQubits. We can use a map intead of vector,
// and remove maxQubits from `ToqmMapper::run`.
void
buildDependencyGraph(const std::vector<GateOp> & gates,
					 std::size_t maxQubits,
					 const Latency & lat,
					 std::set<GateNode*> & firstGates,
					 int & numQubits,
					 Environment& env,
					 int & idealCycles) {
	numQubits = 0;
	
	env.numGates = gates.size();
	env.firstCXPerQubit.resize(maxQubits);
	
	//build dependence graph
	std::vector<GateNode*> lastGatePerQubit(maxQubits);

	for(const auto & gate: gates) {
		auto v_shared = std::unique_ptr<GateNode>(new GateNode{});
		const auto v = v_shared.get();
		env.managedGateNodes.push_back(std::move(v_shared));
		
		v->uid = gate.uid;
		v->control = gate.control;
		v->target = gate.target;
		v->name = gate.type;
		v->criticality = 0;
		v->optimisticLatency = lat.getLatency(v->name, (v->control >= 0 ? 2 : 1), -1, -1);
		v->controlChild = nullptr;
		v->targetChild = nullptr;
		v->controlParent = nullptr;
		v->targetParent = nullptr;
		
		if(v->control >= 0) {
			if(!env.firstCXPerQubit[v->control]) {
				env.firstCXPerQubit[v->control] = v;
			}
			if(!env.firstCXPerQubit[v->target]) {
				env.firstCXPerQubit[v->target] = v;
			}
		}
		
		if(v->control >= numQubits) {
			numQubits = v->control + 1;
		}
		if(v->target >= numQubits) {
			numQubits = v->target + 1;
		}
		
		// TODO: move to input validation of `gates` for `ToqmMapper::run`.
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
}

//Calculate minimum distance between each pair of physical qubits
//ToDo replace this with something more efficient?
// ...this is numQubits^3 execution.
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

struct ToqmMapper::Impl {
	std::unique_ptr<Queue> nodes_queue;
	std::unique_ptr<Expander> expander;
	std::unique_ptr<CostFunc> cost_func;
	std::unique_ptr<Latency> latency;
	std::vector<std::unique_ptr<NodeMod>> node_mods;
	std::vector<std::unique_ptr<Filter>> filters;
	int retainPopped;
	
	std::unique_ptr<ToqmResult>
	run(const std::vector<GateOp> & gate_ops, std::size_t num_qubits, const CouplingMap & coupling_map, int initialSearchCycles, std::vector<char> init_qal) const {
		// create fresh deep copy of filters for run
		std::vector<std::unique_ptr<Filter>> run_filters;
		run_filters.reserve(filters.size());
		for(auto & filter: filters) {
			run_filters.push_back(filter->createEmptyCopy());
		}
		
		auto env = std::unique_ptr<Environment>(new Environment(*cost_func, *latency, node_mods));
		
		// Get most optimistic swap cost. Passing numQubits > 0 and target/control < 0
		// indicates we want best case.
		env->swapCost = latency->getLatency("swp", 2, -1, -1);
		
		env->filters = std::move(run_filters);
		env->couplings = coupling_map.edges;
		env->numPhysicalQubits = coupling_map.numPhysicalQubits;
		
		std::set<GateNode*> firstGates;
		int idealCycles = -1;
		buildDependencyGraph(gate_ops, num_qubits, *latency, firstGates, env->numLogicalQubits, *env, idealCycles);
		
		assert(env->numPhysicalQubits >= env->numLogicalQubits);
		
		// Calculate distances between physical qubits in coupling map (min 1, max numPhysicalQubits-1)
		// First, initialize all pairs to max.
		env->couplingDistances = new int[env->numPhysicalQubits * env->numPhysicalQubits];
		for(int x = 0; x < env->numPhysicalQubits * env->numPhysicalQubits; x++) {
			env->couplingDistances[x] = env->numPhysicalQubits - 1;
		}
		
		// Then, initialize adjacent pairs to min.
		for(auto iter = env->couplings.begin(); iter != env->couplings.end(); iter++) {
			int x = (*iter).first;
			int y = (*iter).second;
			env->couplingDistances[x * env->numPhysicalQubits + y] = 1;
			env->couplingDistances[y * env->numPhysicalQubits + x] = 1;
		}
		
		// Then, calculate min distances between all pairs.
		calcDistances(env->couplingDistances, env->numPhysicalQubits);
		
		// If initial search cycles is not user-specified, set it to "diameter".
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
		
		// For each physical coupling, add a swap gate to `possibleSwaps`.
		//ToDo: make it so this won't cause redundancies when given directed coupling map
		//might need to adjust parts of code that infer its size from coupling's size
		env->possibleSwaps.resize(env->couplings.size());
		auto iter = env->couplings.begin();
		int x = 0;
		while(iter != env->couplings.end()) {
			auto g_shared = std::unique_ptr<GateNode>(new GateNode{});
			auto g = g_shared.get();
			env->managedGateNodes.push_back(std::move(g_shared));
			
			g->uid = -1; // generated swap has no user-provided ID
			g->control = (*iter).first;
			g->target = (*iter).second;
			g->name = "swp";
			g->optimisticLatency = latency->getLatency("swp", 2, g->target, g->control);
			env->possibleSwaps[x] = g;
			x++;
			iter++;
		}
		
		// Set up root node (for cycle -1, before any gates are scheduled_final).
		// Note: the lifetimes of all Node instances must not exceed the lifetime
		//       of env and hence, this method.
		auto root = std::shared_ptr<Node>(new Node(*env));
		
		// TODO: is this right? => Clear mapping for any physical qubits that aren't used by logical qubits
		for(int x = env->numLogicalQubits; x < env->numPhysicalQubits; x++) {
			root->laq[x] = -1;
			root->qal[x] = -1;
		}
		
		// Init root with initial layout
		if (!init_qal.empty()) {
			for(int x = 0; x < env->numPhysicalQubits; x++) {
				root->qal[x] = init_qal[x];
				if(init_qal[x] >= 0) {
					root->laq[(int) init_qal[x]] = x;
				}
			}
		}
		
		root->parent = nullptr;
		root->numUnscheduledGates = env->numGates;
		root->cycle = -1;
		
		// Setting root-cycle < -1 indicates search cycles.
		if(initialSearchCycles) {
			//std::cerr << "//Note: making attempt to find better initial mapping.\n";
			root->cycle -= initialSearchCycles;
		}
		root->readyGates = firstGates;
		root->scheduled = std::shared_ptr<ScheduledGateStack>(new ScheduledGateStack());
		root->cost = cost_func->getCost(*root);
		
		auto nodes = nodes_queue->clone();
		nodes->push(root); // TODO: should Queue own all nodes, only?

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
				printNode(std::cerr, lastNode->scheduled_final);
				n->dead = true;
				continue;
			}
			*/
			
			numPopped++;
			
			//In verbose mode, we pause after popping some number of nodes:
			if(_verbose && counter <= 0) {
				std::cerr << "cycle " << n->cycle << "\n";
				std::cerr << "cost " << n->cost << "\n";
				std::cerr << "unscheduled " << n->numUnscheduledGates << " from this node\n";
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
				//printNode(std::cerr, n->scheduled_final);
				//cf->getCost(n);
				for(auto & ready: n->readyGates) {
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
				
				std::cin >> counter;//pause the program after (counter) steps
				if(counter < 0) exit(1);
			}
			
			notDone = expander->expand(*nodes, n);
			
			counter--;
		}
		
		auto & finalNode = nodes->getBestFinalNode();
		
		//Figure out what the initial mapping must have been
		auto sg = finalNode->scheduled;
		std::vector<int> inferredQal(env->numPhysicalQubits);
		std::vector<int> inferredLaq(env->numPhysicalQubits);
		
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
						std::swap(inferredLaq[inferredQal[sg->value->physicalControl]],
								  inferredLaq[inferredQal[sg->value->physicalTarget]]);
					} else if(inferredQal[sg->value->physicalControl] >= 0) {
						inferredLaq[inferredQal[sg->value->physicalControl]] = sg->value->physicalTarget;
					} else if(inferredQal[sg->value->physicalTarget] >= 0) {
						inferredLaq[inferredQal[sg->value->physicalTarget]] = sg->value->physicalControl;
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
		
		// Create a vector from the scheduled gates stack for the result
		auto & gates = finalNode->scheduled;
		std::vector<ScheduledGateOp> scheduled_final {};
		scheduled_final.reserve(gates->size);
		
		int gate_count = gates->size;
		for (int i = 0; i < gate_count; i++) {
			assert(gates->size > 0);
			
			auto & sg = gates->value;
			auto & g = gates->value->gate;
			
			scheduled_final.push_back(ScheduledGateOp{
					GateOp(g->uid, g->name, g->control, g->target),
					sg->physicalTarget,
					sg->physicalControl,
					sg->cycle,
					sg->latency
			});
			
			gates = gates->next;
		}
		
		// Reverse the stack so gates are in topological order
		// TODO: make this more efficient by creating in reverse order to begin with
		std::reverse(scheduled_final.begin(), scheduled_final.end());
		
		// Create copy of laq for result
		std::vector<int> laq_final(MAX_QUBITS);
		for (auto i = 0; i < MAX_QUBITS; i++) {
			laq_final[i] = finalNode->laq[i];
		}
		
		std::stringstream filterStats;
		env->printFilterStats(filterStats);
		
		auto result = std::unique_ptr<ToqmResult>(new ToqmResult{
				std::move(scheduled_final),
				nodes->size(),
				env->numPhysicalQubits,
				env->numLogicalQubits,
				std::move(laq_final),
				std::move(inferredQal),
				std::move(inferredLaq),
				idealCycles,
				numPopped,
				filterStats.str()
		});
		
		// Cleanup
		// TODO: migrate Environment to smart pointers
		//delete [] env->possibleSwaps;
		delete [] env->couplingDistances;
		
		return result;
	}
};

ToqmMapper::ToqmMapper(const Queue& node_queue,
					   std::unique_ptr<Expander> expander,
					   std::unique_ptr<CostFunc> cost_func,
					   std::unique_ptr<Latency> latency,
					   std::vector<std::unique_ptr<NodeMod>> node_mods,
					   std::vector<std::unique_ptr<Filter>> filters)
		: impl(new Impl{node_queue.clone(), std::move(expander), std::move(cost_func), std::move(latency),
						std::move(node_mods), std::move(filters), 0}) {}

ToqmMapper::~ToqmMapper() = default;

void ToqmMapper::setRetainPopped(int retainPopped) {
	this->impl->retainPopped = retainPopped;
}

void ToqmMapper::setVerbose(bool verbose) {
	_verbose = verbose;
}

std::unique_ptr<ToqmResult> ToqmMapper::run(const std::vector<GateOp> & gates, std::size_t num_qubits,
											const CouplingMap & coupling_map) const {
	// -1 initial search cycles indicates we should `diameter` cycles
	// to do a full search for the best initial layout.
	return impl->run(gates, num_qubits, coupling_map, -1, {});
}

std::unique_ptr<ToqmResult>
ToqmMapper::run(const std::vector<GateOp> & gates, std::size_t num_qubits, const CouplingMap & coupling_map, int initial_search_cycles) const {
	return impl->run(gates, num_qubits, coupling_map, initial_search_cycles, {});
}

std::unique_ptr<ToqmResult>
ToqmMapper::run(const std::vector<GateOp> & gates, std::size_t num_qubits, const CouplingMap & coupling_map, int initial_search_cycles, const std::vector<int> & init_qal) const {
	// TODO: port entire codebase to use std::vector<size_t> for QAL/LAQ and remove this.
	// 	This is only here so the ToqmMapper interface can be ideal to start.
	std::vector<char> init_qal_c {};
	init_qal_c.reserve(init_qal.size());
	for (auto x : init_qal) {
		assert(x <= std::numeric_limits<char>::max());
		init_qal_c.push_back((char)x);
	}
	
	return impl->run(gates, num_qubits, coupling_map, initial_search_cycles, std::move(init_qal_c));
}


}