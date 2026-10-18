#include "cop0.hpp"
#include "mips.hpp"

uint32_t COP0::read(uint32_t reg) {
	return r[reg & 63];
}

void COP0::write(uint32_t reg, uint32_t value) {
	switch (reg) {
		case 8:
		case 15: return;
		case 13: CAUSE = (CAUSE & ~0x300u) | (value & 0x300u); return;
		case 12: SR = value; mips->set_isolate_cache(Isc()); return;
		default: r[reg & 63] = value;
	}
}