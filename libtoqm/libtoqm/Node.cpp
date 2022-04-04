#include "Node.hpp"

#include <libtoqm/Environment.hpp>

#include <set>
#include <cassert>
#include <iostream>
#include <string>
#include <stdexcept>
#include <sstream>

namespace toqm {

//const int MAX_QUBITS = 20;
int GLOBALCOUNTER = 0;

Node::Node(Environment& environment) : env(environment) {
	for(int x = 0; x < MAX_QUBITS; x++) {
		qal[x] = x;
		laq[x] = x;
		lastNonSwapGate[x] = NULL;
		lastGate[x] = NULL;
		lastNonZeroLatencyGate[x] = NULL;
	}
	this->cost = 0;
	this->dead = false;
}

Node::~Node() = default;

bool isSwapGate(const GateNode * gate) {
	return gate->name == "swap" || gate->name == "SWAP";
}

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

	// If a control is specified, make sure the current layout maps it to
	// a physical qubit.
	// If control is present but unmapped, we have a problem because we might
	// schedule a 2q gate on physical bits that aren't connected.
	bool mappedControl = gate->control < 0 || physicalControl >= 0;
	assert(mappedControl);

	if (physicalControl >= 0) {
		assert(physicalTarget >= 0);

		if (env.couplings.count(std::make_pair(physicalControl, physicalTarget)) <= 0) {
			if (env.couplings.count(std::make_pair(physicalTarget, physicalControl)) <= 0) {
				// Cannot schedule 2q gate on unconnected qubits!
				return false;
			}
		}
	}

	// A 1q gate might be scheduled even if the current layout doesn't map its
	// target to a physical qubit (we can map it retro-actively).
	bool knownZeroLatency = false;
	if (physicalTarget >= 0) {
		// We can check the latency of this gate, since it's fully mapped
		// to physical qubits.
		auto latency = env.latency.getLatency(gate->name, (gate->control >= 0 ? 2 : 1), physicalTarget, physicalControl);
		knownZeroLatency = latency == 0;
	}

	if (!knownZeroLatency) {
		// Gate is not known to be zero-latency, so we should only schedule it
		// if physical qubits aren't busy, or it is a 1q gate with a target
		// that isn't currently mapped to a specific physical qubit.

		if (physicalControl >= 0) {
			int busyControl = this->busyCycles(physicalControl);
			if(busyControl > 0 && busyControl > (int) timeOffset) {
				return false;
			}
		}

		if (physicalTarget >= 0) {
			int busyTarget = this->busyCycles(physicalTarget);
			if(busyTarget > 0 && busyTarget > (int) timeOffset) {
				return false;
			}
		}
	}

	this->scheduleGate(gate, physicalTarget, physicalControl, timeOffset);
	return true;
}

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
		if (sg->latency != 0) {
			this->lastNonZeroLatencyGate[physicalControl] = sg.get();
		}
	}
	if(sg->gate->control >= 0 && !isSwap) {
		this->lastNonSwapGate[sg->gate->control] = sg.get();
	}
	
	if(physicalTarget >= 0) {
		this->lastGate[physicalTarget] = sg.get();
		if (sg->latency != 0) {
			this->lastNonZeroLatencyGate[physicalTarget] = sg.get();
		}
	}
	if(sg->gate->target >= 0 && !isSwap) {
		this->lastNonSwapGate[sg->gate->target] = sg.get();
	}
	
	if(!isSwap) {
		if(this->readyGates.erase(gate) != 1) {
			std::stringstream ss {};
			ss << "FATAL ERROR: unable to remove scheduled gate from ready list.\n";
			ss << "\tGate name: " << gate->name << "\n";
			ss << "\tTime offset: " << timeOffset << "\n";
			throw std::runtime_error(ss.str());
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

		bool mappedControlOrSingleQ = gn->control < 0 || laq[gn->control] >= 0;
		bool mappedTarget = laq[gn->target] >= 0;
		if (!mappedControlOrSingleQ || !mappedTarget) {
			// Can't schedule this now since it's 2q and either the control
			// or target isn't mapped.
			continue;
		}

		// Try to recursively schedule this gate in the same cycle as the gate that unblocked it.
		// This will only work if both:
		// 1) it's 0-latency or only 0-latency gates have been scheduled on its qubits this cycle so far.
		// 2) it is compatible with the current layout.
		this->scheduleGate(gn, timeOffset);
	}
}

//prepares a new child node (without scheduling any more gates)
std::unique_ptr<Node> Node::prepChild(Node* parent) {
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
		child->lastNonZeroLatencyGate[x] = parent->lastNonZeroLatencyGate[x];
	}
	child->cost = 0;//Remember to calculate cost in expander, *after* it's done scheduling new gates for this node //child->cost = env->cost->getCost(child);
	
	return child;
}

}
