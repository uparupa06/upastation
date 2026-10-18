#pragma once
#include <fstream>
#include <cstdint>
#include <cstdlib>
#include "json.hpp"
#include "mips.hpp"
#include "utility.hpp"

constexpr int CYCLES_TO_RUN = 4;
constexpr int MAX_FAILURES  = 10;
using json = nlohmann::json;

class TestHandler {
public:
	TestHandler() {}

	void run_test_suite(std::string test) {
		int exception_warnings = 0;
		test_number = 0;
		std::ifstream f(std::string("tests/") + test + std::string(".json"));
		json tests = json::parse(f);
		
		tests_passed = 0;
		tests_failed = 0;

		for (const json& _test : tests) {
			test_number++;
			run_test(_test, exception_warnings, test == "LWL" || test == "LWR" || test == "SWL" || test == "SWR"); // small exception as tests contain bogus error which should be ignored
		}

		if (tests_failed > 0) {
			std::cout << COLOUR_BOLDRED << "Failed " << (int)tests_failed << " tests.\n";
		} else {
			std::cout << COLOUR_BOLDGREEN << "Passed " << (int)tests_passed << " tests.\n";
		}
		if (exception_warnings > 0) {
			std::cout << COLOUR_YELLOW << exception_warnings << " warnings were generated.\n";
		}
	}

	void state_from_json(State& state, json _json) {
		int i = 0;
		for (auto& _r : _json["R"]) {
			state.r[i++] = _r;
		}
		state.hi    = _json["hi"];
		state.lo    = _json["lo"];
		state.epc   = _json["EPC"];
		state.tar   = _json["TAR"];
		state.cause = _json["CAUSE"];
		state.pc    = _json["PC"];

		json delay_branch = _json["delay"]["branch"];
		json delay_load   = _json["delay"]["load"];

		state.delay_branch_slot   = delay_branch["slot"];
		state.delay_branch_take   = delay_branch["take"];
		state.delay_branch_target = delay_branch["target"];

		state.delay_load_target = delay_load["target"];
		state.delay_load_val    = delay_load["val"];
	}

	void run_test(const json& test, int& exception_warnings, bool is_lwl_lwr = false) {
		std::string name = test["name"];
		uint32_t opcode = test["opcode"];
		uint32_t opcode_addr = test["opcode_addr"];
			
		State initial {};
		State final {};
		state_from_json(initial, test["initial"]);
		state_from_json(final,   test["final"]);

		std::vector<Cycle> cycles;
		for (const json& c : test["cycles"]) {
			Cycle cycle;
			cycle.actions = c["actions"];
			cycle.sz      = c["sz"];
			cycle.addr    = c["addr"];
			cycle.val     = c["val"];
			cycles.push_back(cycle);
		}

		mips.load_state(initial);
		mips.load_test_opcode(opcode, opcode_addr);

		for (const Cycle& c : cycles) {
			if (c.actions & READ) {
				mips.seed_read(c.addr, c.sz, c.val);
			}
		}

		mips.clear_bus_log();

		int ticks = 0;
		do {
			mips.tick_component();
		} while (!mips.instruction_done() && ++ticks < CYCLES_TO_RUN);

		bool passed = mips.check_state(final, cycles, test_number, exception_warnings, is_lwl_lwr);
		
		if (passed) {
			tests_passed++;
		} else {
			tests_failed++;
			if (tests_failed >= MAX_FAILURES) {
				std::cout << COLOUR_BOLDRED << "Aborted testing. More than " << (int)(MAX_FAILURES) << " tests failed.\n";
				std::abort();
			}
		}
	}

private:

	MIPS mips;

	int tests_passed = 0;
	int tests_failed = 0;
	int test_number  = 0;

};