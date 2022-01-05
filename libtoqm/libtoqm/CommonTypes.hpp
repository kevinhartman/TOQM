#ifndef TOQM_COMMONTYPES_HPP
#define TOQM_COMMONTYPES_HPP

#include <set>
#include <string>
#include <utility>

namespace toqm {

struct CouplingMap {
	unsigned int numPhysicalQubits;
	std::set<std::pair<int, int>> edges;
};

struct GateOp {
	std::string type;
	int target;
	int control;
};

}

#endif //TOQM_COMMONTYPES_HPP
