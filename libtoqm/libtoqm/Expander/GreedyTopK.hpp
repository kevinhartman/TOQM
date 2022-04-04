#ifndef GREEDYTOPKEXPANDER_HPP
#define GREEDYTOPKEXPANDER_HPP

#include "libtoqm/Expander.hpp"
#include "libtoqm/Queue.hpp"
#include "libtoqm/CostFunc.hpp"

#include <cassert>
#include <vector>
#include <iostream>
#include <queue>
#include <stdexcept>
#include <sstream>

namespace toqm {

class GreedyTopK : public Expander {
private:
	unsigned int K = 0;

	using PriotityQueueType = std::tuple<std::size_t, std::shared_ptr<Node>>;
	
	struct CmpNodeCost {
		// REMINDER: I reversed the cost function here so I could remove inferior nodes on the fly,
		// maintaining a smaller priority queue of only K nodes
		bool operator()(const PriotityQueueType& lhs, const PriotityQueueType& rhs) const {
			auto lhs_node = std::get<1>(lhs);
			auto rhs_node = std::get<1>(rhs);

			//tiebreaker:
			if(lhs_node->cost == rhs_node->cost) {
				if (lhs_node->cost2 != rhs_node->cost2) {
					return lhs_node->cost2 < rhs_node->cost2;
				}

				return std::get<0>(lhs) > std::get<0>(rhs);
			}
			
			return lhs_node->cost < rhs_node->cost;
		}
	};

public:
	explicit GreedyTopK(unsigned int k) {
		this->K = k;
	}
	
