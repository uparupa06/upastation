#include "mips.hpp"
#include "utility.hpp"

// REVIEW WHAT I AM DOING FOR EXCEPTIONS, NEED TO MAKE IT SO THAT C++ DOES NOT TERMINATE THE PROGRAM

void MIPS::check_memory(uint32_t addr, bool is_store, bool is_fetch) {
	try {
		switch (memory.get_exception()) {
			case MemoryException: throw CpuException{ is_store ? 5u : 4u, addr, true };
			case BusException:    throw CpuException{ is_fetch ? 6u : 7u, addr, true };
			default: return;
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
}

uint8_t MIPS::read8(uint32_t addr) {
	if constexpr (TESTING_CPU) {
		return test_memory.read8(addr);
	} else {
		memory.reset_rw_details();
		uint8_t value = memory.read8(addr);
		check_memory(addr, false, false);
		return value;
	}
}

uint16_t MIPS::read16(uint32_t addr) {
	if constexpr (TESTING_CPU) {
		return test_memory.read16(addr);
	} else {
		memory.reset_rw_details();
		uint16_t value = memory.read16(addr);
		check_memory(addr, false, false);
		return value;
	}
}

uint32_t MIPS::read32(uint32_t addr) {
	if constexpr (TESTING_CPU) {
		return test_memory.read32(addr);
	} else {
		memory.reset_rw_details();
		uint32_t value = memory.read32(addr);
		check_memory(addr, false, false);
		return value;
	}
}

void MIPS::write8(uint32_t addr, uint8_t value) {
	if constexpr (TESTING_CPU) {
		test_memory.write8(addr, value);
	} else {
		memory.reset_rw_details();
		memory.write8(addr, value);
		check_memory(addr, true, false);
	}
}

void MIPS::write16(uint32_t addr, uint16_t value) {
	if constexpr (TESTING_CPU) {
		test_memory.write16(addr, value);
	} else {
		memory.reset_rw_details();
		memory.write16(addr, value);
		check_memory(addr, true, false);
	}
}

void MIPS::write32(uint32_t addr, uint32_t value) {
	if constexpr (TESTING_CPU) {
		test_memory.write32(addr, value);
	} else {
		memory.reset_rw_details();
		memory.write32(addr, value);
		check_memory(addr, true, false);
	}
}

uint32_t MIPS::fetch() {
	if constexpr (TESTING_CPU) {
		return test_memory.fetch(pipeline());
	} else {
		uint32_t addr = pipeline();
		memory.reset_rw_details();
		uint32_t value = memory.read32(addr);
		check_memory(addr, false, true);
		return value;
	}
}

void MIPS::set_isolate_cache(bool on) {
	memory.set_isolate_cache(on);
}