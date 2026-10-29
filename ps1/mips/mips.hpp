#pragma once
#include "instruction.hpp"
#include "cop0.hpp"
#include "cop2.hpp"
#include "memory.hpp"
#include "test_memory.hpp"
#include "utility.hpp"
#include "dma.hpp"
#include <cstdint>
#include <vector>

// PRIORITY -> remove circular dependency with COP0 (and uncomment all usages of COP0 once resolved)

enum LoadDelay : uint8_t {
	NO_DELAY    = 0,
	DELAYING    = 1,
	START_DELAY = 2
};

struct CpuException {
	uint32_t code;
	uint32_t bad_address;
	bool real_exception;
};

constexpr uint8_t NUM_MAIN_CPU_REGISTERS = 32;

class MIPS {
public:

	Instruction instruction;
	COP0 cop0;
	COP2 cop2;

	MIPS();

	// mips.cpp
	void tick_component();
	void putchar();
	void initialise();
	void end();
	Instruction fetch_instruction();
	uint64_t get_cycle();
	uint32_t ps1_pc_fetch(); // pc_fetch for PS1 (for timing EXE sideloading)
	void jump_to_exception(uint32_t vector);
	uint32_t read_lwl_lwr(int target_register);
	void do_lwc(const Instruction& instruction, int cop);
	void do_mfc(const Instruction& instruction, int cop);
	void do_mtc(const Instruction& instruction, int cop);
	void do_cfc(const Instruction& instruction, int cop);
	void do_ctc(const Instruction& instruction, int cop);
	void do_cop(const Instruction& instruction, int cop);
	void cop_branch(const Instruction& instruction, int cop);

	// delay_slot.cpp
	uint32_t pipeline();
	uint32_t branch_pc();
	void pipe(uint32_t pc);
	void next();
	void initialise_pc(uint32_t initial_pc);
	void branch_to(uint32_t target);
	uint32_t delay_slot_pc();
	uint32_t branch_target_addr();
	bool branch_taken();

	// load_delay.cpp
	void update_load_delay();
	void load(int register_number, uint32_t value);
	void write_register(int register_number, uint32_t value);

	// mips_testing.cpp (for test handler)
	void load_state(const State& state);
	void load_test_opcode(uint32_t opcode, uint32_t opcode_addr);
	bool check_state(const State& state, std::vector<Cycle>& expected_cycles, int test_number, int& warnings, bool is_lwl_lwr = false);
	bool instruction_done();
	void seed_read(uint32_t addr, int sz, uint32_t value);
	void clear_bus_log();
	std::vector<Cycle> get_bus_log();
	void print_cycles(std::vector<Cycle>& _cycles, std::stringstream& failures);

	// mips_rw (also has some behaviour differing based on whether testing is occurring or not)
	void check_memory(uint32_t addr, bool is_store, bool is_fetch);

	// mips_exe.cpp
	uint64_t read_from_exe(uint32_t start, uint32_t end);
	void exe_sideload(const std::string& exe_file);

	uint8_t   read8(uint32_t addr);
	uint16_t read16(uint32_t addr);
	uint32_t read32(uint32_t addr);

	void  write8(uint32_t addr, uint8_t value);
	void write16(uint32_t addr, uint16_t value);
	void write32(uint32_t addr, uint32_t value);

	uint32_t fetch();

	void set_isolate_cache(bool on);

	// mips.cpp
	// Primary opcode table instruction declarations
	void execute_bcondz(const Instruction& instruction);
	void execute_j(const Instruction& instruction);
	void execute_jal(const Instruction& instruction);
	void execute_beq(const Instruction& instruction);
	void execute_bne(const Instruction& instruction);
	void execute_blez(const Instruction& instruction);
	void execute_bgtz(const Instruction& instruction);
	void execute_addi(const Instruction& instruction);
	void execute_addiu(const Instruction& instruction);
	void execute_slti(const Instruction& instruction);
	void execute_sltiu(const Instruction& instruction);
	void execute_andi(const Instruction& instruction);
	void execute_ori(const Instruction& instruction);
	void execute_xori(const Instruction& instruction);
	void execute_lui(const Instruction& instruction);
	void execute_cop0(const Instruction& instruction);
	void execute_cop1(const Instruction& instruction);
	void execute_cop2(const Instruction& instruction);
	void execute_cop3(const Instruction& instruction);
	void execute_lb(const Instruction& instruction);
	void execute_lh(const Instruction& instruction);
	void execute_lwl(const Instruction& instruction);
	void execute_lw(const Instruction& instruction);
	void execute_lbu(const Instruction& instruction);
	void execute_lhu(const Instruction& instruction);
	void execute_lwr(const Instruction& instruction);
	void execute_sb(const Instruction& instruction);
	void execute_sh(const Instruction& instruction);
	void execute_swl(const Instruction& instruction);
	void execute_sw(const Instruction& instruction);
	void execute_swr(const Instruction& instruction);
	void execute_lwc0(const Instruction& instruction);
	void execute_lwc1(const Instruction& instruction);
	void execute_lwc2(const Instruction& instruction);
	void execute_lwc3(const Instruction& instruction);
	void execute_swc0(const Instruction& instruction);
	void execute_swc1(const Instruction& instruction);
	void execute_swc2(const Instruction& instruction);
	void execute_swc3(const Instruction& instruction);

