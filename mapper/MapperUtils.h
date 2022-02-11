#ifndef QISKIT_TOQM_MAPPERUTILS_H
#define QISKIT_TOQM_MAPPERUTILS_H

#include <libtoqm/CommonTypes.hpp>

#include <vector>

class MapperUtils {
public:
	static std::vector<toqm::LatencyDescription> parseLatencyTable(std::istream & in);
};


#endif //QISKIT_TOQM_MAPPERUTILS_H
