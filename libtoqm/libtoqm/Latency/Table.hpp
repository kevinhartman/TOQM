#ifndef LATENCY_TABLE_HPP
#define LATENCY_TABLE_HPP

#include "libtoqm/Latency.hpp"

#include "libtoqm/CommonTypes.hpp"

#include <cassert>
#include <unordered_map>
#include <utility>
#include <tuple>
#include <iostream>

namespace toqm {

/**
 * A Latency class which uses a latency table.
 * It takes one argument: the path for the latency table text file.
 * The latency table consists of a one or more tuples.
 * Each tuple consists of five elements:
	the number of qubits,
	the gate name,
	the physical target qubit,
	the physical control qubit,
	and the latency.
 * Where appropriate, these elements may be replaced by dashes.
 * For example, consider this latency table:
	2	cx	1	0	3
	2	cx	0	1	3
	2	cx	0	2	3
	2	cx	2	3	4
	2	cx	-	-	2
	2	cy	-	-	12
	2	swp	-	-	6
	2	-	-	-	2
	1	-	-	-	1
 * In this example, the cx gate has a default latency of 2 cycles,
	but for certain physical qubits cx instead takes 3 or 4 cycles.
	The cy gate always takes 12 cycles, and the swp gate always takes 6 cycles.
	All other 2-qubit gates take 2 cycles, and all 1-qubit gates take 1 cycle.
 * If your latency table has an entry with physical qubits for some gate G, 
	then it should also have a default entry for G (i.e. an entry without physical qubits).
	Otherwise our current heuristic functions may exhibit strange behavior.
 */

class Table : public Latency {
private:
	struct key_hash : public std::unary_function<GateOp, std::size_t> {
		std::size_t operator()(const GateOp & k) const {
			return std::hash<std::string>{}(k.type) ^ k.numQubits() ^ k.target ^ k.control;
		}
	};
	
	struct key_equal : public std::binary_function<GateOp, GateOp, bool> {
		bool operator()(const GateOp & v0, const GateOp & v1) const {
			return (v0.type == v1.type &&
					v0.numQubits() == v1.numQubits() &&
					v0.target == v1.target &&
					v0.control == v1.control);
		}
	};
	
	typedef std::tuple<std::string, int> OptimisticLatencyTableKey;
	
	struct key_hash2 : public std::unary_function<OptimisticLatencyTableKey, std::size_t> {
		std::size_t operator()(const OptimisticLatencyTableKey & k) const {
			return std::hash<std::string>{}(std::get<0>(k)) ^ std::get<1>(k);
		}
	};
	
	struct key_equal2 : public std::binary_function<OptimisticLatencyTableKey, OptimisticLatencyTableKey, bool> {
		bool operator()(const OptimisticLatencyTableKey & v0, const OptimisticLatencyTableKey & v1) const {
			return (std::get<0>(v0) == std::get<0>(v1) &&
					std::get<1>(v0) == std::get<1>(v1));
		}
	};
	
	//map using gate's name, # bits, physical target, and physical control as key(s).
	std::unordered_map<GateOp, int, key_hash, key_equal> latencies;
	
	//best-case latency map when we haven't yet decided on physical qubits
	std::unordered_map<OptimisticLatencyTableKey, int, key_hash2, key_equal2> optimisticLatencies;
	
	//Parse the latency table file:
	void buildTable(const std::vector<LatencyDescription> & entries) {
		char * token;
		for (const auto & kv : entries) {
			auto & k = kv.gate;
			auto & v = kv.latency;
			
			//Don't allow entries where physical qubits are only partially specified:
			assert(k.numQubits() < 2 || (k.target != -1 && k.control != -1));
			
			//Don't allow duplicate entries
			auto search = latencies.find(k);
			assert(search == latencies.end());
			
			latencies.emplace(k, v);
			
			//record best-case latency for this gate regardless of physical qubits
			if(!k.type.empty()) {
				auto search = optimisticLatencies.find(std::make_tuple(k.type, k.numQubits()));
				if(search == optimisticLatencies.end()) {
					optimisticLatencies.emplace(std::make_tuple(k.type, k.numQubits()), v);
				} else {
					if(search->second > v) {
						search->second = v;
					}
				}
			}
		}
	}

public:
	explicit Table(const std::vector<LatencyDescription> & entries) {
		buildTable(entries);
	}
	
	int getLatency(std::string gateName, int numQubits, int target, int control) const override {
		if(numQubits > 0 && target < 0 && control < 0) {
			//We're dealing with a logical gate, so let's return the best case among physical possibilities (so that our a* search will still work okay):
			auto search = optimisticLatencies.find(std::make_tuple(gateName, numQubits));
			if(search != optimisticLatencies.end()) {
				return search->second;
			}
		}
		
		//Try to find perfectly matching latency:
		auto search = latencies.find(GateOp {numQubits, gateName, control, target });
		if(search != latencies.end()) {
			return search->second;
		}
		
		//Try to find matching latency without physical qubits specified
		search = latencies.find(GateOp {numQubits , gateName, -1, -1 });
		if(search != latencies.end()) {
			return search->second;
		}
		
		//Try to find matching latency without physical qubits or gate name specified
		search = latencies.find(GateOp {numQubits, "", -1, -1 });
		if(search != latencies.end()) {
			return search->second;
		}
		
		//Crash
		std::cerr << "FATAL ERROR: could not find any valid latency for specified " << gateName << " gate.\n";
		std::cerr << "\t" << numQubits << "\t" << gateName << "\t" << target << "\t" << control << "\n";
		exit(1);
	}
	
	std::unique_ptr<Latency> clone() const override {
		return std::unique_ptr<Latency>(new Table(*this));
	}
};

}

#endif
