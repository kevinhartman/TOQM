#include "CouplingMap.hpp"

#include <istream>

toqm::CouplingMap parseCouplingMap(std::istream & in) {
	auto map = toqm::CouplingMap{};
	
	unsigned int numEdges;
	
	in >> map.numPhysicalQubits;
	in >> numEdges;
	for(unsigned int x = 0; x < numEdges; x++) {
		int a, b;
		in >> a;
		in >> b;
		std::pair<int, int> edge = std::make_pair(a, b);
		map.edges.insert(edge);
	}
	
	return map;
}
