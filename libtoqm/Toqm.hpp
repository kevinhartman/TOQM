#ifndef TOQM_TOQM_HPP
#define TOQM_TOQM_HPP

#include <string>

namespace toqm {

class Expander;
class CostFunc;
class Latency;
class Queue;
class Environment;

extern bool _verbose;

void run(std::string qasmFileName,
         std::string couplingMapFileName,
         Expander *ex,
         CostFunc *cf,
         Latency *lat,
         Queue *nodes,
         Environment *env,
         int retainPopped,
         int initialSearchCycles,
         int use_specified_init_mapping,
         char *init_qal,
         char *init_laq);
}

#endif //TOQM_TOQM_HPP
