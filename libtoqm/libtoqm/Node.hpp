#ifndef NODE_HPP
#define NODE_HPP

#include "GateNode.hpp"
#include "ScheduledGate.hpp"
#include "ScheduledGateStack.hpp"

#include <cassert>
#include <iostream>
#include <memory>
#include <set>
#include <tuple>

namespace toqm {

class Environment;

class Queue;

//Warning: you may need to run make clean after changing MAX_QUBITS
const int MAX_QUBITS = 127;
//extern int GLOBALCOUNTER;

/**
 * Used to sort gate nodes by their UID first, and secondarily by
 * their pointer's memory address if UID is not unique (in which
 * case the ordering will be non-deterministic!).
 */
struct SortByGateNode
{
	bool operator ()(const GateNode* lhs, const GateNode* rhs) const
	{
		return std::tie(lhs->uid, lhs) < std::tie(rhs->uid, rhs);
	}
};

class Node {
public:
	Environment & env;//object with functions/data shared by all nodes
	Node * parent;//node from which this one expanded
	int cycle {};//current cycle
	int cost {};//the node's cost (in cycles)
	int cost2 = 0;//used as tiebreaker in some places
	
	int numUnscheduledGates {};//the number of gates from the original circuit that are not yet part of this node's schedule
	bool expanded = false;//whether or not this node has been popped from the queue
	bool dead = false;//where or not this node has been marked as 'dead' by a filter
	
	//int debugID = GLOBALCOUNTER++;
	
	char qal[MAX_QUBITS]{};//qubit mapping, physical to logical.
	char laq[MAX_QUBITS]{};//qubit mapping (inverted), logical to physical.
	
	ScheduledGate* lastNonSwapGate[MAX_QUBITS]{};//last scheduled non-swap gate per LOGICAL qubit
	ScheduledGate* lastGate[MAX_QUBITS]{};//last scheduled gate per PHYSICAL qubit
	ScheduledGate* lastNonZeroLatencyGate[MAX_QUBITS]{};//last non-zero latency scheduled gate per PHYSICAL qubit

	
	//the number of cycles until the specified physical qubit is available
	inline int busyCycles(int physicalQubit) const {
		auto & sg = this->lastNonZeroLatencyGate[physicalQubit];
		if(!sg) return 0;
		int cycles = sg->cycle + sg->latency - this->cycle;
		if(cycles < 0) return 0;
		return cycles;
	}

	std::set<GateNode*, SortByGateNode> readyGates;//set of gates in DAG whose parents have already been scheduled
	
	std::shared_ptr<ScheduledGateStack> scheduled{};//list of scheduled gates. Warning: this linked list's data overlaps with the same list in parent node
	
	explicit Node(Environment& environment);
	
	~Node();
	
	//swap two physical qubits in qubit map, without scheduling a gate
	inline bool swapQubits(int physicalControl, int physicalTarget) {
		if(qal[physicalControl] < 0 && qal[physicalTarget] < 0) {
			return false;
		} else if(qal[physicalTarget] < 0) {
			laq[(int) qal[physicalControl]] = physicalTarget;
		} else if(qal[physicalControl] < 0) {
			laq[(int) qal[physicalTarget]] = physicalControl;
		} else {
			std::swap(laq[(int) qal[physicalControl]], laq[(int) qal[physicalTarget]]);
		}
		std::swap(qal[physicalControl], qal[physicalTarget]);
		return true;
	}

	/**
	 * Schedule a gate, or return false if it conflicts with an active gate,
	 * or is not compatible with the coupling map.
	 *
	 * If gate is zero-latency for its mapped physical qubits, it will not
	 * conflict with an active gate.
	 *
	 * If gate is 2Q, its qubits must be mapped to physical qubits in the
	 * current layout. If 1Q, gate's target does not need to be mapped.
	 * However, we can't tell if such a gate is zero-latency, so in such
	 * a case, active gates will conflict.
	 *
	 * This function updates this node's layout / qubit map when scheduling
	 * swaps.
	 *
	 * @param gate The gate to schedule. Physical qubits on which to schedule
	 * the gate are determined by the mapping of the gate's logical qubits in this
	 * node's current layout. In the case of swaps, gate's qubits should be given
	 * as physical qubits rather than logical.
	 * @param timeOffset Schedule the gate timeOffset cycles in the future.
	 * @return True if the gate was scheduled, false otherwise.
	 */
	bool scheduleGate(GateNode* gate, unsigned int timeOffset = 0);
	
	//prepares a new child node (without scheduling any more gates)
	static std::unique_ptr<Node> prepChild(Node* parent);

private:
	/**
	 * Schedule gate on the designated physical target and control bits at the designated offset.
	 * Scheduling validation is NOT performed!
	 */
	void scheduleGate(GateNode* gate, int physicalTarget, int physicalControl, unsigned int timeOffset);
};

}

#endif
