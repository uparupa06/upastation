#include "mips.hpp"
#include "utility.hpp"
#include <limits>

// ALL INSTRUCTIONS PASS FOR THE MOST PART
// SEARCH 'FOR LATER' TO FIND COPROCESSOR OPCODES (ONLY REMAINING ONES TO BE IMPLEMENTED)
// ALL branch_pc() + 4 might be incorrect

MIPS::MIPS() {
	cop0.connect_mips(this);
	initialise();
}

uint32_t MIPS::ps1_pc_fetch() {
	return pc_0;
}

void MIPS::tick_component() {
	// Fetch the instruction (if a new fetch is to occur)
	mips_tick++;
	i_cycle++; // MORE TO BE ADDED HERE AS INSTRUCTIONS MIGHT TAKE VARIABLE CYCLES BASED ON MEMORY ACCESSES
	cycle++;
	if (do_fetch) {
		current_pc = pc_0;
		in_delay_slot = delay_branch_slot;
		current_branch_take = delay_branch_take;
		current_branch_target = delay_branch_target;
		delay_branch_slot = false;
		delay_branch_take = false;
		delay_branch_target = 0;
		current_instruction = fetch();
		do_fetch = false;
		i_cycle = 0;
	}

	try {
		if (current_instruction.definition) {
			(this->*current_instruction.definition->execute)(current_instruction);
			putchar();
		} else if (TESTING_CPU) {
			end();
		} else {
			cop0.raise_RI_exception();
		}
	} catch (const CpuException& e) {
		if constexpr (END_UPON_EXCEPTION) {
			std::cerr << "Fault " << (int)e.code << "Bad address " << std::hex << std::setw(8) << std::setfill('0') << (int)e.bad_address << "\n";
		} else {
			if (e.code == 4 || e.code == 5) {
				cop0.set_BadA(e.bad_address);
			}
			cop0.set_ExcCode(e.code);
			cop0.handle_exception();
		}
	}
	

	if (do_fetch) {
		update_load_delay();
	}
 	
 	// TEMPORARY FOR NOW
	if (mips_tick >= 564500) {
		sdl_loop();
		mips_tick = 0;
	}
}

void MIPS::putchar() {
	uint32_t pc_val = current_pc & 0x1FFFFFFF;
	if ((pc_val == 0xA0 && r[9] == 0x3C) || (pc_val == 0xB0 && r[9] == 0x3D)) {
		std::cout << (char)(static_cast<uint8_t>(r[4]));
	}
}

uint64_t MIPS::get_cycle() {
	return cycle;
}

void MIPS::initialise() {
	cop0.initialise();

	initialise_pc(0xBFC00000);
	hi = 0;
	lo = 0;
	for (auto& _r : r) {
		_r = 0;
	}

	do_fetch = true;
	branch_condition = false;
	i_cycle = 0;
	cycle = 0;
}

void MIPS::end() {
	i_cycle = 0;
	do_fetch = true;
}

void MIPS::jump_to_exception(uint32_t vector) {
	pc_0 = vector;
	pc_1 = vector + 4;
	delay_branch_slot = false;
	delay_branch_take = false;
	delay_branch_target = 0;
	do_fetch = true;
}

/* NEED TO WRITE THIS!
Instruction MIPS::fetch_instruction() {
	// TO MAKE

	// Must reset cycle

	cycle = 0;
}
*/

constexpr int32_t sign_extend(uint16_t value) {
	return static_cast<int16_t>(value);
}

constexpr uint32_t get_branch_offset(const Instruction& instruction) {
    return static_cast<uint32_t>(sign_extend(instruction.imm16)) << 2;
}

void MIPS::do_branch(const Instruction& instruction, bool taken) {
	uint32_t target = delay_slot_pc() + get_branch_offset(instruction);
	delay_branch_slot = true;
	delay_branch_take = taken;
	delay_branch_target = target;
	if (taken) {
		branch_to(target);
	}
}

void MIPS::execute_add(const Instruction& instruction) {
	if (add_overflows(r[instruction.rs], r[instruction.rt])) {
		cop0.raise_Ovf_exception();
	} else {
		write_register(instruction.rd, r[instruction.rs] + r[instruction.rt]);
	}
	end();
}

