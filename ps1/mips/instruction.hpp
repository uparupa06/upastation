#pragma once
#include <cstdint>
#include <array>
#include <iostream>
#include "utility.hpp"

struct Instruction;
class MIPS;

// Instruction definition struct

struct InstructionDefinition {
	const char* name;
	void (MIPS::*execute)(const Instruction&);
};

// All instruction definitions

// Primary opcode field

extern InstructionDefinition BcondZ;
extern InstructionDefinition J;
extern InstructionDefinition JAL;
extern InstructionDefinition BEQ;
extern InstructionDefinition BNE;
extern InstructionDefinition BLEZ;
extern InstructionDefinition BGTZ;
extern InstructionDefinition ADDI;
extern InstructionDefinition ADDIU;
extern InstructionDefinition SLTI;
extern InstructionDefinition SLTIU;
extern InstructionDefinition ANDI;
extern InstructionDefinition ORI;
extern InstructionDefinition XORI;
extern InstructionDefinition LUI;
extern InstructionDefinition iCOP0;
extern InstructionDefinition iCOP1;
extern InstructionDefinition iCOP2;
extern InstructionDefinition iCOP3;
extern InstructionDefinition LB;
extern InstructionDefinition LH;
extern InstructionDefinition LWL;
extern InstructionDefinition LW;
extern InstructionDefinition LBU;
extern InstructionDefinition LHU;
extern InstructionDefinition LWR;
extern InstructionDefinition SB;
extern InstructionDefinition SH;
extern InstructionDefinition SWL;
extern InstructionDefinition SW;
extern InstructionDefinition SWR;
extern InstructionDefinition LWC0;
extern InstructionDefinition LWC1;
extern InstructionDefinition LWC2;
extern InstructionDefinition LWC3;
extern InstructionDefinition SWC0;
extern InstructionDefinition SWC1;
extern InstructionDefinition SWC2;
extern InstructionDefinition SWC3;

// Primary opcode table

static const std::array<InstructionDefinition*, 64> primary_opcode_table = {
	nullptr,
	&BcondZ,
	&J,
	&JAL,
	&BEQ,
	&BNE,
	&BLEZ,
	&BGTZ,
	&ADDI,
	&ADDIU,
	&SLTI,
	&SLTIU,
	&ANDI,
	&ORI,
	&XORI,
	&LUI,
	&iCOP0,
	&iCOP1,
	&iCOP2,
	&iCOP3,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	&LB,
	&LH,
	&LWL,
	&LW,
	&LBU,
	&LHU,
	&LWR,
	nullptr,
	&SB,
	&SH,
	&SWL,
	&SW,
	nullptr,
	nullptr,
	&SWR,
	nullptr,
	&LWC0,
	&LWC1,
	&LWC2,
	&LWC3,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	&SWC0,
	&SWC1,
	&SWC2,
	&SWC3,
	nullptr,
	nullptr,
	nullptr,
	nullptr
};

// Secondary opcode field (when primary opcode is equal to 00)

extern InstructionDefinition SLL;
extern InstructionDefinition SRL;
extern InstructionDefinition SRA;
extern InstructionDefinition SLLV;
extern InstructionDefinition SRLV;
extern InstructionDefinition SRAV;
extern InstructionDefinition JR;
extern InstructionDefinition JALR;
extern InstructionDefinition SYSCALL;
extern InstructionDefinition BREAK;
extern InstructionDefinition MFHI;
extern InstructionDefinition MTHI;
extern InstructionDefinition MFLO;
extern InstructionDefinition MTLO;
extern InstructionDefinition MULT;
extern InstructionDefinition MULTU;
extern InstructionDefinition DIV;
extern InstructionDefinition DIVU;
extern InstructionDefinition ADD;
extern InstructionDefinition ADDU;
extern InstructionDefinition SUB;
extern InstructionDefinition SUBU;
extern InstructionDefinition AND;
extern InstructionDefinition OR;
extern InstructionDefinition XOR;
extern InstructionDefinition NOR;
extern InstructionDefinition SLT;
extern InstructionDefinition SLTU;

// Secondary opcode table
static const std::array<InstructionDefinition*, 64> secondary_opcode_table = {
	&SLL,
	nullptr,
	&SRL,
	&SRA,
	&SLLV,
	nullptr,
	&SRLV,
	&SRAV,
	&JR,
	&JALR,
	nullptr,
	nullptr,
	&SYSCALL,
	&BREAK,
	nullptr,
	nullptr,
	&MFHI,
	&MTHI,
	&MFLO,
	&MTLO,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	&MULT,
	&MULTU,
	&DIV,
	&DIVU,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	&ADD,
	&ADDU,
	&SUB,
	&SUBU,
	&AND,
	&OR,
	&XOR,
	&NOR,
	nullptr,
	nullptr,
	&SLT,
	&SLTU,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
};

// Instruction struct

struct Instruction {
	uint32_t raw; // Instruction itself, not split into the different fields
	
	const InstructionDefinition* definition;

	// Fields for each instruction
	uint8_t  primary;   // bits 26-31
	uint8_t  rs;        // bits 21-25
	uint8_t  rt;        // bits 16-20
	uint8_t  rd;        // bits 11-15
	uint8_t  imm5;      // bits 6-10
	uint8_t  secondary; // bits 0-5
	
	uint16_t imm16;     // bits 0-15
	uint32_t imm26;     // bits 0-25
	uint32_t imm25;     // bits 0-24

	uint32_t comm20;    // bits 6-25

	// Setting any instruction will cause automatic decoding
	Instruction& operator=(uint32_t instruction) {
		raw = instruction;

		primary = extract_bits(instruction, 26, 31);
		rs = extract_bits(instruction, 21, 25);
		rt = extract_bits(instruction, 16, 20);
		rd = extract_bits(instruction, 11, 15);
		imm5 = extract_bits(instruction, 6, 10);
		secondary = extract_bits(instruction, 0, 5);

		imm16 = extract_bits(instruction, 0, 15);
		imm26 = extract_bits(instruction, 0, 25);
		imm25 = extract_bits(instruction, 0, 24);

		comm20 = extract_bits(instruction, 6, 25);
		
		definition = decode();

		if constexpr (!TESTING_CPU) {
			if (definition) {
				//std::cout << definition->name << "\n";
			} else {
				//std::cout << "Invalid\n";
			}
		}

		return *this;
	}

	Instruction() = default;
	Instruction(uint32_t instruction) {
		*this = instruction;
	}

private:

	InstructionDefinition* decode() {
		if (primary) {
			return primary_opcode_table[primary];
		} else {
			return secondary_opcode_table[secondary];
		}
	}
};