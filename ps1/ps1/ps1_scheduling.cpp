#include "ps1.hpp"
#include <chrono>

const std::string exe = "RenderPolygon/RenderPolygon16BPP.exe";

void PS1::run() {
	initialise();
	mips.initialise_components();

	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

	if constexpr (SIDELOAD_EXE) {
		while (mips.ps1_pc_fetch() != 0x80030000 && mips.running()) {
			do_tick();
		}
		if (mips.running()) {
			mips.exe_sideload(exe);
		}
	}
	while (mips.running()) {
		do_tick();
	}

	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

	int frames = mips.get_frames();

	float fps = (float)frames / (float)(std::chrono::duration_cast<std::chrono::seconds>(end - begin).count());
	std::cout << "FPS " << (int)fps << "\n";
}

void PS1::initialise() {
	dma = mips.get_dma();
}

void PS1::do_tick() {
	// SCHEDULING WILL GO HERE (FOR TIMING)
	// NOTE TO SELF: OVERFLOW CAUSED ISSUE WITH SCHEDULING HERE!
	// NEED TO CREATE A MORE COMPLICATED SCHEDULING SYSTEM...
	mips.tick_component();
	mips.tick_timers();
	mips.tick_timers();
	// BASIC SOLUTION FOR NOW!

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