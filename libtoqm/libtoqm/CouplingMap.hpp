#ifndef TOQM_COUPLINGMAPPARSER_HPP
#define TOQM_COUPLINGMAPPARSER_HPP

#include "CommonTypes.hpp"

#include <iosfwd>

namespace toqm {

//parse coupling map, producing a list of edges and number of physical qubits
CouplingMap parseCouplingMap(std::istream & in);

}

#endif //TOQM_COUPLINGMAPPARSER_HPP
