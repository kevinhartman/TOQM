#ifndef LIBTOQM_COMPARISONHELPERS_HPP
#define LIBTOQM_COMPARISONHELPERS_HPP

namespace toqm {

class ScheduledGateOp;

bool operator==(ScheduledGateOp const & lhs, ScheduledGateOp const & rhs);
}


#endif//LIBTOQM_COMPARISONHELPERS_HPP
