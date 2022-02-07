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

class ToqmMapper {
public:
	/**
	 * Construct a reusable TOQM mapper object that performs the TOQM algorithm
	 * using the specified configuration.
	 * @param node_queue A template `Queue` instance that will be cloned and used
	 * for each run of the mapper.
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
			const Queue & node_queue,
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
	
	void setRetainPopped(int retain_popped);
	
	/**
	 * Set verbose output mode (currently writes to stderr).
	 * @param verbose True to enable verbose logging.
	 *
	 * TODO: if verbose is set, ToqmMapper reads from cin... remove that!
	 */
	static void setVerbose(bool verbose);
	
	/**
	 * Run the TOQM algorithm.
	 * The initial layout is determined automatically using a cycle limit
	 * equal to the diameter of the couping map, and is available in the
	 * result.
	 * @param gates The topologically ordered gates that define the circuit.
	 * @param num_qubits
	 * @param coupling_map The coupling map describing the target hardware.
	 * @return The result object, which contains the transformed circuit (with inserted SWAPS),
	 * the initial mapping (either calculated or the one the user provided), and various
	 * statistics describing the run.
	 *
	 * 	TODO: remove maxQubits. It's used internally to prealloc arrays, but we can use maps instead.
	 */
	std::unique_ptr<ToqmResult>
	run(const std::vector<GateOp> & gates, std::size_t num_qubits, const CouplingMap & coupling_map) const;
	
	/**
	 * Run the TOQM algorithm.
	 * The initial layout is determined automatically using the specified cycle limit,
	 * and is available in the result.
	 * @param gates
	 * @param num_qubits
	 * @param coupling_map
	 * @param initial_search_cycles The number of cycles. If `-1`, uses the
	 * longest path between any two nodes without going through any given node more
	 * than once (this is a complete search and is the max value that can have an
	 * effect).
	 * @return
	 */
	std::unique_ptr<ToqmResult>
	run(const std::vector<GateOp> & gates, std::size_t num_qubits, const CouplingMap & coupling_map, int initial_search_cycles) const;
	
	/**
	 * Run the TOQM algorithm using the specified initial layout.
	 * @param gates
	 * @param num_qubits
	 * @param coupling_map
	 * @param initial_search_cycles If non-zero, the initial layout is determined automatically
	 * with `init_qal` as a starting point, using up to this many cycles.
	 * @param init_qal
	 * @return
	 */
	std::unique_ptr<ToqmResult>
	run(const std::vector<GateOp> & gates, std::size_t num_qubits, const CouplingMap & coupling_map, int initial_search_cycles, const std::vector<int> & init_qal) const;

private:
	class Impl;
	
	std::unique_ptr<Impl> impl;
};

}

#endif //TOQM_TOQMMAPPER_HPP
