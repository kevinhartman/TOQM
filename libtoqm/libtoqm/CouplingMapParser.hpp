#ifndef TOQM_COUPLINGMAPPARSER_H
#define TOQM_COUPLINGMAPPARSER_H

#include "CommonTypes.hpp"

#include <istream>

namespace toqm {

//parse coupling map, producing a list of edges and number of physical qubits
CouplingMap parseCouplingMap(std::istream& in);

}

#endif //TOQM_COUPLINGMAPPARSER_H