void MIPS::execute_addi(const Instruction& instruction) {
	uint32_t a = r[instruction.rs];
	uint32_t b = sign_extend(instruction.imm16);
	if (add_overflows(a, b)) {
		cop0.raise_Ovf_exception();
	} else {
		write_register(instruction.rt, a + b);
	}
	end();
}

void MIPS::execute_addiu(const Instruction& instruction) {
	uint32_t a = r[instruction.rs];
	uint32_t b = sign_extend(instruction.imm16);
	write_register(instruction.rt, a + b);
	end();
}

void MIPS::execute_addu(const Instruction& instruction) {
	write_register(instruction.rd, r[instruction.rs] + r[instruction.rt]);
	end();
}

void MIPS::execute_and(const Instruction& instruction) {
	write_register(instruction.rd, r[instruction.rs] & r[instruction.rt]);
	end();
}

void MIPS::execute_andi(const Instruction& instruction) {
	write_register(instruction.rt, r[instruction.rs] & instruction.imm16);
	end();
}

void MIPS::execute_beq(const Instruction& instruction) {
	if (i_cycle == 1) {
		do_branch(instruction, r[instruction.rs] == r[instruction.rt]);
		end();
	}
}

void MIPS::execute_bcondz(const Instruction& instruction) {
	if (i_cycle == 1) {
		bool negative = get_bit(r[instruction.rs], 31);
		bool condition = (instruction.rt & 1) ? !negative : negative;
		bool link = (instruction.rt & 0x1E) == 0x10;
		do_branch(instruction, condition);
		if (link) {
			write_register(31, delay_slot_pc() + 4);
		}
		end();
	}
}
void MIPS::execute_bgtz(const Instruction& instruction) {
	if (i_cycle == 1) {
		do_branch(instruction, !get_bit(r[instruction.rs], 31) && r[instruction.rs]);
		end();
	}
}

void MIPS::execute_blez(const Instruction& instruction) {
	if (i_cycle == 1) {
		do_branch(instruction, get_bit(r[instruction.rs], 31) || !r[instruction.rs]);
		end();
	}
}

void MIPS::execute_bne(const Instruction& instruction) {
	if (i_cycle == 1) {
		do_branch(instruction, r[instruction.rs] != r[instruction.rt]);
		end();
	}
}

void MIPS::execute_break(const Instruction& instruction) {
	cop0.raise_Bp_exception();
	end();
}

// FOR LATER
/*
UNDER COP:
0b0100xx* (primary opcode)
if bits 21 to 25 == 0b00000, then it is MFC
if bits 21 to 25 == 0b00100, then it is MTC
if bits 21 to 25 == 0b00010, then it is CFC
if bits 21 to 25 == 0b00110, then it is CTC
if bit 25 == 1, then it is COP
*/

void MIPS::do_mfc(const Instruction& instruction, int cop) {
	if (cop == 3) {
		cop0.raise_RI_exception();
	} else if (cop == 0) {
		uint32_t data = cop0.read(instruction.rd);
		load(instruction.rt, data);
	} else if (cop == 2) {
		uint32_t data = cop2.read(instruction.rd);
		load(instruction.rt, data);
	} else {
		cop0.raise_CpU_exception();
	}
}

void MIPS::do_mtc(const Instruction& instruction, int cop) {
	uint32_t data = r[instruction.rt];
	if (cop == 0) {
		cop0.write(instruction.rd, data);
	} else if (cop == 2) {
		cop2.write(instruction.rd, data);
	} else {
		cop0.raise_CpU_exception();
	}
}

void MIPS::do_cfc(const Instruction& instruction, int cop) {
	if (cop == 2) {
		uint32_t data = cop2.read(instruction.rd);
		load(instruction.rt, data);
	} else {
		cop0.raise_CpU_exception();
	}
}

void MIPS::do_ctc(const Instruction& instruction, int cop) {
	if (cop == 2) {
		uint32_t data = r[instruction.rt];
		cop2.write(instruction.rd, data);
	} else {
		cop0.raise_CpU_exception();
	}
}

void MIPS::do_cop(const Instruction& instruction, int cop) {
	if (cop == 0) {
		cop0.do_cofunc(instruction.imm25);
	} else if (cop == 2) {
		cop2.do_cofunc(instruction.imm25);
	} else {
		cop0.raise_CpU_exception();
	}
}

