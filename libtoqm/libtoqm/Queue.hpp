#ifndef QUEUE_HPP
#define QUEUE_HPP

#include "Environment.hpp"
#include "Filter.hpp"
#include "Node.hpp"

#include <iostream>
#include <memory>

namespace toqm {

class Queue {
private:
	///Push a node into the priority queue
	///Return false iff this fails for any reason
	///Pre-condition: our filters have already said this node is good
	///Pre-condition: newNode->cost has already been set
	virtual bool pushNode(const std::shared_ptr<Node>& newNode) = 0;

protected:
	std::shared_ptr<Node> bestFinalNode = nullptr;
	int numPushed = 0, numFiltered = 0, numPopped = 0;

public:
	virtual ~Queue() = default;
	
	///Pop a node and return it
	virtual std::shared_ptr<Node> pop() = 0;
	
	///Return number of elements in queue
	virtual int size() = 0;
	
	///Push a node into the priority queue
	///Return false iff this fails for any reason
	///Pre-condition: newNode->cost has already been set
	bool push(const std::shared_ptr<Node>& newNode) {
		numPushed++;
		if(!newNode->env.filter(newNode)) {
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
	
	inline const std::shared_ptr<Node>& getBestFinalNode() const {
		return bestFinalNode;
	}
	
	virtual std::unique_ptr<Queue> clone() const = 0;
};

}

#endif
