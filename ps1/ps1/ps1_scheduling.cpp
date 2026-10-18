#include "ps1.hpp"

const std::string exe = "psxtest_cpu.exe";

void PS1::run() {
	mips.initialise_components();
	if constexpr (SIDELOAD_EXE) {
		while (mips.ps1_pc_fetch() != 0x80030000 && mips.running()) {
			do_tick();
		}
		if (mips.running()) {
			mips.exe_sideload(exe);
		}
	}
	while (mips.running()) {
		mips.tick_component();
	}
}

void PS1::do_tick() {
	// SCHEDULING WILL GO HERE (FOR TIMING)
	mips.tick_component();
}

/*
OLD TEST HANDLER CODE

std::vector<std::string> test_suites = {
	"ADD", "ADDI", "ADDIU", "ADDU",
	"AND", "ANDI",
	"BCondZ", "BEQ", "BGTZ", "BLEZ", "BNE",
	"BREAK",
	"DIV", "DIVU",
	"J", "JAL", "JALR", "JR",
	"LB", "LBU", "LH", "LHU", 
	"LUI",
	"LW", "LWL", "LWR",
	"MFHI", "MFLO", "MTHI", "MTLO", // Correct, but have minor errors
	"MULT", "MULTU",
	"NOR",
	"OR", "ORI",
	"SB", "SHL",
	"SLL", "SLLV",
	"SLT", "SLTU", "SLTI", "SLTIU",
	"SRA", "SRAV", "SRL", "SRLV",
	"SUB", "SUBU",
	"SW", "SWL", "SWR",
	"SYSCALL",
	"XOR", "XORI"
};

TestHandler test_handler;

int count = 0;
int test_total = test_suites.size();
for (const auto& suite : test_suites) {
	std::cout << COLOUR_BOLDWHITE << std::setw(5) << std::left << "Test " << COLOUR_GREEN << std::setw(2) << std::right << (++count) << COLOUR_BOLDWHITE << " of " << COLOUR_GREEN << (int)test_total << COLOUR_BOLDWHITE << " " << std::setw(15) << std::left << COLOUR_BOLDWHITE << std::setw(10) << std::left << suite << " ";
	test_handler.run_test_suite(suite);
}

*/