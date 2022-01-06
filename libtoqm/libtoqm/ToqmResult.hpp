#ifndef TOQM_TOQMRESULT_HPP
#define TOQM_TOQMRESULT_HPP

#include "Node.hpp"
#include "Queue.hpp"

#include <memory>
#include <vector>
#include <string>

namespace toqm {

class ToqmResult {
public:
	std::shared_ptr<Node> finalNode;
	std::unique_ptr<Queue> remaining;
	int numPhysicalQubits;
	int numLogicalQubits;
	std::vector<char> inferredQal;
	std::vector<char> inferredLaq;
	int idealCycles;
	int numPopped;
	std::string filterStats;
};

}

#endif //TOQM_TOQMRESULT_HPP
