#include "utility.hpp"

bool add_overflows(uint32_t a, uint32_t b) {
	uint32_t s = a + b;
	return ((a ^ s) & (b ^ s)) >> 31;
}

bool sub_overflows(uint32_t a, uint32_t b) {
	uint32_t d = a - b;
	return ((a ^ b) & (a ^ d)) >> 31;
}