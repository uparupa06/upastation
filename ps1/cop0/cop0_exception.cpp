#include "cop0.hpp"
#include "mips.hpp"

void COP0::handle_exception() {
	SR = (SR & ~0x3F) | ((SR << 2) & 0x3F);

	CAUSE &= ~(3u << 28);

	if (mips->in_delay_slot) {
		EPC = mips->current_pc - 4;
		CAUSE |= (1u << 31);
		TAR = mips->branch_target_addr();
		CAUSE = mips->branch_taken() ? (CAUSE | (1u << 30)) : (CAUSE & ~(1u << 30));
	} else {
		EPC = mips->current_pc;
		CAUSE &= ~((1u << 31) | (1u << 30));
	}

	mips->jump_to_exception(general_exception_vector());
}

uint32_t COP0::reset_exception_vector() {
	return 0xBFC00000;
}

uint32_t COP0::utlb_miss_exception_vector() {
	return BEV() ? 0xBFC00100 : 0x80000000;
}

uint32_t COP0::cop0_break_exception_vector() {
	return BEV() ? 0xBFC00140 : 0x80000040;
}

uint32_t COP0::general_exception_vector() {
	return BEV() ? 0xBFC00180 : 0x80000080;
}

