#ifndef LATENCY_TABLE_HPP
#define LATENCY_TABLE_HPP

#include "libtoqm/Latency.hpp"

#include <cstring>
#include <fstream>
#include <iostream>
#include <cassert>
#include <unordered_map>
#include <utility>
#include <tuple>

using namespace std;

namespace toqm {

/**
 * A Latency class which uses a latency table.
 * It takes one argument: the path for the latency table text file.
 * The latency table consists of a one or more tuples.
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
	2	swp	-	-	6
	2	-	-	-	2
	1	-	-	-	1
 * In this example, the cx gate has a default latency of 2 cycles,
	but for certain physical qubits cx instead takes 3 or 4 cycles.
	The cy gate always takes 12 cycles, and the swp gate always takes 6 cycles.
	All other 2-qubit gates take 2 cycles, and all 1-qubit gates take 1 cycle.
 * If your latency table has an entry with physical qubits for some gate G, 
	then it should also have a default entry for G (i.e. an entry without physical qubits).
	Otherwise our current heuristic functions may exhibit strange behavior.
 */
class Table : public Latency {
private:
	
	typedef std::tuple<char *, int, int, int> key_t;
	
	struct key_hash : public std::unary_function<key_t, std::size_t> {
		std::size_t operator()(const key_t &k) const {
			return std::get<0>(k)[0] ^ std::get<1>(k) ^ std::get<2>(k) ^ std::get<3>(k);
		}
	};
	
	struct key_equal : public std::binary_function<key_t, key_t, bool> {
		bool operator()(const key_t &v0, const key_t &v1) const {
			return (!strcmp(std::get<0>(v0), std::get<0>(v1)) &&
					std::get<1>(v0) == std::get<1>(v1) &&
					std::get<2>(v0) == std::get<2>(v1) &&
					std::get<3>(v0) == std::get<3>(v1));
		}
	};
	
	typedef std::tuple<char *, int> key_t2;
	
	struct key_hash2 : public std::unary_function<key_t2, std::size_t> {
		std::size_t operator()(const key_t2 &k) const {
			return std::get<0>(k)[0] ^ std::get<1>(k);
		}
	};
	
	struct key_equal2 : public std::binary_function<key_t2, key_t2, bool> {
		bool operator()(const key_t2 &v0, const key_t2 &v1) const {
			return (!strcmp(std::get<0>(v0), std::get<0>(v1)) &&
					std::get<1>(v0) == std::get<1>(v1));
		}
	};
	
	//map using gate's name, # bits, physical target, and physical control as key(s).
	std::unordered_map<key_t, int, key_hash, key_equal> latencies;
	
	//best-case latency map when we haven't yet decided on physical qubits
	std::unordered_map<key_t2, int, key_hash2, key_equal2> optimisticLatencies;
	
	//Tokenizer for parsing the latency table file:
	char *getToken(std::istream &infile) {
		char c;
		int MAXBUFFERSIZE = 256;
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
				if(bufferLoc && buffer[bufferLoc-1] == '/') {
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
					char *token = new char[bufferLoc];
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
			char *token = new char[bufferLoc];
			strcpy(token, buffer);
			return token;
		} else {
			return 0;
		}
	}
	
	//Parse the latency table file:
	void parseTable(std::istream &infile) {
		char *token;
		while((token = getToken(infile))) {//Reminder: the single = instead of double == here is intentional.
			int numBits = atoi(token);
			char *gateName = getToken(infile);
			char *target = getToken(infile);
			char *control = getToken(infile);
			char *latency = getToken(infile);;
			
			//Don't allow entries where physical qubits are only partially specified:
			assert(numBits < 2 || (strcmp(target, "-") == strcmp(control, "-")));
			
			int targetVal = -1;
			if(strcmp(target, "-")) {
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
			
			//Don't allow duplicate entries
			auto search = latencies.find(make_tuple(gateName, numBits, targetVal, controlVal));
			assert(search == latencies.end());
			
			latencies.emplace(make_tuple(gateName, numBits, targetVal, controlVal), latencyVal);
			
			//record best-case latency for this gate regardless of physical qubits
			if(strcmp(gateName, "-")) {
				auto search = optimisticLatencies.find(make_tuple((char *) gateName, numBits));
				if(search == optimisticLatencies.end()) {
					optimisticLatencies.emplace(make_tuple((char *) gateName, numBits), latencyVal);
				} else {
					if(search->second > latencyVal) {
						search->second = latencyVal;
					}
				}
			}
		}
	}

public:
	explicit Table(std::istream &source) {
		parseTable(source);
	}
	
	int getLatency(string gateName, int numQubits, int target, int control) const override {
		if(numQubits > 0 && target < 0 && control < 0) {
			//We're dealing with a logical gate, so let's return the best case among physical possibilities (so that our a* search will still work okay):
			auto search = optimisticLatencies.find(make_tuple((char *) gateName.c_str(), numQubits));
			if(search != optimisticLatencies.end()) {
				return search->second;
			}
		}
		
		//Try to find perfectly matching latency:
		auto search = latencies.find(make_tuple((char *) gateName.c_str(), numQubits, target, control));
		if(search != latencies.end()) {
			return search->second;
		}
		
		//Try to find matching latency without physical qubits specified
		search = latencies.find(make_tuple((char *) gateName.c_str(), numQubits, -1, -1));
		if(search != latencies.end()) {
			return search->second;
		}
		
		//Try to find matching latency without physical qubits or gate name specified
		search = latencies.find(make_tuple((char *) "-", numQubits, -1, -1));
		if(search != latencies.end()) {
			return search->second;
		}
		
		//Crash
		std::cerr << "FATAL ERROR: could not find any valid latency for specified " << gateName << " gate.\n";
		std::cerr << "\t" << numQubits << "\t" << gateName << "\t" << target << "\t" << control << "\n";
		exit(1);
	}
};

}

#endif
