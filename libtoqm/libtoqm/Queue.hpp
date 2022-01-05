#ifndef QUEUE_HPP
#define QUEUE_HPP

#include "Node.hpp"
#include "Environment.hpp"
#include "Filter.hpp"
#include <iostream>

namespace toqm {

class Queue {
private:
	///Push a node into the priority queue
	///Return false iff this fails for any reason
	///Pre-condition: our filters have already said this node is good
	///Pre-condition: newNode->cost has already been set
	virtual bool pushNode(Node *newNode) = 0;

protected:
	Node *bestFinalNode = nullptr;
	int numPushed = 0, numFiltered = 0, numPopped = 0;

public:
	virtual ~Queue() {};

	///Pop a node and return it
	virtual Node *pop() = 0;

	///Return number of elements in queue
	virtual int size() = 0;

	///Push a node into the priority queue
	///Return false iff this fails for any reason
	///Pre-condition: newNode->cost has already been set
	bool push(Node *newNode) {
		numPushed++;
		if(!newNode->env->filter(newNode)) {
			bool success = this->pushNode(newNode);
			if(success) {
				return true;
			} else {
				std::cerr << "WARNING: pushNode(Node*) failed somehow.\n";
				return false;
			}
		}
		numFiltered++;
		return false;
	}

	inline Node *getBestFinalNode() {
		return bestFinalNode;
	}
};

}

#endif
