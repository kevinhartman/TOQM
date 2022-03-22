#include "ScheduledGateStack.hpp"

#include <libtoqm/ScheduledGate.hpp>

namespace toqm {

ScheduledGateStack::ScheduledGateStack() = default;

ScheduledGateStack::~ScheduledGateStack() {
	std::shared_ptr<ScheduledGateStack> ptr = std::move(this->next);

	while (ptr && ptr.use_count() == 1) {
		ptr = std::move(ptr->next);
	}
}

std::unique_ptr<ScheduledGateStack> ScheduledGateStack::push(const std::shared_ptr<ScheduledGateStack> & head, const std::shared_ptr<ScheduledGate>& newVal) {
	return std::unique_ptr<ScheduledGateStack>(new ScheduledGateStack(newVal, head, head->size + 1));
}

}