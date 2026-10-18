#include "mips.hpp"
#include <algorithm>
#include <sstream>
#include <iostream>
#include <iomanip>

void MIPS::load_state(const State& state) {
	cop0.initialise();

	int i = 0;

	for (const auto& _r : state.r) {
		r[i++] = _r;
	}

	hi = state.hi;
	lo = state.lo;

	pc_0 = state.pc;
	pc_1 = (state.delay_branch_slot && state.delay_branch_take) ? state.delay_branch_target : state.pc + 4;

	delay_branch_slot = state.delay_branch_slot;
	delay_branch_take = state.delay_branch_take;
	delay_branch_target = state.delay_branch_target;

	delay_load_target = state.delay_load_target;
	delay_load_val = state.delay_load_val;

	cop0.set_epc_tar_cause(state.epc, state.tar, state.cause);
	
	// Resetting other values
	do_fetch = true;
	branch_condition = false;
	cycle = 0;

	branch_offset = 0;
	branch_target = 0;
	temp_buffer = 0;

	std::fill(std::begin(load_value), std::end(load_value), 0);
	std::fill(std::begin(load_pending), std::end(load_pending), NO_DELAY);

	if (delay_load_target != -1) {
		load_value[delay_load_target] = delay_load_val;
		load_pending[delay_load_target] = DELAYING;
	}

	test_memory.clear();
	test_memory.clear_cycles();
}

void MIPS::load_test_opcode(uint32_t opcode, uint32_t opcode_addr) {
	test_memory.write32(opcode_addr, opcode);
}

