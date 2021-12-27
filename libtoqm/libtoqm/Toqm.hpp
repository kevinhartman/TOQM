#ifndef TOQM_TOQM_HPP
#define TOQM_TOQM_HPP

#include <string>
#include <vector>

namespace toqm {

class Expander;
class CostFunc;
class Latency;
class Queue;
class NodeMod;
class Filter;

extern bool _verbose;

void run(const std::string& qasmFileName,
         const std::string& couplingMapFileName,
         Expander *ex,
         CostFunc *cf,
         Latency *lat,
         Queue *nodes,
         const std::vector<NodeMod*>& node_mods,
         const std::vector<Filter*>& filters,
         unsigned int retainPopped,
         int initialSearchCycles,
         int use_specified_init_mapping,
         const char *init_qal,
         const char *init_laq);
}

#endif //TOQM_TOQM_HPP
