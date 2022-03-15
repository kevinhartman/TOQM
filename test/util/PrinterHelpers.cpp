#include "PrinterHelpers.hpp"

#include <libtoqm/libtoqm.hpp>

namespace toqm {
namespace test {

//Print a node's scheduled gates
//returns how many cycles the node takes to complete all its gates
int printNode(std::ostream & stream, const std::vector<toqm::ScheduledGateOp> & gateStack) {
	int cycles = 0;

	for(auto & sg : gateStack) {
		int target = sg.physicalTarget;
		int control = sg.physicalControl;
		stream << sg.gateOp.type << " ";
		if(control >= 0) {
			stream << "q[" << control << "],";
		}
		stream << "q[" << target << "]";
		stream << ";";
		stream << " //cycle: " << sg.cycle;
		if(sg.gateOp.type.compare("swap") && sg.gateOp.type.compare("SWAP")) {
			int target = sg.gateOp.target;
			int control = sg.gateOp.control;
			stream << " //" << sg.gateOp.type << " ";
			if(control >= 0) {
				stream << "q[" << control << "],";
			}
			stream << "q[" << target << "]";
		}
		stream << "\n";

		if(sg.cycle + sg.latency > cycles) {
			cycles = sg.cycle + sg.latency;
		}
	}

	return cycles;
}

}
}