	// Secondary opcode table instruction declarations
	void execute_sll(const Instruction& instruction);
	void execute_srl(const Instruction& instruction);
	void execute_sra(const Instruction& instruction);
	void execute_sllv(const Instruction& instruction);
	void execute_srlv(const Instruction& instruction);
	void execute_srav(const Instruction& instruction);
	void execute_jr(const Instruction& instruction);
	void execute_jalr(const Instruction& instruction);
	void execute_syscall(const Instruction& instruction);
	void execute_break(const Instruction& instruction);
	void execute_mfhi(const Instruction& instruction);
	void execute_mthi(const Instruction& instruction);
	void execute_mflo(const Instruction& instruction);
	void execute_mtlo(const Instruction& instruction);
	void execute_mult(const Instruction& instruction);
	void execute_multu(const Instruction& instruction);
	void execute_div(const Instruction& instruction);
	void execute_divu(const Instruction& instruction);
	void execute_add(const Instruction& instruction);
	void execute_addu(const Instruction& instruction);
	void execute_sub(const Instruction& instruction);
	void execute_subu(const Instruction& instruction);
	void execute_and(const Instruction& instruction);
	void execute_or(const Instruction& instruction);
	void execute_xor(const Instruction& instruction);
	void execute_nor(const Instruction& instruction);
	void execute_slt(const Instruction& instruction);
	void execute_sltu(const Instruction& instruction);

	// For exception handler (some for testing too)
	uint32_t current_pc = 0;
	bool in_delay_slot = false;
	bool current_branch_take = false;
	uint32_t current_branch_target = 0;

	bool running() {
		return memory.running();
	}

	void sdl_loop() {
		memory.sdl_loop();
	}

	void initialise_components() {
		memory.initialise();
	}

	uint64_t get_tick() {
		return mips_tick;
	}

	DMA* get_dma() {
		return memory.get_dma();
	}

	int get_frames() {
		return frames;
	}

	void tick_timers() {
		memory.tick_timers();
	}
	
private:

	uint64_t mips_tick = 0;

	void do_branch(const Instruction& instruction, bool taken);

	// For execution of instructions
	bool do_fetch = true;
	bool branch_condition = false;
	uint8_t i_cycle = 0; // Cycle within the instruction
	uint64_t cycle = 0; // Next cycle to be ticked on
	Instruction current_instruction = 0;

	uint16_t branch_offset = 0;
	uint32_t branch_target = 0;
	uint32_t temp_buffer = 0;

	// registers

	uint32_t r[NUM_MAIN_CPU_REGISTERS] = {};
	uint32_t hi = 0;
	uint32_t lo = 0;

	// delay_slot.cpp -> for the delay slot quirk
	uint32_t pc_0; // current PC to be executed
	uint32_t pc_1; // next PC

	// load_delay.cpp -> for load delays for general purpose registers
	uint32_t  load_value[NUM_MAIN_CPU_REGISTERS] = {};
	LoadDelay load_pending[NUM_MAIN_CPU_REGISTERS] = {};

	// mips_testing.cpp (for test handler)
	bool delay_branch_slot;
	bool delay_branch_take;
	uint32_t delay_branch_target;

	int64_t delay_load_target;
	uint32_t delay_load_val;

	// Memory
	Memory memory;
	TestMemory test_memory;

	// For loads
	uint8_t byte;
	uint16_t half_word;
	uint32_t word;
	uint32_t effective_address;

	// For EXE sideloading
	std::vector<uint8_t> exe {};

	// For exception
	CpuException exception = { 0, 0, false };

	// Temporary
	int frames = 0;
};