#ifndef FILTER_HPP
#define FILTER_HPP

#include <iostream>
#include <memory>

namespace toqm {

class Node;

class Queue;

class Filter {
public:
	virtual ~Filter() = default;;
	
	//this should be called after we're done scheduling gates in newNode
	//return true iff we don't want to add newNode to the nodes list
	virtual bool filter(Node * newNode) = 0;
	
	virtual void printStatistics(std::ostream & stream) {
		//this function should print info such as how many nodes have been filtered out
	}
	
	virtual void deleteRecord(Node * n) {
		//if this filter retains node info, delete the filter's records of node n
	}
	
	virtual std::unique_ptr<Filter> createEmptyCopy() = 0;
};

}

#endif