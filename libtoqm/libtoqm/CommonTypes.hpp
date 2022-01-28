#ifndef TOQM_COMMONTYPES_HPP
#define TOQM_COMMONTYPES_HPP

#include <set>
#include <string>
#include <utility>
#include <vector>

namespace toqm {

struct CouplingMap {
	unsigned int numPhysicalQubits;
	std::set<std::pair<int, int>> edges;
};

struct GateOp {
	std::string type;
	int target; // set just target for 1-qubit gate
	int control;
};

struct ScheduledGateOp {
	GateOp gateOp;
	int physicalTarget;
	int physicalControl;
	int cycle; //cycle when this gate started
	int latency;
};

struct ToqmResult {
	std::vector<ScheduledGateOp> scheduledGates;
	int remainingInQueue;
	int numPhysicalQubits;
	int numLogicalQubits;
	std::vector<char> laq;
	std::vector<char> inferredQal;
	std::vector<char> inferredLaq;
	int idealCycles;
	int numPopped;
	std::string filterStats;
};

}

#endif //TOQM_COMMONTYPES_HPP
