#ifndef LATENCY_1_HPP
#define LATENCY_1_HPP

#include "libtoqm/Latency.hpp"

namespace toqm {

//Latency example: 1 cycle for EVERY gate
class Latency_1 : public Latency {
public:
	int getLatency(std::string gateName, int numQubits, int target, int control) const override {
		return 1;
	}
	
	std::unique_ptr<Latency> clone() const override {
		return std::unique_ptr<Latency>(new Latency_1(*this));
	}
};

}

#endif