void MIPS::cop_branch(const Instruction& instruction, int cop) {
	switch (instruction.rs) {
		case 0b00000:
			do_mfc(instruction, cop);
			break;
		case 0b00100:
			do_mtc(instruction, cop);
			break;
		case 0b00010:
			do_cfc(instruction, cop);
			break;
		case 0b00110:
			do_ctc(instruction, cop);
			break;
		default: {
			if (instruction.rs & 0x10) {
				do_cop(instruction, cop);
			}
			break;
		}
	}
}

void MIPS::execute_cop0(const Instruction& instruction) {
	cop_branch(instruction, 0);
	end();
}

void MIPS::execute_cop1(const Instruction& instruction) {
	cop_branch(instruction, 1);
	end();
}

void MIPS::execute_cop2(const Instruction& instruction) {
	cop_branch(instruction, 2);
	end();
}

void MIPS::execute_cop3(const Instruction& instruction) {
	cop_branch(instruction, 3);
	end();
}

void MIPS::execute_div(const Instruction& instruction) {
	int32_t rs = static_cast<int32_t>(r[instruction.rs]);
	int32_t rt = static_cast<int32_t>(r[instruction.rt]);

	if (rt == 0 && rs >= 0) {
		lo = -1;
		hi = rs;
	}
	else if (rt == 0 && rs < 0) {
		lo = 1;
		hi = rs;
	}
	else if (rt == -1 && rs == 0x80000000) {
		lo = 0x80000000;
		hi = 0;
	} else {
		lo = rs / rt;
		hi = rs % rt;
	}
	end();
}

void MIPS::execute_divu(const Instruction& instruction) {
	uint32_t rs = r[instruction.rs];
	uint32_t rt = r[instruction.rt];

	if (rt == 0) {
		lo = 0xFFFFFFFF;
		hi = rs;
	} else {
		lo = rs / rt;
		hi = rs % rt;
	}
	end();
}

void MIPS::execute_j(const Instruction& instruction) {
	if (i_cycle == 1) {
		uint32_t new_pc = (extract_bits(delay_slot_pc(), 28, 31) << 28)
		                | (instruction.imm26 << 2);
		delay_branch_slot   = true;
		delay_branch_take   = true;
		delay_branch_target = new_pc;
		branch_to(new_pc);
		end();
	}
}

void MIPS::execute_jal(const Instruction& instruction) {
	if (i_cycle == 0) {
		write_register(31, delay_slot_pc() + 4);
	} else {
		uint32_t new_pc = (extract_bits(delay_slot_pc(), 28, 31) << 28)
		                | (instruction.imm26 << 2);
		delay_branch_slot   = true;
		delay_branch_take   = true;
		delay_branch_target = new_pc;
		branch_to(new_pc);
		end();
	}
}

void MIPS::execute_jalr(const Instruction& instruction) {
	if (i_cycle == 0) {
		temp_buffer = r[instruction.rs];
		write_register(instruction.rd, delay_slot_pc() + 4);
	} else {
		uint32_t new_pc = temp_buffer;
		delay_branch_slot   = true;
		delay_branch_take   = true;
		delay_branch_target = new_pc;
		branch_to(new_pc);
		end();
	}
}

void MIPS::execute_jr(const Instruction& instruction) {
	if (i_cycle == 0) {
		temp_buffer = r[instruction.rs];
	} else {
		uint32_t new_pc = temp_buffer;
		delay_branch_slot   = true;
		delay_branch_take   = true;
		delay_branch_target = new_pc;
		branch_to(new_pc);
		end();
	}
}

// LATER
void MIPS::execute_lb(const Instruction& instruction) {
	if (i_cycle == 0) {
		byte = read8((int)r[instruction.rs] + (int)sign_extend(instruction.imm16));
	} else {
		load(instruction.rt, static_cast<int32_t>(static_cast<int8_t>(byte)));
		end();
	}
}

void MIPS::execute_lbu(const Instruction& instruction) {
	if (i_cycle == 0) {
		byte = read8((int)r[instruction.rs] + (int)sign_extend(instruction.imm16));
	} else {
		load(instruction.rt, byte);
		end();
	}
}

