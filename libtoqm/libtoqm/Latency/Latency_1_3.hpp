#ifndef LATENCY_1_3_HPP
#define LATENCY_1_3_HPP

#include "libtoqm/Latency.hpp"

namespace toqm {

//Latency example: 3 cycles per SWP; 1 cycle otherwise
class Latency_1_3 : public Latency {
public:
	int getLatency(std::string gateName, int numQubits, int target, int control) const override {
		if(!gateName.compare("swap") || !gateName.compare("SWAP")) {
			return 3;
		} else {
			return 1;
		}
	}
	
	std::unique_ptr<Latency> clone() const override {
		return std::unique_ptr<Latency>(new Latency_1_3(*this));
	}
};

}

#endif