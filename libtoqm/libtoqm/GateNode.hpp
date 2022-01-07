#ifndef GATENODE_HPP
#define GATENODE_HPP

#include <string>

using namespace std;

namespace toqm {

class GateNode { //part of a DAG of nodes
public:
	string name;
	int control;//control qubit, or -1
	int target;//target qubit
	
	int optimisticLatency;//how many cycles this gate takes, assuming it uses the fastest physical qubit(s)
	int criticality;//length (time) of circuit from here until furthest leaf
	
	//note that the following variables will not take into account inserted SWP gates
	std::shared_ptr<GateNode> controlChild {};//gate which depends on this one's control qubit, or NULL
	std::shared_ptr<GateNode> targetChild {};//gate which depends on this one's target qubit, or NULL
	std::shared_ptr<GateNode> controlParent {};//prior gate which this control qubit depends on, or NULL
	std::shared_ptr<GateNode> targetParent {};//prior gate which this target qubit depends on, or NULL
	
	std::shared_ptr<GateNode> nextControlCNOT {};//next 2-qubit gate which depends on this one's control, or -1
	std::shared_ptr<GateNode> nextTargetCNOT {};//next 2-qubit gate which depends on this one's target, or -1
};

}

#endif
