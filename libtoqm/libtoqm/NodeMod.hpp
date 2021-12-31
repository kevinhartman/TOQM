#ifndef NODEMOD_HPP
#define NODEMOD_HPP

namespace toqm {

class Node;

#define MOD_TYPE_BEFORECOST 1

class NodeMod {
public:
    virtual ~NodeMod() {};

    virtual void mod(Node *node, int flag) const = 0;
};

}

#endif
