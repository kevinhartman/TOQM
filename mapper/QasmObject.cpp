#include "QasmObject.hpp"

#include <cassert>
#include <cstring>
#include <iostream>
#include <vector>

struct QasmObject::Impl {
	//Important variables for outputting an OPENQASM file:
	char * QASM_version{};//string representation of OPENQASM version number (i.e. "2.0")
	std::vector<char *> includes;//list of include statements we need to reproduce in output
	std::vector<char *> customGates;//list of gate definitions we need to reproduce in output
	std::vector<char *> opaqueGates;//list of opaque gate definitions we need to reproduce in output
	std::vector<std::pair<int, int>> measures;//list of measurement gates; first is qbit, second is cbit
	
	//necessary info for mapping original qubit IDs to flat array (and back again, if necessary)
	std::vector<char *> qregName;
	std::vector<int> qregSize;
	
	//necessary info for mapping original cbit IDs to flat array (and back again, if necessary)
	std::vector<char *> cregName;
	std::vector<int> cregSize;
	
	std::vector<toqm::GateOp> gate_ops;
	
	///Gives the flat-array index of the first bit in the specified qreg
	int getQregOffset(char * name) {
		int offset = 0;
		for(unsigned int x = 0; x < qregName.size(); x++) {
			if(!std::strcmp(name, qregName[x])) {
				return offset;
			} else {
				offset += qregSize[x];
			}
		}
		
		std::cerr << "FATAL ERROR: couldn't recognize qreg name " << name << "\n";
		
		assert(false);
		return -1;
	}
	
	///Gives the flat-array index of the first bit in the specified creg
	int getCregOffset(char * name) {
		int offset = 0;
		for(unsigned int x = 0; x < cregName.size(); x++) {
			if(!std::strcmp(name, cregName[x])) {
				return offset;
			} else {
				offset += cregSize[x];
			}
		}
		
		assert(false);
		return -1;
	}
};

QasmObject::QasmObject() : impl(new Impl{}) {};

QasmObject::~QasmObject() = default;

std::size_t QasmObject::numQubits() const {
	std::size_t qubits = 0;
	for(std::size_t x = 0; x < impl->qregName.size(); x++) {
		qubits += impl->qregSize[x];
	}
	return qubits;
}

const std::vector<toqm::GateOp> & QasmObject::gateOperations() const {
	return impl->gate_ops;
}

