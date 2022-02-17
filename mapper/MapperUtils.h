#ifndef QISKIT_TOQM_MAPPERUTILS_H
#define QISKIT_TOQM_MAPPERUTILS_H

#include "QasmObject.hpp"

#include <libtoqm/CommonTypes.hpp>
#include <vector>

class MapperUtils {
public:
	/**
	 * Parse a latency table into a list of latency descriptions.
	 *
	 * Input format:
	 * A latency table consists of a one or more tuples, separated
	 * by newlines.
	 * Each tuple consists of five elements:
		the number of qubits,
		the gate name,
		the physical target qubit,
		the physical control qubit,
		and the latency.
	 * Where appropriate, these elements may be replaced by dashes.
	 * For example, consider this latency table:
		2	cx	1	0	3
		2	cx	0	1	3
		2	cx	0	2	3
		2	cx	2	3	4
		2	cx	-	-	2
		2	cy	-	-	12
		2	swap-	-	6
		2	-	-	-	2
		1	-	-	-	1
	 * In this example, the cx gate has a default latency of 2 cycles,
		but for certain physical qubits cx instead takes 3 or 4 cycles.
		The cy gate always takes 12 cycles, and the swp gate always takes 6 cycles.
		All other 2-qubit gates take 2 cycles, and all 1-qubit gates take 1 cycle.
	 * If your latency table has an entry with physical qubits for some gate G,
		then it should also have a default entry for G (i.e. an entry without physical qubits).
		Otherwise our current heuristic functions may exhibit strange behavior.
	 * @param in the latency table text stream.
	 * @return the list of latency descriptions.
	 */
	static std::vector<toqm::LatencyDescription> parseLatencyTable(std::istream & in);
	static toqm::CouplingMap parseCouplingMap(std::istream & in);
	static constexpr auto parseQasm2 = QasmObject::fromQasm2;
};


#endif //QISKIT_TOQM_MAPPERUTILS_H
