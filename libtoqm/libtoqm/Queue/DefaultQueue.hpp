#ifndef DEFAULT_QUEUE_HPP
#define DEFAULT_QUEUE_HPP

#include "libtoqm/Queue.hpp"

#include <queue>
#include <vector>
#include <iostream>
#include <tuple>

namespace toqm {

extern bool _verbose;

//This queue uses std::priority_queue
class DefaultQueue : public Queue {
private:
	using PriotityQueueType = std::tuple<std::size_t, std::shared_ptr<Node>>;

	struct CmpDefaultQueue {
		bool operator()(const PriotityQueueType & lhs, const PriotityQueueType & rhs) const {
			auto lhs_node = std::get<1>(lhs);
			auto rhs_node = std::get<1>(rhs);

			//tiebreaker:
			if(lhs_node->cost == rhs_node->cost) {
				// Favor pushed most recently.
				return std::get<0>(lhs) < std::get<0>(rhs);
				//return lhs->scheduled->size > rhs->scheduled->size;
				//return lhs->numUnscheduledGates > rhs->numUnscheduledGates;
				//return lhs->cycle < rhs->cycle;
			}
			
			//lower cost is better
			return lhs_node->cost > rhs_node->cost;
		}
	};
	
	std::priority_queue<PriotityQueueType, std::vector<PriotityQueueType>, CmpDefaultQueue> nodes {};
	
	bool pushNode(const std::shared_ptr<Node>& newNode) override {
		nodes.push(std::make_tuple(numPushed, newNode));
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
		
		auto ret = std::get<1>(nodes.top());
		nodes.pop();
		
		if(ret->readyGates.empty()) {
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
	
	size_t size() override {
		return nodes.size();
	}
	
	std::unique_ptr<Queue> clone() const override {
		return std::unique_ptr<Queue>(new DefaultQueue(*this));
	}
};

}

#endif