void MIPS::execute_lh(const Instruction& instruction) {
	if (i_cycle == 0) {
		effective_address = (int)r[instruction.rs] + (int)sign_extend(instruction.imm16);
	} else {
		if (effective_address & 1) {
			cop0.set_BadA(effective_address); // MAY BE AN INCORRECT BAD ADDRESS SET
			cop0.raise_AdEL_exception();
		} else {
			half_word = read16(effective_address);
			load(instruction.rt, static_cast<int32_t>(static_cast<int16_t>(half_word)));
		}
		end();
	}
}

void MIPS::execute_lhu(const Instruction& instruction) {
	if (i_cycle == 0) {
		effective_address = (int)r[instruction.rs] + (int)sign_extend(instruction.imm16);
	} else {
		if (effective_address & 1) {
			cop0.set_BadA(effective_address); // MAY BE AN INCORRECT BAD ADDRESS SET
			cop0.raise_AdEL_exception();
		} else {
			half_word = read16(effective_address);
			load(instruction.rt, half_word);
		}
		end();
	}
}

void MIPS::execute_lui(const Instruction& instruction) {
	write_register(instruction.rt, instruction.imm16 << 16);
	end();
}

void MIPS::execute_lw(const Instruction& instruction) {
	if (i_cycle == 0) {
		effective_address = (int)r[instruction.rs] + (int)sign_extend(instruction.imm16);
	} else {
		if (effective_address & 3) {
			cop0.set_BadA(effective_address); // MAY BE AN INCORRECT BAD ADDRESS SET
			cop0.raise_AdEL_exception();
		} else {
			word = read32(effective_address);
			load(instruction.rt, static_cast<int32_t>(word));
		}
		end();
	}
}

// Special 'circuitry' for LWL/LWR (bypassing load delays)
uint32_t MIPS::read_lwl_lwr(int target_register) {
	if (load_pending[target_register] == DELAYING) {
		return load_value[target_register];
	} else {
		return r[target_register];
	}
}

// LSB of SRC into MSB of DST
void MIPS::execute_lwl(const Instruction& instruction) {
	uint32_t dst = read_lwl_lwr(instruction.rt);
	effective_address = (int)r[instruction.rs] + (int)sign_extend(instruction.imm16);
		
	uint32_t src = 0x00;
	switch (effective_address & 3) {
	case 0: // 1-byte read
		src = read8(effective_address);
		dst = (src << 24) | (dst & 0x00FFFFFF);
		break;
	case 1: // 2-byte read
		src = read16(effective_address - 1);
		dst = (src << 16) | (dst & 0x0000FFFF);
		break;
	case 2: // 3-byte read
		src = read16(effective_address - 2);
		dst = (src << 8) | (dst & 0xFF0000FF);
		src = read8(effective_address);
		dst = (src << 24)  | (dst & 0x00FFFFFF);
		break;
	case 3: // 4-byte read
		src = read32(effective_address - 3);
		dst = src;
		break;
	default:
		end();
		return;
	}
	load(instruction.rt, dst);
	end();
}


// MSB of SRC into LSB of DST
void MIPS::execute_lwr(const Instruction& instruction) {
	uint32_t dst = read_lwl_lwr(instruction.rt);
	effective_address = (int)r[instruction.rs] + (int)sign_extend(instruction.imm16);
		
	uint32_t src = 0x00;
	switch (effective_address & 3) {
	case 3: // 1-byte read
		src = read8(effective_address);
		dst = src | (dst & 0xFFFFFF00);
		break;
	case 2: // 2-byte read
		src = read16(effective_address);
		dst = src | (dst & 0xFFFF0000);
		break;
	case 1: // 3-byte read
		src = read16(effective_address);
		dst = src | (dst & 0xFFFF0000);
		src = read8(effective_address + 2);
		dst = (src << 16)  | (dst & 0xFF00FFFF);
		break;
	case 0: // 4-byte read
		src = read32(effective_address);
		dst = src;
		break;
	default:
		end();
		return;
	}
	load(instruction.rt, dst);
	end();
}

// TEMPORARY (NEED TO ADD QUIRKS TO MFHI, MFLO, MTHI, MTHLO)
// IDEA WILL BE TO USE COUNTERS, WORK THIS OUT!

void MIPS::execute_mfhi(const Instruction& instruction) {
	if (instruction.rd == 0) {
		end();
		return;
	}
	write_register(instruction.rd, hi);
	end();
}

