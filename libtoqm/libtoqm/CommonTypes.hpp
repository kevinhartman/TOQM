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
	/**
	 * Construct a 0-qubit gate.
	 */
	GateOp(int uid, std::string type) : uid(uid), type(std::move(type)) {}
	
	/**
	 * Construct a 1-qubit gate.
	 */
	GateOp(int uid, std::string type, int target) : uid(uid), type(std::move(type)), target(target) {}
	
	/**
	 * Construct a 2-qubit gate.
	 */
	GateOp(int uid, std::string type, int control, int target) : uid(uid), type(std::move(type)), control(control), target(target) {}
	
	int uid;
	std::string type;
	int control = -1;
	int target = -1;
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
	std::vector<int> laq;
	std::vector<int> inferredQal;
	std::vector<int> inferredLaq;
	int idealCycles;
	int numPopped;
	std::string filterStats;
};

struct LatencyDescription {
	/**
	 * Construct an optimistic latency for all gates with the specified number of qubits.
	 */
	LatencyDescription(int numQubits, int latency) : numQubits(numQubits), latency(latency) {}
	
	/**
	 * Construct an optimistic latency for all gates with the specified number of qubits
	 * with the same specified type name.
	 */
	LatencyDescription(int numQubits, std::string type, int latency) : numQubits(numQubits), type(std::move(type)), latency(latency) {}
	
	/**
	 * Construct a specific latency for a gate between the two specified Qubits.
	 */
	LatencyDescription(int numQubits, std::string type, int control, int target, int latency) : numQubits(numQubits), type(std::move(type)), control(control), target(target), latency(latency) {}
	
	int numQubits;
	int latency;
	
	std::string type = "";
	int control = -1;
	int target = -1;
};

}

#endif //TOQM_COMMONTYPES_HPP
