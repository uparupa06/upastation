#include "instruction.hpp"
#include "mips.hpp"



// STILL NEED TO DO...

// 01 BcondZ
InstructionDefinition BcondZ {
	"BcondZ",
	&MIPS::execute_bcondz
};

// 02 J
InstructionDefinition J {
	"J",
	&MIPS::execute_j
};

// 03 JAL
InstructionDefinition JAL {
	"JAL",
	&MIPS::execute_jal
};

// 04 BEQ
InstructionDefinition BEQ {
	"BEQ",
	&MIPS::execute_beq
};

// 05 BNE
InstructionDefinition BNE {
	"BNE",
	&MIPS::execute_bne
};

// 06 BLEZ
InstructionDefinition BLEZ {
	"BLEZ",
	&MIPS::execute_blez
};

// 07 BGTZ
InstructionDefinition BGTZ {
	"BGTZ",
	&MIPS::execute_bgtz
};

// 08 ADDI
InstructionDefinition ADDI {
	"ADDI",
	&MIPS::execute_addi
};

// 09 ADDIU
InstructionDefinition ADDIU {
	"ADDIU",
	&MIPS::execute_addiu
};

// 0A SLTI
InstructionDefinition SLTI {
	"SLTI",
	&MIPS::execute_slti
};

// 0B SLTIU
InstructionDefinition SLTIU {
	"SLTIU",
	&MIPS::execute_sltiu
};

// 0C ANDI
InstructionDefinition ANDI {
	"ANDI",
	&MIPS::execute_andi
};

// 0D ORI
InstructionDefinition ORI {
	"ORI",
	&MIPS::execute_ori
};

// 0E XORI
InstructionDefinition XORI {
	"XORI",
	&MIPS::execute_xori
};

// 0F LUI
InstructionDefinition LUI {
	"LUI",
	&MIPS::execute_lui
};

// 10 COP0
InstructionDefinition iCOP0 {
	"COP0",
	&MIPS::execute_cop0
};

// 11 COP1
InstructionDefinition iCOP1 {
	"COP1",
	&MIPS::execute_cop1
};

// 12 COP2
InstructionDefinition iCOP2 {
	"COP2",
	&MIPS::execute_cop2
};

// 13 COP3
InstructionDefinition iCOP3 {
	"COP3",
	&MIPS::execute_cop3
};

// 20 LB
InstructionDefinition LB {
	"LB",
	&MIPS::execute_lb
};

// 21 LH
InstructionDefinition LH {
	"LH",
	&MIPS::execute_lh
};

// 22 LWL
InstructionDefinition LWL {
	"LWL",
	&MIPS::execute_lwl
};

// 23 LW
InstructionDefinition LW {
	"LW",
	&MIPS::execute_lw
};

// 24 LBU
InstructionDefinition LBU {
	"LBU",
	&MIPS::execute_lbu
};

// 25 LHU
InstructionDefinition LHU {
	"LHU",
	&MIPS::execute_lhu
};

// 26 LWR
InstructionDefinition LWR {
	"LWR",
	&MIPS::execute_lwr
};

// 28 SB
InstructionDefinition SB {
	"SB",
	&MIPS::execute_sb
};

// 29 SH
InstructionDefinition SH {
	"SH",
	&MIPS::execute_sh
};

// 2A SWL
InstructionDefinition SWL {
	"SWL",
	&MIPS::execute_swl
};

// 2B SW
InstructionDefinition SW {
	"SW",
	&MIPS::execute_sw
};

// 2E SWR
InstructionDefinition SWR {
	"SWR",
	&MIPS::execute_swr
};

// 30 LWC0
InstructionDefinition LWC0 {
	"LWC0",
	&MIPS::execute_lwc0
};

// 31 LWC1
InstructionDefinition LWC1 {
	"LWC1",
	&MIPS::execute_lwc1
};