void MIPS::execute_mflo(const Instruction& instruction) {
	if (instruction.rd == 0) {
		end();
		return;
	}
	write_register(instruction.rd, lo);
	end();
}

void MIPS::execute_mthi(const Instruction& instruction) {
	hi = r[instruction.rs];
	end();
}

void MIPS::execute_mtlo(const Instruction& instruction) {
	lo = r[instruction.rs];
	end();
}

void MIPS::execute_mult(const Instruction& instruction) {
	int64_t t = static_cast<int64_t>(static_cast<int32_t>(r[instruction.rs])) * static_cast<int64_t>(static_cast<int32_t>(r[instruction.rt]));
	lo = t;
	hi = t >> 32;
	end();
}

void MIPS::execute_multu(const Instruction& instruction) {
	uint64_t t = static_cast<uint64_t>(r[instruction.rs]) * static_cast<uint64_t>(r[instruction.rt]);
	lo = t;
	hi = t >> 32;
	end();
}

void MIPS::execute_nor(const Instruction& instruction) {
	write_register(instruction.rd, ~(r[instruction.rs] | r[instruction.rt]));
	end();
}

void MIPS::execute_or(const Instruction& instruction) {
	write_register(instruction.rd, r[instruction.rs] | r[instruction.rt]);
	end();
}

void MIPS::execute_ori(const Instruction& instruction) {
	write_register(instruction.rt, r[instruction.rs] | instruction.imm16);
	end();
}

// FOR LATER
void MIPS::execute_sb(const Instruction& instruction) {
	uint32_t effective_address = (int)r[instruction.rs] + (int)sign_extend(instruction.imm16);
	write8(effective_address, r[instruction.rt] & 0xFF);
	end();
}

void MIPS::execute_sh(const Instruction& instruction) {
	uint32_t effective_address = (int)r[instruction.rs] + (int)sign_extend(instruction.imm16);
	if (effective_address & 1) {
		cop0.set_BadA(effective_address); // MAY BE AN INCORRECT BAD ADDRESS SET
		cop0.raise_AdES_exception();
		end();
		return;
	}
	write16(effective_address, r[instruction.rt] & 0xFFFF);
	end();
}

void MIPS::execute_sll(const Instruction& instruction) {
	write_register(instruction.rd, r[instruction.rt] << instruction.imm5);
	end();
}

void MIPS::execute_sllv(const Instruction& instruction) {
	write_register(instruction.rd, r[instruction.rt] << (r[instruction.rs] & 0x1F));
	end();
}

void MIPS::execute_slt(const Instruction& instruction) {
	if (static_cast<int32_t>(r[instruction.rs]) < static_cast<int32_t>(r[instruction.rt])) {
		write_register(instruction.rd, 1u);
	} else {
		write_register(instruction.rd, 0u);
	}
	end();
}

void MIPS::execute_slti(const Instruction& instruction) {
	if (static_cast<int32_t>(r[instruction.rs]) < sign_extend(instruction.imm16)) {
		write_register(instruction.rt, 1u);
	} else {
		write_register(instruction.rt, 0u);
	}
	end();
}

void MIPS::execute_sltiu(const Instruction& instruction) {
	if (static_cast<uint32_t>(r[instruction.rs]) < static_cast<uint32_t>(sign_extend(instruction.imm16))) {
		write_register(instruction.rt, 1u);
	} else {
		write_register(instruction.rt, 0u);
	}
	end();
}

void MIPS::execute_sltu(const Instruction& instruction) {
	if (static_cast<uint32_t>(r[instruction.rs]) < static_cast<uint32_t>(r[instruction.rt])) {
		write_register(instruction.rd, 1u);
	} else {
		write_register(instruction.rd, 0u);
	}
	end();
}

void MIPS::execute_sra(const Instruction& instruction) {
	write_register(instruction.rd, (int32_t)((int64_t)((int32_t)r[instruction.rt]) >> instruction.imm5));
	end();
}

void MIPS::execute_srav(const Instruction& instruction) {
	write_register(instruction.rd, (int32_t)((int64_t)((int32_t)r[instruction.rt]) >> (r[instruction.rs] & 0x1F)));
	end();
}

void MIPS::execute_srl(const Instruction& instruction) {
	write_register(instruction.rd, r[instruction.rt] >> instruction.imm5);
	end();
}

