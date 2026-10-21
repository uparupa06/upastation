#include "utility.hpp"

bool add_overflows(uint32_t a, uint32_t b) {
	uint32_t s = a + b;
	return ((a ^ s) & (b ^ s)) >> 31;
}

bool sub_overflows(uint32_t a, uint32_t b) {
	uint32_t d = a - b;
	return ((a ^ b) & (a ^ d)) >> 31;
}

void clear_bit(uint32_t& value, uint8_t bit) {
	value &= ~(1u << bit);
}

void set_bit(uint32_t& value, uint8_t bit) {
	value |= (1u << bit);
}

void log_32(uint32_t value) {
	std::cout << std::hex << std::setw(8) << std::setfill('0') << (int)value;
}

int16_t sign_extend_11(uint32_t value) {
	value &= 0x7FF;

	if (value & 0x400) {
		value |= 0xF800;
	}

	return static_cast<int16_t>(value);
}