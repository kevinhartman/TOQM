#ifndef LATENCY_1_HPP
#define LATENCY_1_HPP

#include "libtoqm/Latency.hpp"

namespace toqm {

//Latency example: 1 cycle for EVERY gate
class Latency_1 : public Latency {
public:
	int getLatency(string gateName, int numQubits, int target, int control) const {
		return 1;
	}
};

}

#endif
