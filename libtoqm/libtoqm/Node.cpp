#include "Node.hpp"

#include <libtoqm/Environment.hpp>

#include <set>
#include <cassert>
#include <iostream>
#include <string>

namespace toqm {

//const int MAX_QUBITS = 20;
int GLOBALCOUNTER = 0;

Node::Node(Environment& environment) : env(environment) {
	for(int x = 0; x < MAX_QUBITS; x++) {
		qal[x] = x;
		laq[x] = x;
		lastNonSwapGate[x] = NULL;
		lastGate[x] = NULL;
	}
	this->cost = 0;
	this->dead = false;
}

Node::~Node() = default;

bool isSwapGate(const GateNode * gate) {
	return gate->name == "swap" || gate->name == "SWAP";
}

//schedule a gate, or return false if it conflicts with an active gate
//the gate parameter uses logical qubits (except in swaps); this function determines physical locations based on prior swaps
//the timeOffset can be used if we want to schedule a gate to start X cycles in the future
//this function adjusts qubit map when scheduling a swap
bool Node::scheduleGate(GateNode* gate, unsigned int timeOffset) {
	bool isSwap = isSwapGate(gate);

	int physicalControl = gate->control;
	int physicalTarget = gate->target;
	if(!isSwap) {
		if(physicalControl >= 0) {
			physicalControl = laq[physicalControl];
		}
		if(physicalTarget >= 0) {
			physicalTarget = laq[physicalTarget];
		}
	}

	int busyControl = this->busyCycles(physicalControl);
	if(physicalControl >= 0 && busyControl > 0 && busyControl > (int) timeOffset) {
		return false;
	}
	int busyTarget = this->busyCycles(physicalTarget);
	if(physicalTarget >= 0 && busyTarget > 0 && busyTarget > (int) timeOffset) {
		return false;
	}

	this->scheduleGate(gate, physicalTarget, physicalControl, timeOffset);
	return true;
}

/**
 * Schedule gate on the designated physical target and control bits at the designated offset.
 * Scheduling validation is NOT performed!
 * @param gate
 * @param physicalTarget
 * @param physicalControl
 * @param timeOffset
 */
void Node::scheduleGate(GateNode* gate, int physicalTarget, int physicalControl, unsigned int timeOffset) {
	bool isSwap = isSwapGate(gate);

	std::vector<GateNode *> unblockedGates {};
	unblockedGates.reserve(2);

	if(!isSwap) {
		//if appropriate, add double-child to ready gates
		if(gate->controlChild && gate->controlChild == gate->targetChild) {
			unblockedGates.push_back(gate->controlChild);
		}
		//if appropriate, add control child to ready gates
		if(gate->controlChild && gate->controlChild != gate->targetChild) {
			if(gate->controlChild->control < 0) {//single-qubit gate
				unblockedGates.push_back(gate->controlChild);
			} else {
				int childParentBit;
				GateNode * otherParent;
				if(gate->controlChild->controlParent == gate) {
					otherParent = gate->controlChild->targetParent;
					if(gate->controlChild->targetParent) {
						childParentBit = gate->controlChild->target;
					} else {
						childParentBit = -1;
					}
				} else {
					assert(gate->controlChild->targetParent == gate);
					otherParent = gate->controlChild->controlParent;
					if(gate->controlChild->controlParent) {
						childParentBit = gate->controlChild->control;
					} else {
						childParentBit = -1;
					}
				}
				if(childParentBit < 0 || (this->lastNonSwapGate[childParentBit] &&
										  this->lastNonSwapGate[childParentBit]->gate == otherParent)) {
					unblockedGates.push_back(gate->controlChild);
				}
			}
		}
		//if appropriate, add target child to ready gates
		if(gate->targetChild && gate->controlChild != gate->targetChild) {
			if(gate->targetChild->control < 0) {//single-qubit gate
				unblockedGates.push_back(gate->targetChild);
			} else {
				int childParentBit;
				GateNode * otherParent;
				if(gate->targetChild->controlParent == gate) {
					otherParent = gate->targetChild->targetParent;
					if(gate->targetChild->targetParent) {
						childParentBit = gate->targetChild->target;
					} else {
						childParentBit = -1;
					}
				} else {
					assert(gate->targetChild->targetParent == gate);
					otherParent = gate->targetChild->controlParent;
					if(gate->targetChild->controlParent) {
						childParentBit = gate->targetChild->control;
					} else {
						childParentBit = -1;
					}
				}
				if(childParentBit < 0 || (this->lastNonSwapGate[childParentBit] &&
										  this->lastNonSwapGate[childParentBit]->gate == otherParent)) {
					unblockedGates.push_back(gate->targetChild);
				}
			}
		}
	}
	
	auto sg = std::shared_ptr<ScheduledGate>(new ScheduledGate(gate, this->cycle + timeOffset));
	sg->physicalControl = physicalControl;
	sg->physicalTarget = physicalTarget;
	sg->latency = env.latency.getLatency(sg->gate->name, (sg->physicalControl >= 0 ? 2 : 1), sg->physicalTarget,
										  sg->physicalControl);
	
	if(physicalControl >= 0) {
		this->lastGate[physicalControl] = sg.get();
	}
	if(sg->gate->control >= 0 && !isSwap) {
		this->lastNonSwapGate[sg->gate->control] = sg.get();
	}
	
	if(physicalTarget >= 0) {
		this->lastGate[physicalTarget] = sg.get();
	}
	if(sg->gate->target >= 0 && !isSwap) {
		this->lastNonSwapGate[sg->gate->target] = sg.get();
	}
	
	if(!isSwap) {
		if(this->readyGates.erase(gate) != 1) {
			std::cerr << "FATAL ERROR: unable to remove scheduled gate from ready list.\n";
			std::cerr << "\tGate name: " << gate->name << "\n";
			std::cerr << "\tTime offset: " << timeOffset << "\n";
			assert(false);
		}
		this->numUnscheduledGates--;
	}
	
	this->scheduled = ScheduledGateStack::push(this->scheduled, sg);
	
	//adjust qubit map
	if(isSwap) {
		if(qal[physicalControl] >= 0 && qal[physicalTarget] >= 0) {
			std::swap(laq[(int) qal[physicalControl]], laq[(int) qal[physicalTarget]]);
		} else if(qal[physicalControl] >= 0) {
			laq[(int) qal[physicalControl]] = physicalTarget;
		} else if(qal[physicalTarget] >= 0) {
			laq[(int) qal[physicalTarget]] = physicalControl;
		} else {
			assert(false);
		}
		std::swap(qal[physicalControl], qal[physicalTarget]);
	}

	// keha: add unblocked gates to ready gates and schedule any 0-duration gates immediately
	for (GateNode * gn : unblockedGates) {
		readyGates.insert(gn);

		if (isSwapGate(gn)) {
			// Skip swaps. 0-latency swaps aren't supported since nodes
			// can only have a single LAQ/QAL mapping.
			continue;
		}

		// Determine if this newly unblocked gate should be scheduled immediately
		// in the current cycle.
		auto gnPhysicalControl = gn->control >= 0 ? laq[gn->control] : -1;
		auto gnPhysicalTarget = laq[gn->target];
		auto latency = env.latency.getLatency(gn->name, gnPhysicalControl >= 0 ? 2 : 1, gnPhysicalTarget, gnPhysicalControl);

		// Special case for cycle == 0.
		// In the 0th cycle only, the unblocked gate's bits might only
		// have 0-duration gates scheduled on them so far.
		// In that case, we should always schedule it immediately even
		// if it has a non-zero duration, else it'll incorrectly end
		// up in cycle 1.
		bool emptyFirstCycles = true;
		if (cycle == 0) {
			// Determine if a non-zero duration instruction has already been scheduled
			// on these bits.
			auto sgs = scheduled.get();
			while (sgs->value) {
				auto usesControl = gnPhysicalControl >= 0 && (sgs->value->physicalControl == gnPhysicalControl || sgs->value->physicalTarget == gnPhysicalControl);
				auto usesTarget = sgs->value->physicalControl == gnPhysicalTarget || sgs->value->physicalTarget == gnPhysicalTarget;

				if ((usesControl || usesTarget) && sgs->value->cycle == 0 && sgs->value->latency > 0) {
					emptyFirstCycles = false;
					break;
				}

				sgs = sgs->next.get();
			}
		} else {
			emptyFirstCycles = false;
		}

		if (latency == 0 || emptyFirstCycles) {
			// 0-latency gate, or 0th cycle special case.
			// Recursively schedule this gate in the same cycle as the gate that unblocked it.
			this->scheduleGate(gn, gnPhysicalTarget, gnPhysicalControl, timeOffset);
		}
	}
}

//prepares a new child node (without scheduling any more gates)
std::unique_ptr<Node> Node::prepChild(const std::shared_ptr<Node>& parent) {
	auto child = std::unique_ptr<Node>(new Node(parent->env));
	child->numUnscheduledGates = parent->numUnscheduledGates;
	child->parent = parent;
	child->cycle = parent->cycle + 1;
	child->readyGates = parent->readyGates;//note: this actually produces a separate copy
	child->scheduled = parent->scheduled;
	for(int x = 0; x < parent->env.numPhysicalQubits; x++) {
		child->qal[x] = parent->qal[x];
		child->laq[x] = parent->laq[x];
		child->lastNonSwapGate[x] = parent->lastNonSwapGate[x];
		child->lastGate[x] = parent->lastGate[x];
	}
	child->cost = 0;//Remember to calculate cost in expander, *after* it's done scheduling new gates for this node //child->cost = env->cost->getCost(child);
	
	return child;
}

}