bool MIPS::check_state(const State& state, std::vector<Cycle>& expected_cycles, int test_number, int& warnings, bool is_lwl_lwr) {
	std::stringstream failures;
	bool flag;

	int i = 0;

	flag = false;

	for (const auto& _r : state.r) {
		if (r[i] != _r) {
			if (!flag) {
				failures << COLOUR_RED << "Register failure:\n" << COLOUR_RESET;
			}
			flag = true;
			failures << COLOUR_YELLOW << "Received: " << COLOUR_GREEN << "r[" << std::dec << i << "] = " << std::hex << std::setw(8) << std::setfill('0') << r[i] << "\n" << COLOUR_RESET;
			failures << COLOUR_YELLOW << "Expected: " << COLOUR_GREEN << "r[" << std::dec << i << "] = " << std::hex << std::setw(8) << std::setfill('0') << _r   << "\n" << COLOUR_RESET;
		}
		i++;
	}

	// Definitely a better way to do this, but keeping it very naive and simple for now...
	flag = false;
	if (hi != state.hi) {
		failures << COLOUR_CYAN << "Received " << COLOUR_GREEN << "HI = " << std::hex << std::setw(8) << std::setfill('0') << hi << COLOUR_CYAN << ", but expected" << COLOUR_GREEN << " HI = " << std::hex << std::setw(8) << std::setfill('0') << state.hi << "\n" << COLOUR_RESET; 
	}
	if (lo != state.lo) {
		failures << COLOUR_CYAN << "Received " << COLOUR_GREEN << "LO = " << std::hex << std::setw(8) << std::setfill('0') << lo << COLOUR_CYAN << ", but expected" << COLOUR_GREEN << " LO = " << std::hex << std::setw(8) << std::setfill('0') << state.lo << "\n" << COLOUR_RESET; 
	}
	if (pc_0 != state.pc) {
		failures << COLOUR_CYAN << "Received " << COLOUR_GREEN << "PC = " << std::hex << std::setw(8) << std::setfill('0') << pc_0 << COLOUR_CYAN << ", but expected" << COLOUR_GREEN << " PC = " << std::hex << std::setw(8) << std::setfill('0') << state.pc << "\n" << COLOUR_RESET; 
	}
	if (cop0.get_EPC() != state.epc) {
		failures << COLOUR_BLUE << "Received " << COLOUR_GREEN << "EPC = " << std::hex << std::setw(8) << std::setfill('0') << cop0.get_EPC() << COLOUR_BLUE << ", but expected " << COLOUR_GREEN << "EPC = " << std::hex << std::setw(8) << std::setfill('0') << state.epc << "\n" << COLOUR_RESET; 
	}
	if (cop0.get_TAR() != state.tar) {
		failures << COLOUR_BLUE << "Received " << COLOUR_GREEN << "TAR = " << std::hex << std::setw(8) << std::setfill('0') << cop0.get_TAR() << COLOUR_BLUE << ", but expected " << COLOUR_GREEN << "TAR = " << std::hex << std::setw(8) << std::setfill('0') << state.tar << "\n" << COLOUR_RESET; 
	}
	if (cop0.get_CAUSE() != state.cause) {
		if ( (cop0.get_CAUSE() & ~(3u << 28)) == (state.cause & ~(3u << 28))) {
			warnings++;
		} else {
			failures << COLOUR_BLUE << "Received " << COLOUR_GREEN << "CAUSE = " << std::hex << std::setw(8) << std::setfill('0') << cop0.get_CAUSE() << COLOUR_BLUE << ", but expected " << COLOUR_GREEN << "CAUSE = " << std::hex << std::setw(8) << std::setfill('0') << state.cause << "\n" << COLOUR_RESET; 
		}
	}

	if (delay_branch_slot != state.delay_branch_slot || delay_branch_take != state.delay_branch_take || delay_branch_target != state.delay_branch_target) {
		failures << COLOUR_MAGENTA << "Delay slot failure: " << COLOUR_RESET << "\n";
		if (delay_branch_slot != state.delay_branch_slot) {
			if (delay_branch_slot) {
				failures << COLOUR_GREEN << "Next instruction incorrectly placed in delay slot.\n";
			} else {
				failures << COLOUR_GREEN << "Next instruction was not placed in delay slot.\n";
			}
		}
		if (delay_branch_take != state.delay_branch_take) {
			if (delay_branch_take) {
				failures << COLOUR_GREEN << "Branch was incorrectly taken.\n";
			} else {
				failures << COLOUR_GREEN << "Branch was not taken.\n";
			}
		}
		if (delay_branch_target != state.delay_branch_target) {
			failures << COLOUR_GREEN << "Received delay branch target " << std::hex << std::setw(8) << std::setfill('0') << delay_branch_target << ", but expected: " << std::hex << std::setw(8) << std::setfill('0') << state.delay_branch_target << "\n";
		}
		failures << COLOUR_RESET;
	}

	delay_load_target = -1;
	delay_load_val = 0;
	for (int k = 0; k < NUM_MAIN_CPU_REGISTERS; k++) {
		if (load_pending[k] == DELAYING) {
			delay_load_target = k;
			delay_load_val = load_value[k];
		}
	}
	
	if (delay_load_target != state.delay_load_target || delay_load_val != state.delay_load_val) {
		failures << COLOUR_MAGENTA << "Load delay failure: " << COLOUR_RESET << "\n";
		if (delay_load_target != state.delay_load_target) {
			failures << COLOUR_GREEN << "Received load delay target register " << std::hex << std::setw(8) << std::setfill('0') << delay_load_target << ", but expected: " << std::hex << std::setw(8) << std::setfill('0') << state.delay_load_target << "\n";
		}
		if (delay_load_val != state.delay_load_val) {
			failures << COLOUR_GREEN << "Received load delay value " << std::hex << std::setw(8) << std::setfill('0') << delay_load_val << ", but expected: " << std::hex << std::setw(8) << std::setfill('0') << state.delay_load_val << "\n";
		}
		failures << COLOUR_RESET;
	}

	flag = false;
	std::vector<Cycle> received_cycles = test_memory.get_cycles();
	if (received_cycles.size() != expected_cycles.size()) {
		flag = true;
		failures << COLOUR_CYAN << "Bus failure: \n";
		failures << COLOUR_CYAN << "Did " << std::dec << (int)received_cycles.size() << " rwf actions, but expected " << std::dec << (int)expected_cycles.size() << " rwf actions.\n"; 
	}
	if (!flag && !is_lwl_lwr) {
		for (int i = 0; i < expected_cycles.size() && i < received_cycles.size(); i++) {
			Cycle ec = expected_cycles[i];
			Cycle rc = received_cycles[i];
			if (ec.actions != rc.actions || ec.sz != rc.sz || ec.addr != rc.addr || ec.val != rc.val) {
				failures << COLOUR_CYAN << "Bus failure: \n";
				flag = true;
			}
		}
	}
	if (flag && !is_lwl_lwr) {
		failures << COLOUR_CYAN << "Received following bus log: " << "\n";
		print_cycles(received_cycles, failures);
		failures << COLOUR_CYAN << "\nExpected following bus log: " << "\n";
		print_cycles(expected_cycles, failures);
	}

	bool passed = failures.str().empty();

	if (!passed) {
		std::cout << COLOUR_BOLDRED << "\nFailed test " << test_number << ":\n" << COLOUR_RESET;
		std::cout << failures.str() << "\n";
	}

	return passed;
}

bool MIPS::instruction_done() {
	return do_fetch;
}

void MIPS::seed_read(uint32_t addr, int sz, uint32_t value) {
	for (int i = 0; i < 4; i++) {
		test_memory.write8(addr + i, (value >> (8 * i)) & 0xFF, false);
	}
}

void MIPS::clear_bus_log() {
	test_memory.clear_cycles();
}

std::vector<Cycle> MIPS::get_bus_log() {
	return test_memory.get_cycles();
}

void MIPS::print_cycles(std::vector<Cycle>& _cycles, std::stringstream& failures) {
	for (auto& c : _cycles) {
		switch(c.actions) {
			case NOACTION:
				failures << COLOUR_CYAN << "NOACTION, ";
				break;
			case READ:
				failures << COLOUR_CYAN << "READ, ";
				break;
			case WRITE:
				failures << COLOUR_CYAN << "WRITE, ";
				break;
			case FETCH:
				failures << COLOUR_CYAN << "FETCH, ";
				break;
			default:
				failures << COLOUR_CYAN << "UNKNOWN, ";
				break;
		}
		failures << COLOUR_CYAN << c.sz << " bytes, ";
		failures << COLOUR_CYAN << "Address: " << std::hex << std::setw(8) << std::setfill('0') << c.addr << ", ";
		failures << COLOUR_CYAN << "Value: " << std::hex << std::setw(8) << std::setfill('0') << c.val << "\n";
	}
}
