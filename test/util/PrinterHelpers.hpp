#ifndef LIBTOQM_PRINTERS_HPP
#define LIBTOQM_PRINTERS_HPP

#include <ostream>
#include <vector>

namespace toqm {

class ScheduledGateOp;

namespace test {

//Print a node's scheduled gates
//returns how many cycles the node takes to complete all its gates
int printNode(std::ostream & stream, const std::vector<ScheduledGateOp>& gateStack);

}
}

#endif//LIBTOQM_PRINTERS_HPP
