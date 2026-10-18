#include "cop0.hpp"

void COP0::connect_mips(MIPS* mips) { 
	this->mips = mips;
}

void COP0::initialise() {
	for (auto& _r : r) {
		_r = 0;
	}
	SR   = 0x10900000;
	PRID = 0x00000002;
}