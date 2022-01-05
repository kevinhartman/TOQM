#ifndef COSTFUNC_HPP
#define COSTFUNC_HPP

#include "Node.hpp"
#include "NodeMod.hpp"

namespace toqm {

class CostFunc {
public:
	virtual ~CostFunc() {};
	
	virtual int _getCost(Node * node) const = 0;
	
	///Returns the cost of the node
	///This may invoke node modifiers prior to calculating the cost.
	int getCost(Node * node) const {
		Environment * env = node->env;
		env->runNodeModifiers(node, MOD_TYPE_BEFORECOST);
		return _getCost(node);
	}
};

}

#endif