void MIPS::execute_srlv(const Instruction& instruction) {
	write_register(instruction.rd, r[instruction.rt] >> (r[instruction.rs] & 0x1F));
	end();
}

void MIPS::execute_sub(const Instruction& instruction) {
	if (sub_overflows(r[instruction.rs], r[instruction.rt])) {
		cop0.raise_Ovf_exception();
	} else {
		write_register(instruction.rd, r[instruction.rs] - r[instruction.rt]);
	}
	end();
}

void MIPS::execute_subu(const Instruction& instruction) {
	write_register(instruction.rd, r[instruction.rs] - r[instruction.rt]);
	end();
}

void MIPS::execute_sw(const Instruction& instruction) {
	uint32_t effective_address = (int)r[instruction.rs] + (int)sign_extend(instruction.imm16);
	if (effective_address & 3) {
		cop0.set_BadA(effective_address); // MAY BE AN INCORRECT BAD ADDRESS SET
		cop0.raise_AdES_exception();
		end();
		return;
	}
	write32(effective_address, r[instruction.rt]);
	end();
}

// TO DO
void MIPS::execute_swc0(const Instruction& instruction) { return; }
void MIPS::execute_swc1(const Instruction& instruction) { return; }
void MIPS::execute_swc2(const Instruction& instruction) { return; }
void MIPS::execute_swc3(const Instruction& instruction) { return; }


void MIPS::execute_swl(const Instruction& instruction) {
	uint32_t src = r[instruction.rt];
	effective_address = (int)r[instruction.rs] + (int)sign_extend(instruction.imm16);
		
	switch (effective_address & 3) {
	case 0: // 1-byte read
		write8(effective_address, (src & 0xFF000000) >> 24);
		break;
	case 1: // 2-byte read
		write16(effective_address - 1, (src & 0xFFFF0000) >> 16);
		break;
	case 2: // 3-byte read
		write16(effective_address - 2, (src & 0x00FFFF00) >> 8);
		write8(effective_address, (src & 0xFF000000) >> 24);
		break;
	case 3: // 4-byte read
		write32(effective_address - 3, src);
		break;
	default:
		break;
	}
	end();
}

void MIPS::execute_swr(const Instruction& instruction) {
	uint32_t src = r[instruction.rt];
	effective_address = (int)r[instruction.rs] + (int)sign_extend(instruction.imm16);
	
	switch (effective_address & 3) {
	case 3:
		write8(effective_address, src & 0xFF);
		break;
	case 2:
		write16(effective_address, src & 0xFFFF);
		break;
	case 1:
		write16(effective_address, src & 0xFFFF);
		write8(effective_address + 2, (src & 0x00FF0000) >> 16);
		break;
	case 0:
		write32(effective_address, src);
		break;
	default:
		break;
	}
	end();
}

void MIPS::execute_syscall(const Instruction& instruction) {
	cop0.raise_Sys_exception();
	end();
}

void MIPS::do_lwc(const Instruction& instruction, int cop) {
	switch(cop) {
	case 0:
	case 1:
	case 3:
		cop0.raise_CpU_exception();
		break;
	case 2:
		effective_address = (int)r[instruction.rs] + (int)sign_extend(instruction.imm16);
		if (effective_address & 3) {
			cop0.set_BadA(effective_address); // MAY BE AN INCORRECT BAD ADDRESS SET
			cop0.raise_AdEL_exception();
		} else {
			cop2.write(instruction.rt, read32(effective_address));
		}
		break;
	}
}

void MIPS::execute_lwc0(const Instruction& instruction) {
	do_lwc(instruction, 0);
	end();
}

void MIPS::execute_lwc1(const Instruction& instruction) { 
	do_lwc(instruction, 1);
	end();
}

void MIPS::execute_lwc2(const Instruction& instruction) {
	do_lwc(instruction, 2);
	end();
}

void MIPS::execute_lwc3(const Instruction& instruction) {
	do_lwc(instruction, 3);
	end();
}

void MIPS::execute_xor(const Instruction& instruction) { 
	write_register(instruction.rd, r[instruction.rs] ^ r[instruction.rt]);
	end();
}

void MIPS::execute_xori(const Instruction& instruction) {
	write_register(instruction.rt, r[instruction.rs] ^ instruction.imm16);
	end();
}
