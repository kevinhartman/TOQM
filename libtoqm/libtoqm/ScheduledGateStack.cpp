#include "ScheduledGateStack.hpp"

#include <libtoqm/ScheduledGate.hpp>

namespace toqm {

ScheduledGateStack::ScheduledGateStack() = default;

ScheduledGateStack::~ScheduledGateStack() {
	while (this->next != nullptr && this->next.use_count() == 1) {
		this->next = this->next->next;
	}
}

//Create a new LinkedStack such that this one is its second element
//Returns the new LinkedStack
std::unique_ptr<ScheduledGateStack> ScheduledGateStack::push(const std::shared_ptr<ScheduledGateStack> & head, const std::shared_ptr<ScheduledGate>& newVal) {
	return std::unique_ptr<ScheduledGateStack>(new ScheduledGateStack(newVal, head, head->size + 1));
}

}