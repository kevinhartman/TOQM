#ifndef TOQM_TOQMMAPPER_HPP
#define TOQM_TOQMMAPPER_HPP

#include "CommonTypes.hpp"

#include <string>
#include <vector>
#include <memory>
#include <set>
#include <utility>
#include <string>
#include <functional>

namespace toqm {

class Expander;

class CostFunc;

class Latency;

class Queue;

class NodeMod;

class Filter;

using QueueFactory = std::function<std::unique_ptr<Queue>()>;

class ToqmMapper {
public:
	/**
	 * Construct a reusable TOQM mapper object that performs the TOQM algorithm
	 * using the specified configuration.
	 * @param node_queue A factory function that produces Queue instances for
	 * each run of the mapper.
	 * @param expander The `Expander` implementation used to determine each
	 * node's children (the next possible circuit states given a current state).
	 * @param cost_func The `CostFunc` implementation used to determine the
	 * associated cost of a given node (circuit state).
	 * @param latency The `Latency` implementation used to determine the number
	 * of cycles a given gate will take to complete its execution.
	 * @param node_mods A sequence of `NodeMod` implementations.
	 * @param filters A sequence of `Filter` implementations used to prune
	 * redundant or otherwise uninteresting nodes from the search space.
	 */
	explicit ToqmMapper(
			QueueFactory node_queue,
			std::unique_ptr<Expander> expander,
			std::unique_ptr<CostFunc> cost_func,
			std::unique_ptr<Latency> latency,
			std::vector<std::unique_ptr<NodeMod>> node_mods,
			std::vector<std::unique_ptr<Filter>> filters
	);
	
	/**
	 * Destructor.
	 */
	~ToqmMapper();
	
	/**
	 * Set the number of cycles to spend up front searching for an initial mapping.
	 * If `-1`, will use longest path between any two nodes without going through
	 * any given node more than once (this is a complete search and is the max
	 * value that can have an effect).
	 * @param initial_search_cycles The number of cycles.
	 */
	void setInitialSearchCycles(int initial_search_cycles);
	
	void setRetainPopped(int retain_popped);
	
	void setInitialMappingQal(const std::vector<char>& init_qal);
	
	void setInitialMappingLaq(const std::vector<char>& init_laq);
	
	/**
	 * Clear any initial mappings that were previously set.
	 */
	void clearInitialMapping();
	
	/**
	 * Set verbose output mode (currently writes to stderr).
	 * @param verbose True to enable verbose logging.
	 *
	 * TODO: if verbose is set, ToqmMapper reads from cin... remove that!
	 */
	static void setVerbose(bool verbose);
	
	/**
	 * Run the TOQM algorithm.
	 * @param gates The topologically ordered gates that define the circuit.
	 * @param num_qubits
	 * @param coupling_map
	 * @return The result object, which contains the transformed circuit (with inserted SWAPS),
	 * the initial mapping (either calculated or the one the user provided), and various
	 * statistics describing the run.
	 *
	 * 	TODO: remove maxQubits. It's used internally to prealloc arrays, but we can use maps instead.
	 */
	std::unique_ptr<ToqmResult>
	run(const std::vector<GateOp> & gates, std::size_t num_qubits, const CouplingMap & coupling_map) const;

private:
	class Impl;
	
	std::unique_ptr<Impl> impl;
};

}

#endif //TOQM_TOQMMAPPER_HPP
