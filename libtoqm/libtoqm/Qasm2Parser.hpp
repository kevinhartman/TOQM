#ifndef ARI_PARSER
#define ARI_PARSER

#include "CommonTypes.h"

#include <vector>

using namespace std;

namespace toqm {

class Environment;

/**
 * Parses a quantum file, sets appropriate parts of the Environment, and returns list of quantum gates.
 * @param env The environment
 * @param fileName File path for openqasm 2.0 file
 * @return vector of ParsedGate
 */
std::vector<GateOp> parseQasm2(Environment *env, const char *fileName);

}

#endif
