#include "PrinterHelpers.hpp"

#include <libtoqm/libtoqm.hpp>

namespace toqm {

//Print a node's scheduled gates
//returns how many cycles the node takes to complete all its gates
std::ostream & operator<<(std::ostream & os, const std::vector<ScheduledGateOp> & gates) {
	for(auto & sg : gates) {
		int target = sg.physicalTarget;
		int control = sg.physicalControl;
		os << sg.gateOp.type << " ";
		if(control >= 0) {
			os << "q[" << control << "],";
		}
		os << "q[" << target << "]";
		os << ";";
		os << " //cycle: " << sg.cycle;
		if(sg.gateOp.type.compare("swap") && sg.gateOp.type.compare("SWAP")) {
			int target = sg.gateOp.target;
			int control = sg.gateOp.control;
			os << " //" << sg.gateOp.type << " ";
			if(control >= 0) {
				os << "q[" << control << "],";
			}
			os << "q[" << target << "]";
		}
		os << "\n";
	}

	return os;
}
}