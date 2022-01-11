#ifndef TOQM_COUPLINGMAPPARSER_HPP
#define TOQM_COUPLINGMAPPARSER_HPP

#include <libtoqm/CommonTypes.hpp>

#include <iosfwd>

//parse coupling map, producing a list of edges and number of physical qubits
toqm::CouplingMap parseCouplingMap(std::istream & in);

#endif //TOQM_COUPLINGMAPPARSER_HPP
