#ifndef DEFAULTEXPANDER_HPP
#define DEFAULTEXPANDER_HPP

#include "libtoqm/Expander.hpp"
#include "libtoqm/Queue.hpp"
#include "libtoqm/CostFunc.hpp"

#include <cassert>
#include <vector>
#include <iostream>
#include <stdexcept>

namespace toqm {

namespace {
//return true iff inserting swap g in node's child would make a useless swap cycle
bool isCyclic(const Node & node, const GateNode * g) {
	int target = g->target;
	int control = g->control;

	if(node.lastGate[target] && node.lastGate[control]) {
		auto schdule = node.scheduled;
		while(schdule->size > 0) {
			if(schdule->value->gate == g) {
				return true;
			} else {
				int c = schdule->value->physicalControl;
				int t = schdule->value->physicalTarget;
				if(c >= 0) {
					if(c == control || c == target || t == control || t == target) {
						return false;
					}
				}
			}
			schdule = schdule->next;
		}
	}
	return false;
}
}

class DefaultExpander : public Expander {
public:
	bool expand(Queue& nodes, const std::shared_ptr<Node>& node) const override {
		//return false if we're done expanding
		if(nodes.getBestFinalNode() && node->cost >= nodes.getBestFinalNode()->cost) {
			return false;
		}
		
		unsigned int nodesSize = nodes.size();
		
		auto noMoreCX = std::vector<bool>(node->env.numPhysicalQubits);
		for(int x = 0; x < node->env.numPhysicalQubits; x++) {
			noMoreCX[x] = false;
			auto sg = node->lastNonSwapGate[x];
			if(sg) {
				if(sg->gate->target == x) {
					if(!sg->gate->nextTargetCNOT) {
						noMoreCX[x] = true;
					}
				} else {
					assert(sg->gate->control == x);
					if(!sg->gate->nextControlCNOT) {
						noMoreCX[x] = true;
					}
				}
			}
		}
		
		//generate list of valid gates, based on ready list
		std::vector<GateNode*> possibleGates;//possible swaps and valid 2+ cycle gates
		std::vector<GateNode*> singleCycleGates;//valid 1 cycle non-swap gates
		int numDependentGates = 0;
		for(auto iter = node->readyGates.begin(); iter != node->readyGates.end(); iter++) {
			auto & g = *iter;
			int target = (g->target < 0) ? -1 : node->laq[g->target];
			int control = (g->control < 0) ? -1 : node->laq[g->control];
			
			bool good = node->cycle >= -1;
			bool dependsOnSomething = false;
			
			if(control >= 0) {//gate has a control qubit
				int busy = node->busyCycles(control);
				if(busy) {
					dependsOnSomething = true;
					if(busy > 1) {
						good = false;
					}
				}
			} else {
				if(!g->nextTargetCNOT) {
					noMoreCX[target] = true;
				}
			}
			
			if(g->target >= 0) {//gate has a target qubit
				int busy = node->busyCycles(target);
				if(busy) {
					dependsOnSomething = true;
					if(busy > 1) {
						good = false;
					}
				}
			}
			
			if(dependsOnSomething) {
				numDependentGates++;
			}
			
			if(good && node->cycle > 0 && nodesSize > 0 && !dependsOnSomething) {
				good = false;
			}
			
			if(good && control >= 0 && target >= 0) {//gate has 2 qubits
				if(node->env.couplings.count(std::make_pair(target, control)) <= 0) {
					if(node->env.couplings.count(std::make_pair(control, target)) <= 0) {
						good = false;
					}
				}
			}
			
			if(good) {
				int latency = node->env.latency.getLatency(g->name, (control >= 0 ? 2 : 1), target, control);
				// keha: added latency == 0 to support case when 0-latency
				// gate is at front of circuit or couldn't be scheduled immediately mid-circuit
				// due to incompatible current layout.
				if(latency == 1 || latency == 0) {
					singleCycleGates.push_back(g);
				} else {
					possibleGates.push_back(g);
				}
			}
		}
		//generate list of valid gates, based on possible swaps
		for(unsigned int x = 0; x < node->env.couplings.size(); x++) {
			auto & g = node->env.possibleSwaps[x];
			int target = g->target;//note: since g is swap, this is already a physical target
			int control = g->control;//note: since g is swap, this is already a physical control
			int logicalTarget = (target >= 0) ? node->qal[target] : -1;
			int logicalControl = (control >= 0) ? node->qal[control] : -1;
			
			bool good = true;
			bool dependsOnSomething = false;
			
			bool usesLogicalQubit = false;
			if(good && logicalTarget >= 0) {
				usesLogicalQubit = true;
			}
			if(good && logicalControl >= 0) {
				usesLogicalQubit = true;
			}
			good = good && usesLogicalQubit;
			
			bool moreCXLogicalTarget = logicalTarget >= 0 && !noMoreCX[logicalTarget];
			bool moreCXLogicalControl = logicalControl >= 0 && !noMoreCX[logicalControl];
			
			//make sure this swap involves a qubit that has more 2-qubit gates ahead
			if(good && (!moreCXLogicalTarget && !moreCXLogicalControl)) {
				good = false;
			}
			
			bool usesUsefulLogicalQubit = false;
			if(good) {
				if(logicalTarget >= 0) {
					auto t = node->lastNonSwapGate[logicalTarget];
					if(t) {
						GateNode * tg = t->gate;
						if(tg->target == logicalTarget) {
							if(tg->targetChild) {
								usesUsefulLogicalQubit = true;
							}
						} else {
							assert(tg->control == logicalTarget);
							if(tg->controlChild) {
								usesUsefulLogicalQubit = true;
							}
						}
					} else {
						usesUsefulLogicalQubit = true;//better safe than sorry
					}
				}
				
				if(logicalControl >= 0) {
					auto c = node->lastNonSwapGate[logicalControl];
					if(c) {
						GateNode * cg = c->gate;
						if(cg->target == logicalControl) {
							if(cg->targetChild) {
								usesUsefulLogicalQubit = true;
							}
						} else {
							assert(cg->control == logicalControl);
							if(cg->controlChild) {
								usesUsefulLogicalQubit = true;
							}
						}
					} else {
						usesUsefulLogicalQubit = true;//better safe than sorry
					}
				}
			}
			if(!usesUsefulLogicalQubit) {//swapping qubits that are never used again
				good = false;
			}
			
			int busyT = node->busyCycles(target);
			if(good && busyT) {
				dependsOnSomething = true;
				if(busyT > 1) {
					good = false;
				}
			}
			int busyC = node->busyCycles(control);
			if(good && busyC) {
				dependsOnSomething = true;
				if(busyC > 1) {
					good = false;
				}
			}
			if(good && node->cycle > 0 && nodesSize > 0 && !dependsOnSomething) {
				good = false;
			}
			if(good && isCyclic(*node, g)) {
				good = false;
			}
			if(good) {
				numDependentGates++;
				possibleGates.push_back(g);
			}
		}
		if(nodesSize > 0 && !numDependentGates) {//this node can't lead to anything optimal
			//Reminder: this line caused problems when I tried placing it before looking at swaps
			return true;
		}

		if (possibleGates.size() >= 64) {
			// TODO: modify approach to support >= 64 gates.
			throw std::runtime_error("FATAL ERROR: current implementation cannot handle more than 63 possible gates.");
		}

		unsigned long long numIters = 1LL << possibleGates.size();
		for(unsigned long long x = 0; x < numIters; x++) {
			std::shared_ptr<Node> child = Node::prepChild(node.get());
			bool good = true;
			//Schedule a unique subset of {swaps and 2-qubit gates}:
			for(unsigned int y = 0; good && y < possibleGates.size(); y++) {
				if(x & (1LL << y)) {
					if(node->cycle >= -1) {
						good = child->scheduleGate(possibleGates[y]);
					} else {//keha: should we assert that this is a swap gate?
						good = child->swapQubits(possibleGates[y]->target, possibleGates[y]->control);
					}
				}
			}
			
			if(good) {
				//Schedule as many of the 1-cycle ready gates as we can:
				for(unsigned int y = 0; good && y < singleCycleGates.size(); y++) {
					// TODO: should `good` be updated here?
					child->scheduleGate(singleCycleGates[y]);
				}
				
				int cycleMod = (child->cycle < 0) ? child->cycle : 0;
				child->cycle -= cycleMod;
				child->cost = node->env.cost.getCost(*child);
				child->cycle += cycleMod;
				
				nodes.push(child);
			}
		}
		
		return true;
	}
	
	std::unique_ptr<Expander> clone() const override {
		return std::unique_ptr<Expander>(new DefaultExpander(*this));
	}
};

}

#endif
