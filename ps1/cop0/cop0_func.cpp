#include "cop0.hpp"

void COP0::do_cofunc(uint32_t cofunc) {
	if ((cofunc & 0x3F) == 0x10) {
		SR = (SR & ~0xFu) | ((SR >> 2) & 0xFu);
	}
}