namespace {

///Gets next token in the OPENQASM file we're parsing
char * getToken(std::istream & infile, bool & sawSemicolon) {
	char c;
	int MAXBUFFERSIZE = 256;
	char buffer[MAXBUFFERSIZE];
	int bufferLoc = 0;
	bool paren = false;//true if inside parentheses, e.g. partway through reading U3(...) gate
	bool bracket = false;//true if inside brackets
	bool quote = false;//true if inside quotation marks
	bool comment = false;//true if between "//" and end-of-line
	
	if(sawSemicolon) {//saw semicolon in previous call, but had to return a different token
		sawSemicolon = false;
		char * token = new char[2];
		token[0] = ';';
		token[1] = 0;
		return token;
	}
	
	while(infile.get(c)) {
		assert(bufferLoc < MAXBUFFERSIZE);
		
		if(comment) {//currently parsing a single-line comment
			if(c == '\n') {
				comment = false;
			}
		} else if(quote) {
			buffer[bufferLoc++] = c;
			if(c == '"') {
				quote = false;
				buffer[bufferLoc++] = 0;
				char * token = new char[bufferLoc];
				strcpy(token, buffer);
				return token;
			}
		} else if(c == '"') {
			assert(!bufferLoc);
			quote = true;
			buffer[bufferLoc++] = c;
		} else if(c == '\r') {
		} else if(c == '/') {//probably parsing the start of a single-line comment
			if(bufferLoc && buffer[bufferLoc - 1] == '/') {
				bufferLoc--;//remove '/' from buffer
				comment = true;
			} else {
				buffer[bufferLoc++] = c;
			}
		} else if(c == ';') {
			assert(!paren);
			assert(!bracket);
			
			if(bufferLoc == 0) {
				buffer[bufferLoc++] = c;
			} else {
				sawSemicolon = true;
			}
			
			buffer[bufferLoc++] = 0;
			char * token = new char[bufferLoc];
			strcpy(token, buffer);
			return token;
		} else if(c == ' ' || c == '\n' || c == '\t' || c == ',') {
			if(paren || bracket) {
				buffer[bufferLoc++] = c;
			} else if(bufferLoc) { //this whitespace is a token separator
				buffer[bufferLoc++] = 0;
				char * token = new char[bufferLoc];
				strcpy(token, buffer);
				return token;
			}
		} else if(c == '(') {
			assert(!paren);
			assert(!bracket);
			paren = true;
			buffer[bufferLoc++] = c;
		} else if(c == ')') {
			assert(paren);
			assert(!bracket);
			paren = false;
			buffer[bufferLoc++] = c;
		} else if(c == '[') {
			assert(!paren);
			assert(!bracket);
			bracket = true;
			buffer[bufferLoc++] = c;
		} else if(c == ']') {
			assert(!paren);
			assert(bracket);
			bracket = false;
			buffer[bufferLoc++] = c;
		} else {
			buffer[bufferLoc++] = c;
		}
	}
	
	if(bufferLoc) {
		buffer[bufferLoc++] = 0;
		char * token = new char[bufferLoc];
		strcpy(token, buffer);
		return token;
	} else {
		return 0;
	}
}

//returns entire gate definition (except the 'gate' keyword) as a string.
char * getCustomGate(std::istream & infile) {
	char c;
	int MAXBUFFERSIZE = 1024;
	char buffer[MAXBUFFERSIZE];
	int bufferLoc = 0;
	bool curlybrace = false;
	bool comment = false;//true if between "//" and end-of-line
	
	while(infile.get(c)) {
		assert(bufferLoc < MAXBUFFERSIZE);
		
		if(comment) {//currently parsing a single-line comment
			if(c == '\n') {
				comment = false;
			}
		} else if(c == '/') {//probably parsing the start of a single-line comment
			if(bufferLoc && buffer[bufferLoc - 1] == '/') {
				bufferLoc--;//remove '/' from buffer
				comment = true;
			}
		} else if(c == '{') {
			assert(!curlybrace);
			curlybrace = true;
			buffer[bufferLoc++] = c;
		} else if(c == '}') {
			assert(curlybrace);
			buffer[bufferLoc++] = c;
			
			buffer[bufferLoc++] = 0;
			char * token = new char[bufferLoc];
			strcpy(token, buffer);
			return token;
		} else {
			buffer[bufferLoc++] = c;
		}
	}
	
	assert(false);
	return 0;
}

//returns the rest of the statement up to and including the semicolon at its end
//I use this for saving opaque gate statements
char * getRestOfStatement(std::istream & infile) {
	char c;
	int MAXBUFFERSIZE = 1024;
	char buffer[MAXBUFFERSIZE];
	int bufferLoc = 0;
	bool comment = false;//true if between "//" and end-of-line
	
	while(infile.get(c)) {
		assert(bufferLoc < MAXBUFFERSIZE);
		
		if(comment) {//currently parsing a single-line comment
			if(c == '\n') {
				comment = false;
			}
		} else if(c == '/') {//probably parsing the start of a single-line comment
			if(bufferLoc && buffer[bufferLoc - 1] == '/') {
				bufferLoc--;//remove '/' from buffer
				comment = true;
			}
		} else if(c == ';') {
			buffer[bufferLoc++] = c;
			
			buffer[bufferLoc++] = 0;
			char * token = new char[bufferLoc];
			strcpy(token, buffer);
			return token;
		} else {
			buffer[bufferLoc++] = c;
		}
	}
	
	assert(false);
	return 0;
}

//Print a node's scheduled gates
//returns how many cycles the node takes to complete all its gates
int printNode(std::ostream & stream, const std::vector<toqm::ScheduledGateOp>& gateStack) {
	int cycles = 0;
	
	for (auto & sg : gateStack) {
		int target = sg.physicalTarget;
		int control = sg.physicalControl;
		stream << sg.gateOp.type << " ";
		if(control >= 0) {
			stream << "q[" << control << "],";
		}
		stream << "q[" << target << "]";
		stream << ";";
		stream << " //cycle: " << sg.cycle;
		if(sg.gateOp.type.compare("swp") && sg.gateOp.type.compare("SWP")) {
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

///Parses the specified OPENQASM file
std::unique_ptr<QasmObject> QasmObject::fromQasm2(std::istream & infile) {
	auto qasmObject = std::unique_ptr<QasmObject>(new QasmObject());
	auto & impl = qasmObject->impl;
	
	std::vector<toqm::GateOp> & gates = impl->gate_ops;
	
	char * token = 0;
	bool b = false;
	while((token = getToken(infile, b))) {//Reminder: the single = instead of double == here is intentional.
		
		if(!strcmp(token, "OPENQASM")) {
			token = getToken(infile, b);
			if(strcmp(token, "2.0")) {
				std::cerr << "WARNING: unexpected OPENQASM version. This may fail.\n";
			}
			
			assert(impl->QASM_version == nullptr);
			impl->QASM_version = token;
			
			token = getToken(infile, b);
			assert(!strcmp(token, ";"));
		} else if(!strcmp(token, "if")) {
			std::cerr << "FATAL ERROR: if-statements not supported.\n";
			exit(1);
		} else if(!strcmp(token, "gate")) {
			token = getCustomGate(infile);
			impl->customGates.push_back(token);
		} else if(!strcmp(token, "opaque")) {
			token = getRestOfStatement(infile);
			impl->opaqueGates.push_back(token);
		} else if(!strcmp(token, "include")) {
			token = getToken(infile, b);
			assert(token[0] == '"');
			impl->includes.push_back(token);
			
			token = getToken(infile, b);
			assert(!strcmp(token, ";"));
		} else if(!strcmp(token, "qreg")) {
			char * bitArray = getToken(infile, b);
			token = getToken(infile, b);
			assert(!strcmp(token, ";"));
			
			char * temp = bitArray;
			int size = 0;
			while(*temp != '[' && *temp != 0) {
				temp++;
			}
			if(*temp == 0) {
				size = 1;
			} else {
				size = std::atoi(temp + 1);
			}
			
			*temp = 0;
			impl->qregName.push_back(bitArray);
			impl->qregSize.push_back(size);
		} else if(!strcmp(token, "creg")) {
			char * bitArray = getToken(infile, b);
			token = getToken(infile, b);
			assert(!strcmp(token, ";"));
			
			char * temp = bitArray;
			int size = 0;
			while(*temp != '[' && *temp != 0) {
				temp++;
			}
			if(*temp == 0) {
				size = 1;
			} else {
				size = std::atoi(temp + 1);
			}
			
			*temp = 0;
			impl->cregName.push_back(bitArray);
			impl->cregSize.push_back(size);
		} else if(!strcmp(token, "measure")) {
			char * qbit = getToken(infile, b);
			
			token = getToken(infile, b);
			assert(!strcmp(token, "->"));
			
			char * cbit = getToken(infile, b);
			
			token = getToken(infile, b);
			assert(!strcmp(token, ";"));
			
			char * temp = qbit;
			int qdx = 0;
			while(*temp != '[' && *temp != 0) {
				temp++;
			}
			if(*temp == 0) {
				qdx = 0;
			} else {
				qdx = std::atoi(temp + 1);
			}
			*temp = 0;
			
			temp = cbit;
			int cdx = 0;
			while(*temp != '[' && *temp != 0) {
				temp++;
			}
			if(*temp == 0) {
				cdx = 0;
			} else {
				cdx = std::atoi(temp + 1);
			}
			*temp = 0;
			
			impl->measures.emplace_back(qdx + impl->getQregOffset(qbit), cdx + impl->getCregOffset(cbit));
		} else if(!strcmp(token, ";")) {
			std::cerr << "Warning: unexpected semicolon.\n";
		} else {
			char * gateName = token;
			char * qubit1Token = getToken(infile, b);
			if(strcmp(qubit1Token, ";")) {
				assert(qubit1Token && qubit1Token[0] != 0);
				int temp = 1;
				while(qubit1Token[temp] != '[' && qubit1Token[temp] != 0) {
					temp++;
				}
				
				//Get flat array index corresponding to this qubit
				int originalOffset = 0;
				if(qubit1Token[temp] != 0) {
					originalOffset = atoi(qubit1Token + temp + 1);
				}
				qubit1Token[temp] = 0;
				int qubit1FlatOffset = impl->getQregOffset(qubit1Token) + originalOffset;
				
				char * qubit2Token = getToken(infile, b);
				if(strcmp(qubit2Token, ";")) {
					assert(qubit2Token && qubit2Token[0] != 0);
					int temp = 1;
					while(qubit2Token[temp] != '[' && qubit2Token[temp] != 0) {
						temp++;
					}
					
					//Get flat array index corresponding to this qubit
					int originalOffset = 0;
					if(qubit2Token[temp] != 0) {
						originalOffset = atoi(qubit2Token + temp + 1);
					}
					qubit2Token[temp] = 0;
					int qubit2FlatOffset = impl->getQregOffset(qubit2Token) + originalOffset;
					
					//We do not accept gates with three (or more) qubits:
					token = getToken(infile, b);
					assert(!strcmp(token, ";"));
					
					//Push this 2-qubit gate onto gate list
					gates.push_back({gateName, qubit2FlatOffset, qubit1FlatOffset});
					
				} else {
					//Push this 1-qubit gate onto gate list
					gates.push_back({gateName, qubit1FlatOffset, -1});
				}
			} else {
				//Push this 0-qubit gate onto gate list
				gates.push_back({gateName, -1, -1});
			}
		}
	}
	
	return qasmObject;
}

void QasmObject::toQasm2(std::ostream & out, const toqm::ToqmResult & result) const {
	//Print out the initial mapping:
	std::cout << "//Note: initial mapping (logical qubit at each location): ";
	for(int x = 0; x < result.numPhysicalQubits; x++) {
		std::cout << (int) result.inferredQal[x] << ", ";
	}
	std::cout << "\n";
	std::cout << "//Note: initial mapping (location of each logical qubit): ";
	for(int x = 0; x < result.numLogicalQubits; x++) {
		std::cout << (int) result.inferredLaq[x] << ", ";
	}
	std::cout << "\n";
	
	//Print the OPENQASM output:
	out << "OPENQASM " << impl->QASM_version << ";\n";
	for(auto & include: impl->includes) {
		out << "include " << include << ";\n";
	}
	for(auto & customGate: impl->customGates) {
		out << "gate " << customGate << "\n";
	}
	for(auto & opaqueGate: impl->opaqueGates) {
		out << "opaque " << opaqueGate << "\n";
	}
	out << "qreg q[" << result.numPhysicalQubits << "];\n";
	out << "creg c[" << result.numPhysicalQubits << "];\n";
	int numCycles = printNode(out, result.scheduledGates);
	for(auto & measure: impl->measures) {
		out << "measure q[" << (int) result.laq[measure.first] << "] -> c["
			<< measure.second << "];\n";
	}
	
	//if(verbose) {
		//Print some metadata about the input & output:
		std::cout << "//" << impl->gate_ops.size() << " original gates\n";
		std::cout << "//" << result.scheduledGates.size() << " gates in generated circuit\n";
		std::cout << "//" << result.idealCycles << " ideal depth (cycles)\n";
		std::cout << "//" << numCycles
				  << " depth of generated circuit\n"; //" (and costFunc reports " << finalNode->cost << ")\n";
		std::cout << "//" << (result.numPopped - 1) << " nodes popped from queue for processing.\n";
		std::cout << "//" << result.remainingInQueue << " nodes remain in queue.\n";
		std::cout << result.filterStats;
	//}

}