	bool expand(Queue& nodes, const std::shared_ptr<Node>& node) const override {
		//return false if we're done expanding
		if(nodes.getBestFinalNode() && node->cost >= nodes.getBestFinalNode()->cost) {
			return false;
		}
		
		unsigned int nodesSize = nodes.size();
		int numQubits = node->env.numPhysicalQubits;
		
		auto occupied = std::vector<bool>(numQubits); // physical qubits used already by guaranteed gates
		auto onReadyFrontier = std::vector<bool>(numQubits); // physical qubits on the ready gates frontier.
		bool hasBusyQubits = false;
		for(int x = 0; x < numQubits; x++) {
			occupied[x] = false;
			onReadyFrontier[x] = false;
			if(node->busyCycles(x) > 0) {
				hasBusyQubits = true;
			}
		}
		
		//Identify, for each logical qubit, the CX (if any) that's on the CX frontier:
		auto CXFrontier = std::vector<GateNode *>(numQubits);
		for(int x = 0; x < numQubits; x++) {
			CXFrontier[x] = nullptr;
		}
		for(auto g : node->readyGates) {
			if(g->control >= 0) {
				CXFrontier[g->target] = g;
				CXFrontier[g->control] = g;
			}
		}
		for(auto g : node->readyGates) {
			if(g->control < 0) {
				g = g->nextTargetCNOT;
				if(g) {
					if(!CXFrontier[g->control]) {
						CXFrontier[g->target] = g;
						CXFrontier[g->control] = g;
					} else if(CXFrontier[g->control]->criticality < g->criticality) {
						CXFrontier[CXFrontier[g->control]->target] = nullptr;
						CXFrontier[g->target] = g;
						CXFrontier[g->control] = g;
					}
				}
			}
		}
		
		//generate list of valid gates, based on ready list and list of possible swaps
		std::vector<GateNode*> possibleGates;
		std::vector<GateNode*> guaranteedGates;
		for(auto iter = node->readyGates.begin(); iter != node->readyGates.end(); iter++) {
			auto & g = *iter;
			int physicalTarget = (g->target < 0) ? -1 : node->laq[g->target];
			int physicalControl = (g->control < 0) ? -1 : node->laq[g->control];
			
			bool good = (node->cycle >= -1);
			//bool dependsOnSomething = false;
			
			if(physicalControl >= 0) {//gate has a physicalControl qubit
				onReadyFrontier[physicalControl] = true;
				int busy = node->busyCycles(physicalControl);
				if(busy) {
					//dependsOnSomething = true;
					if(busy > 1) {
						good = false;
					}
				}
			}
			
			if(physicalTarget >= 0) {//gate has a physicalTarget qubit
				onReadyFrontier[physicalTarget] = true;
				int busy = node->busyCycles(physicalTarget);
				if(busy) {
					//dependsOnSomething = true;
					if(busy > 1) {
						good = false;
					}
				}
			}
			
			if(good && physicalControl >= 0 && physicalTarget >= 0) {//gate has 2 qubits
				if(node->env.couplings.count(std::make_pair(physicalTarget, physicalControl)) <= 0) {
					if(node->env.couplings.count(std::make_pair(physicalControl, physicalTarget)) <= 0) {
						good = false;
					}
				}
			}
			
			if(good) {
				guaranteedGates.push_back(g);

				if(physicalTarget >= 0) {
					occupied[physicalTarget] = true;
				}

				if(physicalControl >= 0) {
					occupied[physicalControl] = true;
				}
			}
		}
		for(unsigned int x = 0; x < node->env.couplings.size(); x++) {
			GateNode * g = node->env.possibleSwaps[x];
			int target = g->target;//note: since g is swap, this is already the physical target
			int control = g->control;//note: since g is swap, this is already the physical control
			int logicalTarget = (target >= 0) ? node->qal[target] : -1;
			int logicalControl = (control >= 0) ? node->qal[control] : -1;
			
			bool helpsCX = false;
			//bool hurtsExecutableCX = false;
			if(logicalTarget >= 0 && CXFrontier[logicalTarget]) {
				GateNode * cx = CXFrontier[logicalTarget];
				assert(cx->target >= 0);
				assert(cx->control >= 0);

				// keha: added outer conditional to avoid heap overflow of node->env.couplingDistances
				if (node->laq[cx->target] >= 0 && node->laq[cx->control] >= 0) {
					int currentDist = node->env.couplingDistances[node->laq[cx->control] * node->env.numPhysicalQubits +
																  node->laq[cx->target]];
					node->swapQubits(target, control);
					int hypotheticDist = node->env.couplingDistances[node->laq[cx->control] * node->env.numPhysicalQubits + node->laq[cx->target]];
					node->swapQubits(target, control);

					if(hypotheticDist < currentDist) {
						helpsCX = true;
					} else if(hypotheticDist > currentDist && currentDist == 1) {
						//hurtsExecutableCX = true;
					}
				}
			}
			if(logicalControl >= 0 && CXFrontier[logicalControl]) {
				GateNode * cx = CXFrontier[logicalControl];
				assert(cx->target >= 0);
				assert(cx->control >= 0);

				// keha: added outer conditional to avoid heap overflow of node->env.couplingDistances
				if (node->laq[cx->target] >= 0 && node->laq[cx->control] >= 0) {
					int currentDist = node->env.couplingDistances[node->laq[cx->control] * node->env.numPhysicalQubits +
																  node->laq[cx->target]];
					node->swapQubits(target, control);
					int hypotheticDist = node->env.couplingDistances[node->laq[cx->control] * node->env.numPhysicalQubits + node->laq[cx->target]];
					node->swapQubits(target, control);

					if(hypotheticDist < currentDist) {
						helpsCX = true;
					} else if(hypotheticDist > currentDist && currentDist == 1) {
						//hurtsExecutableCX = true;
					}
				}
			}
			
			bool good = (node->cycle < -1 || (!occupied[target] && !occupied[control])) &&
						(helpsCX);// && !hurtsExecutableCX);
			bool dependsOnSomething = false;
			
			bool usesLogicalQubit = false;
			if(good && logicalTarget >= 0) {
				usesLogicalQubit = true;
			}
			if(good && logicalControl >= 0) {
				usesLogicalQubit = true;
			}
			good = good && usesLogicalQubit;

			// check if qubits for this swap would actually get used by the remaining
			// gates. If not, mark it as bad.
			bool usesUsefulLogicalQubit = false;
			if(good) {
				if(logicalTarget >= 0) {
					ScheduledGate * t = node->lastNonSwapGate[logicalTarget];
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
					ScheduledGate * c = node->lastNonSwapGate[logicalControl];
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
			if(good && nodesSize > 0 && !dependsOnSomething && !onReadyFrontier[target] && !onReadyFrontier[control]) {
				//good = false;
			}
			if(good) {
				possibleGates.push_back(g);
			}
		}
		
		std::priority_queue<PriotityQueueType, std::vector<PriotityQueueType>, CmpNodeCost> tempNodes;
		std::size_t numPushed = 0;
		
		if (possibleGates.size() >= 64) {
			// TODO: modify approach to support >= 64 gates.
			throw std::runtime_error("FATAL ERROR: current implementation cannot handle more than 63 possible gates.");
		}

		unsigned long long numIters = 1LL << possibleGates.size();
		
		for(unsigned long long x = 0; x < numIters; x++) {
			std::shared_ptr<Node> child = Node::prepChild(node.get());
			bool good = true;
			//schedule different subset of swaps and 2-qubit gates than for previous child nodes
			for(unsigned int y = 0; good && y < possibleGates.size(); y++) {
				if(x & (1LL << y)) {
					if(node->cycle >= -1) {
						good = child->scheduleGate(possibleGates[y]);
					} else {
						good = child->swapQubits(possibleGates[y]->target, possibleGates[y]->control);
					}
				}
			}
			
			if(x == 0) {
				if(guaranteedGates.empty() && !hasBusyQubits) {
					continue;
				}
			}
			
			if(good) {
				//schedule as many of the 1-cycle ready gates as we can:
				for(unsigned int y = 0; good && y < guaranteedGates.size(); y++) {
					good = child->scheduleGate(guaranteedGates[y]);
					assert(good);
				}
				
				child->cost = node->env.cost.getCost(*child);
				
				//if(!this->K || this->K >= numIters) {
				//	if(!nodes.push(child)) {
				//		delete child;
				//	}
				//} else {
					tempNodes.push(std::make_tuple(numPushed++, child));
					
					//If priority queue is overfilled, delete extra node:
					if(tempNodes.size() > this->K) {
						tempNodes.pop();
					}
					assert(tempNodes.size() <= this->K);
				//}
			}
		}
		
		//if(this->K && this->K < numIters) {
			//Push top K into main priority queue
			long counter = this->K;
			while(counter > 0 && !tempNodes.empty()) {
				auto child = std::get<1>(tempNodes.top());
				tempNodes.pop();
				if(nodes.push(child)) {
					counter--;
				} else {
					std::cerr
							<< "SANITY CHECK ERROR: looks like not all pushed nodes will go through for top-k after all; I need to redo the optimization that handles the overfill\n";
				}
			}
			
			//cleanup the discarded children
			while(!tempNodes.empty()) {
				tempNodes.pop();
			}
		//}
		
		return true;
	}
	
	std::unique_ptr<Expander> clone() const override {
		return std::unique_ptr<Expander>(new GreedyTopK(*this));
	}
};

}

#endif
