#ifndef LATENCY_HPP
#define LATENCY_HPP

#include <string>

namespace toqm {

class Latency {
public:
	virtual ~Latency() = default;;
	
	virtual int getLatency(std::string gateName, int numQubits, int target, int control) const = 0;
};

}

#endif