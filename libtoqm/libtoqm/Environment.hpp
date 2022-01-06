#ifndef ENVIRONMENT_HPP
#define ENVIRONMENT_HPP

#include "libtoqm/Latency.hpp"
#include "libtoqm/Filter.hpp"
#include "libtoqm/NodeMod.hpp"
#include <set>
#include <vector>
#include <cassert>
#include <cstring>
#include <iosfwd>

using namespace std;

namespace toqm {

class GateNode;

class CostFunc;

class Environment {//for data shared across all nodes
public:
	Environment(const CostFunc & cost, const Latency & latency, const vector<std::unique_ptr<NodeMod>> & node_mods)
			: cost(cost), latency(latency), nodeMods(node_mods) {}
	
	const vector<std::unique_ptr<NodeMod>> & nodeMods;
	const CostFunc & cost;//contains function to calculate a node's cost
	const Latency & latency;//contains function to calculate a gate's latency
	
	//each env gets its own copy
	vector<unique_ptr<Filter>> filters;
	
	set<pair<int, int> > couplings; //the coupling map (as a list of qubit-pairs)
	GateNode ** possibleSwaps{}; //list of swaps implied by the coupling map
	int * couplingDistances{};//array of size (numPhysicalQubits*numPhysicalQubits), containing the minimal number of hops between each pair of qubits in the coupling graph
	
	int numLogicalQubits{};//number of logical qubits in circuit; if there's a gap then this includes unused qubits
	int numPhysicalQubits{};//number of physical qubits in the coupling map
	int swapCost{}; //best possible swap cost; this should be set by main using the latency function
	int numGates{}; //the number of gates in the original circuit
	
	GateNode ** firstCXPerQubit{};//the first 2-qubit gate that uses each logical qubit
	
	///Invoke all node mods, using the specified node and specified flag
	void runNodeModifiers(Node& node, int flag) {
		for(const auto & nodeMod : this->nodeMods) {
			nodeMod->mod(node, flag);
		}
	}
	
	///Invoke the active filters; returns true if we should delete the node.
	bool filter(const std::shared_ptr<Node> & newNode) {
		for(unsigned int x = 0; x < this->filters.size(); x++) {
			if(this->filters[x]->filter(newNode)) {
				for(unsigned int y = 0; y < x; y++) {
					this->filters[y]->deleteRecord(*newNode);
				}
				return true;
			}
		}
		
		return false;
	}
	
	///Instructs all active filters to delete all pointers to the specified node.
	void deleteRecord(const Node & oldNode) {
		for(auto & filter : this->filters) {
			filter->deleteRecord(oldNode);
		}
	}
	
	///Recreates the filters, forcibly erasing any data they've gathered.
	void resetFilters() {
		for(auto & filter : this->filters) {
			auto & old = filter;
			filter = old->createEmptyCopy();
		}
	}
	
	///Invokes the printStatistics function for every active filter.
	void printFilterStats(std::ostream & stream) {
		for(auto & filter : this->filters) {
			filter->printStatistics(stream);
		}
	}
};

}

#endif
