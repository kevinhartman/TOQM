#include "ComparisonHelpers.hpp"

#include <libtoqm/CommonTypes.hpp>

#include <tuple>

namespace toqm {
bool operator==(ScheduledGateOp const & lhs, ScheduledGateOp const & rhs) {
	return std::tie(lhs.cycle, lhs.latency, lhs.physicalControl, lhs.physicalTarget) ==
		   std::tie(rhs.cycle, rhs.latency, rhs.physicalControl, rhs.physicalTarget);
}
}