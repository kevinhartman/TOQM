#ifndef GATENODE_HPP
#define GATENODE_HPP

#include <string>
#include <memory>

namespace toqm {

class GateNode { //part of a DAG of nodes
public:
	int uid {}; // user-provided ID for tracking
	std::string name{};
	int control{};//control qubit, or -1
	int target{};//target qubit
	
	int optimisticLatency{};//how many cycles this gate takes, assuming it uses the fastest physical qubit(s)
	int criticality{};//length (time) of circuit from here until furthest leaf
	
	//note that the following variables will not take into account inserted SWP gates
	GateNode* controlChild {};//gate which depends on this one's control qubit, or NULL
	GateNode* targetChild {};//gate which depends on this one's target qubit, or NULL
	GateNode* controlParent {};//prior gate which this control qubit depends on, or NULL
	GateNode* targetParent {};//prior gate which this target qubit depends on, or NULL
	
	GateNode* nextControlCNOT {};//next 2-qubit gate which depends on this one's control, or -1
	GateNode* nextTargetCNOT {};//next 2-qubit gate which depends on this one's target, or -1
};

}

#endif
