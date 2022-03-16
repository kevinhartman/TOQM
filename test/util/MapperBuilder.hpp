#ifndef LIBTOQM_MAPPERBUILDER_HPP
#define LIBTOQM_MAPPERBUILDER_HPP

#include <memory>
#include <vector>

namespace toqm {

class Queue;
class Expander;
class CostFunc;
class Latency;
class NodeMod;
class Filter;
class ToqmMapper;

namespace test {

class MapperBuilder {
public:
	MapperBuilder();
	std::unique_ptr<toqm::ToqmMapper> build();

	static MapperBuilder forSmallCircuits();
	static MapperBuilder forLargeCircuits();

	// Public fields to configure builder
	std::unique_ptr<toqm::Queue> Queue;
	std::unique_ptr<toqm::Expander> Expander;
	std::unique_ptr<toqm::CostFunc> CostFunc;
	std::unique_ptr<toqm::Latency> Latency;
	std::unique_ptr<toqm::NodeMod> NodeMod;
	std::unique_ptr<toqm::Filter> Filter1;
	std::unique_ptr<toqm::Filter> Filter2;
	int RetainPopped {};

private:
	std::vector<std::unique_ptr<toqm::NodeMod>> NodeMods() const;
	std::vector<std::unique_ptr<toqm::Filter>> Filters() const;
};

}
}

#endif//LIBTOQM_MAPPERBUILDER_HPP
