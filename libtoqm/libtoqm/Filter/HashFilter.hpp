#ifndef HASHFILTER_HPP
#define HASHFILTER_HPP

#include "libtoqm/Filter.hpp"
#include "libtoqm/Node.hpp"

#include <iostream>
#include <functional>
#include <unordered_map>
#include <vector>

#ifndef HASH_COMBINE_FUNCTION
#define HASH_COMBINE_FUNCTION

//based on hash_combine from Boost libraries
template<class T>
inline void hash_combine(std::size_t & seed, const T & v) {
	std::hash<T> hasher;
	seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

#endif

namespace toqm {

inline std::size_t hashFunc1(const Node& n) {
	std::size_t hash_result = 0;
	int numQubits = n.env.numPhysicalQubits;
	
	//combine into hash: qubit map (array of integers)
	for(int x = 0; x < numQubits; x++) {
		hash_combine(hash_result, n.laq[x]);
	}
	
	//combine into hash: ready gate (set of pointers)
	for(auto x = n.readyGates.begin(); x != n.readyGates.end(); x++) {
		hash_combine(hash_result, (std::uintptr_t) (*x));
	}
	
	return hash_result;
}

class HashFilter : public Filter {
private:
	int numFiltered = 0;
	std::unordered_map<std::size_t, vector<std::shared_ptr<Node>>> hashmap;

public:
	std::unique_ptr<Filter> createEmptyCopy() override {
		auto f = std::unique_ptr<HashFilter>(new HashFilter());
		f->numFiltered = this->numFiltered;
		return f;
	}
	
	void deleteRecord(const Node& n) override {
		std::size_t hash_result = hashFunc1(n);
		auto & mapValue = this->hashmap[hash_result];
		for(unsigned int blah = 0; blah < mapValue.size(); blah++) {
			auto n2 = mapValue[blah];
			if(n2.get() == &n) {
				if(mapValue.size() > 1 && blah < mapValue.size() - 1) {
					std::swap(mapValue[blah], mapValue[mapValue.size() - 1]);
				}
				mapValue.pop_back();
				return;
			}
		}
	}
	
	bool filter(const std::shared_ptr<Node>& newNode) override {
		int numQubits = newNode->env.numPhysicalQubits;
		std::size_t hash_result = hashFunc1(*newNode);
		
		//auto findNode = this->hashmap.find(hash_result);
		//if(findNode != this->hashmap.end()) {
		for(const auto& candidate: this->hashmap[hash_result]) {
			//Node * candidate = findNode->second;
			bool willFilter = true;
			
			for(int x = 0; x < numQubits; x++) {
				if(candidate->laq[x] != newNode->laq[x]) {
					//std::cerr << "Warning: duplicate hash values.\n";
					willFilter = false;
					break;
				}
			}
			if(newNode->readyGates.size() != candidate->readyGates.size()) {
				//std::cerr << "Warning: duplicate hash values.\n";
				willFilter = false;
			} else if(willFilter) {
				for(auto x = newNode->readyGates.begin(), y = candidate->readyGates.begin();
					x != newNode->readyGates.end(); x++, y++) {
					if((*x) != (*y)) {
						//std::cerr << "Warning: duplicate hash values.\n";
						willFilter = false;
						break;
					}
				}
			}
			bool allEqual = willFilter;
			for(int x = 0; willFilter && x < numQubits; x++) {
				int time = 0;
				int newBusy = newNode->busyCycles(x);
				if(newBusy) {
					int temp = newBusy + newNode->cycle;
					assert(temp > time);
					time = temp;
				}
				int time2 = 0;
				int canBusy = candidate->busyCycles(x);
				if(canBusy) {
					int temp = canBusy + candidate->cycle;
					assert(temp > time2);
					time2 = temp;
				}
				if(time < time2) {
					//std::cerr << "Warning: duplicate hash values.\n";
					willFilter = false;
					allEqual = false;
					break;
				} else if(time > time2) {
					allEqual = false;
				}
			}
			
			if(willFilter && (newNode->cycle == candidate->cycle || !allEqual)) {
				numFiltered++;
				return true;
			}
		}
		this->hashmap[hash_result].push_back(newNode);
		
		return false;
	}
	
	void printStatistics(std::ostream & stream) override {
		stream << "//HashFilter filtered " << numFiltered << " total nodes.\n";
	}
};

}

#endif