// 32 LWC2
InstructionDefinition LWC2 {
	"LWC2",
	&MIPS::execute_lwc2
};

// 33 LWC3
InstructionDefinition LWC3 {
	"LWC3",
	&MIPS::execute_lwc3
};

// 38 SWC0
InstructionDefinition SWC0 {
	"SWC0",
	&MIPS::execute_swc0
};

// 39 SWC1
InstructionDefinition SWC1 {
	"SWC1",
	&MIPS::execute_swc1
};

// 3A SWC2
InstructionDefinition SWC2 {
	"SWC2",
	&MIPS::execute_swc2
};

// 3B SWC3
InstructionDefinition SWC3 {
	"SWC3",
	&MIPS::execute_swc3
};

// Secondary opcode field (when primary opcode is equal to 00)

// 00 SLL
InstructionDefinition SLL {
	"SLL",
	&MIPS::execute_sll
};

// 02 SRL
InstructionDefinition SRL {
	"SRL",
	&MIPS::execute_srl
};

// 03 SRA
InstructionDefinition SRA {
	"SRA",
	&MIPS::execute_sra
};

// 04 SLLV
InstructionDefinition SLLV {
	"SLLV",
	&MIPS::execute_sllv
};

// 06 SRLV
InstructionDefinition SRLV {
	"SRLV",
	&MIPS::execute_srlv
};

// 07 SRAV
InstructionDefinition SRAV {
	"SRAV",
	&MIPS::execute_srav
};

// 08 JR
InstructionDefinition JR {
	"JR",
	&MIPS::execute_jr
};

// 09 JALR
InstructionDefinition JALR {
	"JALR",
	&MIPS::execute_jalr
};

// 0C SYSCALL
InstructionDefinition SYSCALL {
	"SYSCALL",
	&MIPS::execute_syscall
};

// 0D BREAK
InstructionDefinition BREAK {
	"BREAK",
	&MIPS::execute_break
};

// 10 MFHI
InstructionDefinition MFHI {
	"MFHI",
	&MIPS::execute_mfhi
};

// 11 MTHI
InstructionDefinition MTHI {
	"MTHI",
	&MIPS::execute_mthi
};

// 12 MFLO
InstructionDefinition MFLO {
	"MFLO",
	&MIPS::execute_mflo
};

// 13 MTLO
InstructionDefinition MTLO {
	"MTLO",
	&MIPS::execute_mtlo
};

// 18 MULT
InstructionDefinition MULT {
	"MULT",
	&MIPS::execute_mult
};

// 19 MULTU
InstructionDefinition MULTU {
	"MULTU",
	&MIPS::execute_multu
};

// 1A DIV
InstructionDefinition DIV {
	"DIV",
	&MIPS::execute_div
};

// 1B DIVU
InstructionDefinition DIVU {
	"DIVU",
	&MIPS::execute_divu
};

// 20 ADD
InstructionDefinition ADD {
	"ADD",
	&MIPS::execute_add
};

// 21 ADDU
InstructionDefinition ADDU {
	"ADDU",
	&MIPS::execute_addu
};

// 22 SUB
InstructionDefinition SUB {
	"SUB",
	&MIPS::execute_sub
};

// 23 SUBU
InstructionDefinition SUBU {
	"SUBU",
	&MIPS::execute_subu
};

// 24 AND
InstructionDefinition AND {
	"AND",
	&MIPS::execute_and
};

// 25 OR
InstructionDefinition OR {
	"OR",
	&MIPS::execute_or
};

// 26 XOR
InstructionDefinition XOR {
	"XOR",
	&MIPS::execute_xor
};

// 27 NOR
InstructionDefinition NOR {
	"NOR",
	&MIPS::execute_nor
};

// 2A SLT
InstructionDefinition SLT {
	"SLT",
	&MIPS::execute_slt
};

// 2B SLTU
InstructionDefinition SLTU {
	"SLTU",
	&MIPS::execute_sltu
};