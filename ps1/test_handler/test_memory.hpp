#pragma once
#include <cstdint>
#include <unordered_map>
#include <iostream>
#include <vector>
#include "utility.hpp"

class TestMemory {
public:
	TestMemory() { }

	uint8_t   read8(uint32_t addr, bool log = true) {
		auto it = data.find(addr);
		uint32_t value = it == data.end() ? 0 : it->second;
		if (log) {
			log_cycle(READ, 1, addr, read32(addr, false));
		}
		return value;
	}

	uint16_t read16(uint32_t addr) {
		uint32_t value = uint16_t(read8(addr, false))
		               | uint16_t(read8(addr + 1, false)) << 8;
		log_cycle(READ, 2, addr, read32(addr, false));
		
		return value;
	}

	uint32_t read32(uint32_t addr, bool log = true) {
		uint32_t value = (uint32_t(read8(addr, false)))
		                 | (uint32_t(read8(addr + 1, false)) << 8)
		                 | (uint32_t(read8(addr + 2, false)) << 16)
		                 | (uint32_t(read8(addr + 3, false)) << 24);
		if (log) {
			log_cycle(READ, 4, addr, value);
		}
		return value;
	}

	void  write8(uint32_t addr, uint8_t value, bool log = true, bool override_log_value = false, uint32_t log_value = 0x0) {
		if (value == 0) {
			data.erase(addr);
		} else {
			data[addr] = value;
		}

		if (log) {
			log_cycle(WRITE, 1, addr, override_log_value ? log_value : (uint32_t)value);
		}
	}

	void write16(uint32_t addr, uint16_t value, bool override_log_value = false, uint32_t log_value = 0x0) {
		write8(addr, value & 0xFF, false);
		write8(addr + 1, value >> 8, false);

		log_cycle(WRITE, 2, addr, override_log_value ? log_value : (uint32_t)value);
	}

	void write32(uint32_t addr, uint32_t value) {
		write8(addr, value & 0xFF, false);
		write8(addr + 1, (value >> 8)  & 0xFF, false);
		write8(addr + 2, (value >> 16) & 0xFF, false);
		write8(addr + 3, (value >> 24) & 0xFF, false);

		log_cycle(WRITE, 4, addr, value);
	}

	uint32_t fetch(uint32_t addr) {
		uint32_t value = read32(addr, false);
		log_cycle(FETCH, 4, addr, value);
		return value;
	}

	void clear() {
		data.clear();
	}

	std::vector<Cycle> get_cycles() {
		return cycles;
	}

	void log_cycle(int actions, int sz, uint32_t addr, uint32_t val) {
		Cycle c { actions, sz, addr, val };
		cycles.push_back(c);
	}

	void clear_cycles(){
		cycles.clear();
	}

private:
	std::unordered_map<uint32_t, uint8_t> data;
	std::vector<Cycle> cycles;
};