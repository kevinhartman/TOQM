#ifndef EXPAND_HPP
#define EXPAND_HPP

#include "Latency.hpp"

#include <memory>

namespace toqm {

class Node;

class Queue;

class Expander {
public:
	virtual ~Expander() = default;
	
	//expands given node, unless it has same-or-worse cost than best final node
	//returns false iff given node's cost >= best final node's cost
	virtual bool expand(Queue& nodes, Node& node) const = 0;
};

}

#endif
