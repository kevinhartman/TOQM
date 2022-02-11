#ifndef QISKIT_TOQM_MAPPERUTILS_H
#define QISKIT_TOQM_MAPPERUTILS_H

#include "QasmObject.hpp"

#include <libtoqm/CommonTypes.hpp>
#include <vector>

class MapperUtils {
public:
	static std::vector<toqm::LatencyDescription> parseLatencyTable(std::istream & in);
	static toqm::CouplingMap parseCouplingMap(std::istream & in);
	static constexpr auto parseQasm2 = QasmObject::fromQasm2;
};


#endif //QISKIT_TOQM_MAPPERUTILS_H
