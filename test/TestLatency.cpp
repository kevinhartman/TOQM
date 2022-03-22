#include <catch2/catch.hpp>

#include "util/ComparisonHelpers.hpp"
#include "util/MapperBuilder.hpp"
#include "util/PrinterHelpers.hpp"

#include <libtoqm/libtoqm.hpp>
#include <vector>
#include <ostream>
#include <tuple>

using namespace toqm::test;

TEST_CASE("Latency table can be configured to behave like simple latencies.", "[latency]") {
	std::vector<toqm::GateOp> gates = {
			toqm::GateOp(0, "cx", 0, 1),
			toqm::GateOp(1, "cx", 0, 2),
			toqm::GateOp(2, "cx", 0, 4),
			toqm::GateOp(3, "cx", 1, 2),
			toqm::GateOp(4, "cx", 1, 4),
			toqm::GateOp(5, "cx", 2, 4)
	};

	auto coupling_map = toqm::CouplingMap {
			7,
			{
					{0, 1},
					{1, 2},
					{2, 3},
					{3, 4},
					{4, 5},
					{5, 6},
			}
	};

	std::vector<toqm::LatencyDescription> latencies {};

	for (int x = 0; x < 7; x++) {
		for (int y = 0; y < 7; y++) {
			if (x != y) {
				latencies.emplace_back("cx", x, y, 2);
				latencies.emplace_back("swap", x, y, 6);
			}
		}
	}

	auto table = std::unique_ptr<toqm::Latency>(new toqm::Table(latencies));
	auto simple = std::unique_ptr<toqm::Latency>(new toqm::Latency_1_2_6());

	for (int x = 0; x < 7; x++) {
		for (int y = 0; y < 7; y++) {
			if (x != y) {
				REQUIRE(table->getLatency("cx", 2, x, y) == simple->getLatency("cx", 2, x, y));
				REQUIRE(table->getLatency("swap", 2, x, y) == simple->getLatency("swap", 2, x, y));
			}
		}
	}

	REQUIRE(table->getLatency("cx", 2, -1, -1) == simple->getLatency("cx", 2, -1, -1));
	REQUIRE(table->getLatency("swap", 2, -1, -1) == simple->getLatency("swap", 2, -1, -1));

	auto simple_mapper = MapperBuilder::forSmallCircuits();
	simple_mapper.Latency = std::move(simple);
	auto simple_result = simple_mapper.build()->run(gates, 7, coupling_map);

	auto table_mapper = MapperBuilder::forSmallCircuits();
	table_mapper.Latency = std::move(table);
	auto table_result = table_mapper.build()->run(gates, 7, coupling_map);

	REQUIRE(std::equal(simple_result->scheduledGates.begin(), simple_result->scheduledGates.end(), table_result->scheduledGates.begin()));
}

TEST_CASE("Test 0-latency instructions work as expected.", "[latency]") {
	auto coupling_map = toqm::CouplingMap {
			3,
			{
					{0, 1},
					{1, 2},
			}
	};

	std::vector<toqm::GateOp> gates = {
			toqm::GateOp(0, "cx", 0, 1),
			toqm::GateOp(1, "rz", 1),
			toqm::GateOp(2, "cx", 1, 0),
	};

	std::vector<toqm::LatencyDescription> latencies {};

	latencies.emplace_back(1, "rz", 0);
	latencies.emplace_back(2, "cx", 2);
	latencies.emplace_back(2, "swap", 6);

	auto table = std::unique_ptr<toqm::Latency>(new toqm::Table(latencies));

	auto mapper = MapperBuilder::forSmallCircuits();
	mapper.Latency = std::move(table);

	auto result = mapper.build()->run(gates, coupling_map.numPhysicalQubits, coupling_map, 0);

	// Require order is the same, since this is the only valid ordering for this circuit.
	REQUIRE(result->scheduledGates[0].gateOp.uid == 0);
	REQUIRE(result->scheduledGates[1].gateOp.uid == 1);
	REQUIRE(result->scheduledGates[2].gateOp.uid == 2);

	// Require 0-duration RZ happens in the same cycle as first CX.
	REQUIRE(result->scheduledGates[0].cycle == 0);
	REQUIRE(result->scheduledGates[1].cycle == 0);
	REQUIRE(result->scheduledGates[2].cycle == 2);
}

TEST_CASE("Test 0-latency instructions at start of circuit.", "[latency]") {
	auto coupling_map = toqm::CouplingMap {
			3,
			{
					{0, 1},
					{1, 2},
			}
	};

	std::vector<toqm::GateOp> gates = {
			toqm::GateOp(0, "rz", 1),
			toqm::GateOp(1, "rz", 1),
			toqm::GateOp(2, "cx", 0, 1),
			toqm::GateOp(3, "cx", 1, 0),
	};

	std::vector<toqm::LatencyDescription> latencies {};

	latencies.emplace_back(1, "rz", 0);
	latencies.emplace_back(2, "cx", 2);
	latencies.emplace_back(2, "swap", 6);

	auto table = std::unique_ptr<toqm::Latency>(new toqm::Table(latencies));

	auto mapper = MapperBuilder::forSmallCircuits();
	mapper.Latency = std::move(table);

	auto result = mapper.build()->run(gates, coupling_map.numPhysicalQubits, coupling_map, 0);

	std::cout << result->scheduledGates;

	// Require order is the same, since this is the only valid ordering for this circuit.
	REQUIRE(result->scheduledGates[0].gateOp.uid == 0);
	REQUIRE(result->scheduledGates[1].gateOp.uid == 1);
	REQUIRE(result->scheduledGates[2].gateOp.uid == 2);
	REQUIRE(result->scheduledGates[3].gateOp.uid == 3);

	// Require first CX (non-zero duration) happens in the 0th cycle after RZs.
	REQUIRE(result->scheduledGates[0].cycle == 0);
	REQUIRE(result->scheduledGates[1].cycle == 0);
	REQUIRE(result->scheduledGates[2].cycle == 0);
	REQUIRE(result->scheduledGates[3].cycle == 2);
}