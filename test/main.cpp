#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <libtoqm/libtoqm.hpp>
#include <vector>
#include <ostream>
#include <tuple>

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

class MapperBuilder {
public:
	MapperBuilder() = default;

	std::unique_ptr<toqm::ToqmMapper> build() {
		return std::unique_ptr<toqm::ToqmMapper>(new toqm::ToqmMapper(
				*this->Queue,
				this->Expander->clone(),
				this->CostFunc->clone(),
				this->Latency->clone(),
				this->NodeMods(),
				this->Filters()
		));
	}

	std::unique_ptr<toqm::Queue> Queue = std::unique_ptr<toqm::Queue>(new toqm::DefaultQueue());
	std::unique_ptr<toqm::Expander> Expander = std::unique_ptr<toqm::Expander>(new toqm::DefaultExpander());
	std::unique_ptr<toqm::CostFunc> CostFunc = std::unique_ptr<toqm::CostFunc>(new toqm::CXFrontier());
	std::unique_ptr<toqm::Latency> Latency = std::unique_ptr<toqm::Latency>(new toqm::Latency_1_2_6());
	std::unique_ptr<toqm::NodeMod> NodeMod = nullptr;
	std::unique_ptr<toqm::Filter> Filter1 = std::unique_ptr<toqm::Filter>(new toqm::HashFilter());
	std::unique_ptr<toqm::Filter> Filter2 =	std::unique_ptr<toqm::Filter>(new toqm::HashFilter2());

private:
	std::vector<std::unique_ptr<toqm::NodeMod>> NodeMods() const {
		std::vector<std::unique_ptr<toqm::NodeMod>> mods{};
		if (this->NodeMod != nullptr) {
			mods.push_back(this->NodeMod->clone());
		}

		return std::move(mods);
	}

	std::vector<std::unique_ptr<toqm::Filter>> Filters() const {
		std::vector<std::unique_ptr<toqm::Filter>> filters{};
		if (this->Filter1 != nullptr) {
			filters.push_back(this->Filter1->clone());
		}

		if (this->Filter2 != nullptr) {
			filters.push_back(this->Filter2->clone());
		}

		return std::move(filters);
	}
};

namespace toqm {
bool operator==(ScheduledGateOp const & lhs, ScheduledGateOp const & rhs) {
	return std::tie(lhs.cycle, lhs.latency, lhs.physicalControl, lhs.physicalTarget) ==
		   std::tie(rhs.cycle, rhs.latency, rhs.physicalControl, rhs.physicalTarget);
}
}

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

	MapperBuilder simple_mapper{};
	simple_mapper.Latency = std::move(simple);
	auto simple_result = simple_mapper.build()->run(gates, 7, coupling_map);

	//printNode(std::cout, simple_result->scheduledGates);
	//std::cout << std::endl;

	MapperBuilder table_mapper{};
	table_mapper.Latency = std::move(table);
	auto table_result = table_mapper.build()->run(gates, 7, coupling_map);

	//printNode(std::cout, table_result->scheduledGates);

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
	//latencies.emplace_back("cx", 1, 0, 1);
	latencies.emplace_back(2, "swap", 6);

	auto table = std::unique_ptr<toqm::Latency>(new toqm::Table(latencies));

	MapperBuilder mapper{};
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
	//latencies.emplace_back("cx", 1, 0, 1);
	latencies.emplace_back(2, "swap", 6);

	auto table = std::unique_ptr<toqm::Latency>(new toqm::Table(latencies));

	MapperBuilder mapper{};
	mapper.Latency = std::move(table);

	auto result = mapper.build()->run(gates, coupling_map.numPhysicalQubits, coupling_map, 0);

	printNode(std::cout, result->scheduledGates);

	// Require order is the same, since this is the only valid ordering for this circuit.
	REQUIRE(result->scheduledGates[0].gateOp.uid == 0);
	REQUIRE(result->scheduledGates[1].gateOp.uid == 1);
	REQUIRE(result->scheduledGates[2].gateOp.uid == 2);
	REQUIRE(result->scheduledGates[3].gateOp.uid == 3);

	// Require 0-duration RZ happens in the same cycle as first CX.
	REQUIRE(result->scheduledGates[0].cycle == 0);
	REQUIRE(result->scheduledGates[1].cycle == 0);
	REQUIRE(result->scheduledGates[2].cycle == 0);
	REQUIRE(result->scheduledGates[3].cycle == 2);
}