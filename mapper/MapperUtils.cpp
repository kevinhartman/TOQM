#include "MapperUtils.h"

#include <istream>
#include <cassert>

namespace {

//Tokenizer for parsing the latency table file:
char * latencyGetToken(std::istream & infile) {
	char c;
	const int MAXBUFFERSIZE = 256;
	char buffer[MAXBUFFERSIZE];
	int bufferLoc = 0;
	bool paren = false;//true iff inside parentheses, i.e. partway through reading U3(...) gate name
	bool comment = false;//true iff between "//" and end-of-line
	
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
			} else {
				buffer[bufferLoc++] = c;
			}
		} else if(c == ' ' || c == '\n' || c == '\t' || c == ',' || c == '\r') {
			if(paren) {
				buffer[bufferLoc++] = c;
			} else if(bufferLoc) { //this whitespace is a token separator
				buffer[bufferLoc++] = 0;
				char * token = new char[bufferLoc];
				strcpy(token, buffer);
				return token;
			}
		} else if(c == '(') {
			assert(!paren);
			paren = true;
			buffer[bufferLoc++] = c;
		} else if(c == ')') {
			assert(paren);
			paren = false;
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

}

std::vector<toqm::LatencyDescription> MapperUtils::parseLatencyTable(std::istream & in) {
	std::vector<toqm::LatencyDescription> result {};
	
	char * token;
	while((token = latencyGetToken(in))) {//Reminder: the single = instead of double == here is intentional.
		int numBits = atoi(token);
		std::string gateName = latencyGetToken(in);
		char * target = latencyGetToken(in);
		char * control = latencyGetToken(in);
		char * latency = latencyGetToken(in);
		
		if(gateName == "-") {
			gateName = "";
		}
		
		int targetVal = -1;
		if(strcmp(target, "-") != 0) {
			targetVal = atoi(target);
		}
		
		int controlVal = -1;
		if(strcmp(control, "-")) {
			controlVal = atoi(control);
		}
		
		int latencyVal = -1;
		if(strcmp(latency, "-")) {
			latencyVal = atoi(latency);
		}
		
		result.emplace_back(numBits, gateName, controlVal, targetVal, latencyVal);
	}
	
	return result;
}

toqm::CouplingMap MapperUtils::parseCouplingMap(std::istream & in) {
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