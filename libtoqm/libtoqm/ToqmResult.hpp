#ifndef TOQM_TOQMRESULT_HPP
#define TOQM_TOQMRESULT_HPP

#include "CommonTypes.hpp"

#include <vector>
#include <string>

namespace toqm {

class ToqmResult {
public:
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

#endif //TOQM_TOQMRESULT_HPP
