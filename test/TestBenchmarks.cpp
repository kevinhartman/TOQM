#include <catch2/catch.hpp>

#include "util/ComparisonHelpers.hpp"
#include "util/MapperBuilder.hpp"
#include "util/PrinterHelpers.hpp"
#include "../mapper/MapperUtils.h"

#include <libtoqm/libtoqm.hpp>

#include <fstream>
#include <tuple>
#include <sstream>

const auto IN_POSTFIX = ".qasm";
const auto OUT_POSTFIX = "_expected.qasm";

TEST_CASE("Small circuits, IBM QX2, 1Q: 1 cycles, 2Q: 2 cycles, SWAP: 6 cycles", "[benchmarks]") {
	std::string in_dir = "data/circuits/small/";
	std::string expected_out_dir = "data/expected_output/small_qx2_benchmarks/";
	std::string coupling_file = "data/couplings/qx2.txt";
	std::string circuit_name = GENERATE(
			"3_17_13",
			"4gt11_82",
			"4gt11_84",
			"4gt13_92",
			"4mod5-v0_19",
			"4mod5-v0_20",
			"4mod5-v1_22",
			"4mod5-v1_24",
			"alu-v0_27",
			"alu-v1_28",
			"alu-v1_29",
			"alu-v2_33",
			"alu-v3_34",
			"alu-v3_35",
			"alu-v4_37",
			"ex-1_166",
			"ham3_102",
			"miller_11",
			"mod5d1_63",
			"mod5mils_65",
			"qft_4",
			"rd32-v0_66",
			"rd32-v1_68");

	auto coupling_istream = std::ifstream(coupling_file);
	auto circuit_istream = std::ifstream(in_dir + circuit_name + IN_POSTFIX);
	auto circuit_expected_istream = std::ifstream(expected_out_dir + circuit_name + OUT_POSTFIX);

	REQUIRE(coupling_istream);
	REQUIRE(circuit_istream);
	REQUIRE(circuit_expected_istream);

	auto coupling_map = MapperUtils::parseCouplingMap(coupling_istream);
	auto circuit = MapperUtils::parseQasm2(circuit_istream);

	auto mapper = toqm::test::MapperBuilder::forSmallCircuits().build();
	auto result = mapper->run(circuit->gateOperations(), circuit->numQubits(), coupling_map);

	std::stringstream outQasm {};
	circuit->toQasm2(outQasm, *result);

	std::stringstream expectedQasm;
	expectedQasm << circuit_expected_istream.rdbuf();

	REQUIRE(outQasm.str() == expectedQasm.str());
}