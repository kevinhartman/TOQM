#ifndef DEFAULT_QUEUE_HPP
#define DEFAULT_QUEUE_HPP

#include "libtoqm/Queue.hpp"

#include <queue>
#include <vector>
#include <iostream>

namespace toqm {

extern bool _verbose;

//This queue uses std::priority_queue
class DefaultQueue : public Queue {
private:
	struct CmpDefaultQueue {
		bool operator()(const std::shared_ptr<Node>& lhs, const std::shared_ptr<Node>& rhs) const {
			//tiebreaker:
			if(lhs->cost == rhs->cost) {
				//return lhs->scheduled->size > rhs->scheduled->size;
				//return lhs->numUnscheduledGates > rhs->numUnscheduledGates;
				//return lhs->cycle < rhs->cycle;
			}
			
			//lower cost is better
			return lhs->cost > rhs->cost;
		}
	};
	
	std::priority_queue<std::shared_ptr<Node>, std::vector<std::shared_ptr<Node>>, CmpDefaultQueue> nodes;
	
	bool pushNode(const std::shared_ptr<Node>& newNode) override {
		nodes.push(newNode);
		if(_verbose) {
			if(newNode->numUnscheduledGates < garbage) {
				garbage = newNode->numUnscheduledGates;
				garbage2 = newNode->cost;
				
				std::cerr << "dbg More progress!\n";
				std::cerr << " " << garbage << " gates remain!\n";
				std::cerr << " cost is " << newNode->cost << "\n";
				if(newNode->parent)
					std::cerr << " parent cost is " << newNode->parent->cost << "\n";
				else
					std::cerr << " root node!\n";
				std::cerr << " num ready gates is " << newNode->readyGates.size() << "\n";
			} else if(newNode->numUnscheduledGates == garbage) {
				if(newNode->cost < garbage2) {
					garbage2 = newNode->cost;
					std::cerr << "dbg Better progress!\n";
					std::cerr << " new cost is " << newNode->cost << "\n";
				}
			}
		}
		
		return true;
	}
	
	int garbage = 9999999;
	int garbage2 = 9999999;

public:
	std::shared_ptr<Node> pop() override {
		numPopped++;
		
		auto ret = nodes.top();
		nodes.pop();
		
		if(!ret->readyGates.size()) {
			assert(ret->numUnscheduledGates == 0);
			bool done = true;
			if(done) {
				if(!bestFinalNode) {
					if(_verbose) std::cerr << "dbg msg: found a final node.\n";
					bestFinalNode = ret;
				} else if(ret->cost < bestFinalNode->cost) {
					if(_verbose) std::cerr << "dbg msg: found a better final node.\n";
					//delete bestFinalNode;
					bestFinalNode = ret;
				}
			}
		}
		
		return ret;
	}
	
	int size() override {
		return nodes.size();
	}
	
	std::unique_ptr<Queue> clone() const override {
		return std::unique_ptr<Queue>(new DefaultQueue(*this));
	}
};

}

#endif