#ifndef LIBTOQM_PRINTERS_HPP
#define LIBTOQM_PRINTERS_HPP

#include <ostream>
#include <vector>

namespace toqm {

class ScheduledGateOp;
std::ostream &operator<<(std::ostream &os, const std::vector<ScheduledGateOp> & gates);

}

#endif//LIBTOQM_PRINTERS_HPP
