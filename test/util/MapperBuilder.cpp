#include "MapperBuilder.hpp"

#include <libtoqm/libtoqm.hpp>

namespace toqm {
namespace test {

MapperBuilder::MapperBuilder() :
	Queue(std::unique_ptr<toqm::Queue>(new toqm::DefaultQueue())),
	Expander(std::unique_ptr<toqm::Expander>(new toqm::DefaultExpander())),
	CostFunc(std::unique_ptr<toqm::CostFunc>(new toqm::CXFrontier())),
	Latency(std::unique_ptr<toqm::Latency>(new toqm::Latency_1_2_6())),
	NodeMod(nullptr),
	Filter1(std::unique_ptr<toqm::Filter>(new toqm::HashFilter())),
	Filter2(std::unique_ptr<toqm::Filter>(new toqm::HashFilter2())) {}

std::unique_ptr<toqm::ToqmMapper> MapperBuilder::build() {
	return std::unique_ptr<toqm::ToqmMapper>(new toqm::ToqmMapper(
			*this->Queue,
			this->Expander->clone(),
			this->CostFunc->clone(),
			this->Latency->clone(),
			this->NodeMods(),
			this->Filters()));
}

std::vector<std::unique_ptr<toqm::NodeMod>> MapperBuilder::NodeMods() const {
	std::vector<std::unique_ptr<toqm::NodeMod>> mods{};
	if (this->NodeMod != nullptr) {
		mods.push_back(this->NodeMod->clone());
	}

	return std::move(mods);
}

std::vector<std::unique_ptr<toqm::Filter>> MapperBuilder::Filters() const {
	std::vector<std::unique_ptr<toqm::Filter>> filters{};
	if (this->Filter1 != nullptr) {
		filters.push_back(this->Filter1->clone());
	}

	if (this->Filter2 != nullptr) {
		filters.push_back(this->Filter2->clone());
	}

	return std::move(filters);
}


}
}