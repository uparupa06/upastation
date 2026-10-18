#include "mips.hpp"

uint32_t MIPS::pipeline() {
	uint32_t _pc = pc_0;
	next();
	return _pc;
}

uint32_t MIPS::branch_pc() {
	return pc_1; // Just having this helper in case I have messed up this concept
}

void MIPS::pipe(uint32_t pc) {
	pc_0 = pc_1;
	pc_1 = pc;
}

void MIPS::next() {
	pipe(pc_1 + 4);
}

void MIPS::initialise_pc(uint32_t initial_pc) {
	pc_1 = initial_pc;
	next();
}

void MIPS::branch_to(uint32_t target) {
	pc_1 = target;
}

uint32_t MIPS::delay_slot_pc() {
	return pc_0;
}

uint32_t MIPS::branch_target_addr() {
	return current_branch_target;
}

bool MIPS::branch_taken() {
	return current_branch_take;
}