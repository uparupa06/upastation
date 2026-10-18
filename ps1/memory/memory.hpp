#pragma once
#include <cstdint>
#include <vector>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <filesystem>
#include "utility.hpp"
#include "dma.hpp"
#include "gpu.hpp"

/*
TO-DO:
DMA CHANNELS, GET DMA2 AND DMA6 WORKING!!!
*/

constexpr uint32_t MAIN_RAM_SIZE       = 0x00200000;
constexpr uint32_t DEV0_EXPANSION_SIZE = 0x00800000;
constexpr uint32_t SCRATCHPAD_SIZE     = 0x00000400;
constexpr uint32_t IO_PORTS_SIZE       = 0x00001000;
constexpr uint32_t DEV8_EXPANSION_SIZE = 0x00002000;
constexpr uint32_t DEV1_EXPANSION_SIZE = 0x00200000;
constexpr uint32_t BIOS_SIZE           = 0x00080000;
constexpr uint32_t BIOS_WINDOW_SIZE    = 0x00400000;
constexpr uint32_t CACHE_CONTROL_SIZE   = 0x00000200;

constexpr uint32_t MAIN_RAM_BASE       = 0x00000000;
constexpr uint32_t DEV0_EXPANSION_BASE = 0x1F000000;
constexpr uint32_t SCRATCHPAD_BASE     = 0x1F800000;
constexpr uint32_t IO_PORTS_BASE       = 0x1F801000;
constexpr uint32_t DEV8_EXPANSION_BASE = 0x1F802000;
constexpr uint32_t DEV1_EXPANSION_BASE = 0x1FA00000;
constexpr uint32_t BIOS_BASE           = 0x1FC00000;
constexpr uint32_t CACHE_CONTROL_BASE  = 0xFFFE0000;

// IO PORTS
constexpr uint32_t GP0_ADDRESS = 0x1F801810;
constexpr uint32_t GP1_ADDRESS = 0x1F801814;

enum MemoryRegion : uint8_t {
	KSEG0 = 0,
	KSEG1 = 1,
	KSEG2 = 2,
	KUSEG = 3
};

enum RWException : uint8_t {
	NoException     = 0,
	MemoryException = 1,
	BusException    = 2
};

#define R_PAUSE_IF_EXCEPTION \
	if (memory_exception) { \
		return 0x00; \
	}

#define W_PAUSE_IF_EXCEPTION \
	if (memory_exception) { \
		return; \
	}

static bool in_range(uint32_t physical_address, uint32_t base, uint32_t size) {
	return (physical_address - base) < size;
}



class Memory {
public:
	Memory() 
	  : main_ram (MAIN_RAM_SIZE),
		dev0_expansion (DEV0_EXPANSION_SIZE),
		scratchpad (SCRATCHPAD_SIZE),
		io_ports (IO_PORTS_SIZE),
		dev8_expansion (DEV8_EXPANSION_SIZE),
		dev1_expansion (DEV1_EXPANSION_SIZE),
		bios (BIOS_SIZE),
		cpu_control (CACHE_CONTROL_SIZE)
	{
		load_bios();
		dma.initialise();
		gpu.initialise();
	}

	MemoryRegion get_region(uint32_t addr) {
		MemoryRegion region = KSEG2;
		switch (addr >> 29) {
			case 0b000: region = KUSEG; break;
			case 0b100: region = KSEG0; break;
			case 0b101: region = KSEG1; break;
			case 0b110:
			case 0b111: region = KSEG2; break;
			default:    memory_exception = BusException; break;
		}

		if (memory_exception) {
			return KSEG2;
		}

		if (region == KSEG2 && (addr < 0xFFFE0000 || addr >= 0xFFFE0200)) {
			memory_exception = BusException;
			return KSEG2;
		}

		return region;
	}

	void set_isolate_cache(bool on) {
		isolate_cache = on;
	}

	bool ram_mirror()            { return true;  }
	bool bios_ram_mirror()       { return false; }
	bool dev0_expansion_exists() { return false; }
	bool dev1_expansion_exists() { return false; }
	bool dev8_expansion_exists() { return false; }

	uint8_t* map(uint32_t addr, bool is_write) {
		io_port_access = false;
		memory_region = get_region(addr);
		if (memory_exception) {
			return nullptr;
		}
		if (in_range(addr, CACHE_CONTROL_BASE, CACHE_CONTROL_SIZE)) {
			return &cpu_control[addr - CACHE_CONTROL_BASE];
		}
		uint32_t physical_address = addr & 0x1FFFFFFF;
		if (physical_address < 0x00200000 || (physical_address < 0x00800000 && ram_mirror())) {
			return &main_ram[physical_address & (MAIN_RAM_SIZE - 1)];
		}
		if (in_range(physical_address, DEV0_EXPANSION_BASE, DEV0_EXPANSION_SIZE)) {
			if (!dev0_expansion_exists()) {
				open_bus_sink = 0xFF;
				return &open_bus_sink;
			}
			return &dev0_expansion[physical_address - DEV0_EXPANSION_BASE];
		}
		if (in_range(physical_address, SCRATCHPAD_BASE, SCRATCHPAD_SIZE) && (memory_region == KUSEG || memory_region == KSEG0)) {
			return &scratchpad[physical_address - SCRATCHPAD_BASE];
		}
		if (in_range(physical_address, IO_PORTS_BASE, IO_PORTS_SIZE)) {
			if (physical_address >= 0x1F801080 && physical_address <= 0x1F8010EF && (physical_address & 0xF) == 0xC) {
				physical_address -= 4;
			}
			io_port_access = true;
			return &io_ports[physical_address - IO_PORTS_BASE];
		} else {
			io_port_access = false;
		}
		if (in_range(physical_address, DEV8_EXPANSION_BASE, DEV8_EXPANSION_SIZE)) {
			if (!dev8_expansion_exists()) {
				open_bus_sink = 0xFF;
				return &open_bus_sink;
			}
			return &dev8_expansion[physical_address - DEV8_EXPANSION_BASE];
		}
		if (in_range(physical_address, DEV1_EXPANSION_BASE, DEV1_EXPANSION_SIZE)) {
			if (!dev1_expansion_exists()) {
				open_bus_sink = 0xFF;
				return &open_bus_sink;
			}
			return &dev1_expansion[physical_address - DEV1_EXPANSION_BASE];
		}
		if (in_range(physical_address, BIOS_BASE, BIOS_WINDOW_SIZE)) {
			uint32_t offset = physical_address - BIOS_BASE;
			if (offset < BIOS_SIZE) {
				return is_write ? &rom_write_sink : &bios[offset];
			}
			if (bios_ram_mirror()) {
				return is_write ? &rom_write_sink : &bios[offset & (BIOS_SIZE - 1)];
			}
		}
		memory_exception = BusException;
		return nullptr;
	}

	uint8_t read8(uint32_t addr) {
		uint8_t* byte = map(addr, false);
		if (io_port_access) {
			return ((read_io(addr) >> (8 * (addr & 0x3))) & 0xFF);
		}
		if (byte) {
			return *byte;
		} else {
			memory_exception = BusException;
			return 0x00;
		}
	}

	uint16_t read16(uint32_t addr) {
		if (addr & 1) {
			memory_exception = MemoryException;
			return 0x00;
		}
		uint32_t value = uint16_t(read8(addr));
		R_PAUSE_IF_EXCEPTION
		value = value | uint16_t(read8(addr + 1) << 8);
		R_PAUSE_IF_EXCEPTION
		return value;
	}

	uint32_t read32(uint32_t addr) {
		if (addr & 3) {
			memory_exception = MemoryException;
			return 0x00;
		}
		switch(addr) {
			case GP0_ADDRESS:
			case GP1_ADDRESS:
				return read_gpu(addr == GP1_ADDRESS);
			default:
				break;
		}
		uint32_t value = uint32_t(read8(addr));
		R_PAUSE_IF_EXCEPTION
		value = value | (uint32_t(read8(addr + 1) << 8));
		R_PAUSE_IF_EXCEPTION
		value = value | (uint32_t(read8(addr + 2) << 16));
		R_PAUSE_IF_EXCEPTION
		value = value | (uint32_t(read8(addr + 3) << 24));
		R_PAUSE_IF_EXCEPTION
		return value;
	}

	void write8(uint32_t addr, uint8_t value) {
		uint8_t* byte = map(addr, true);
		if (isolate_cache && memory_region != KSEG2) {
			return;
		}
		if (io_port_access) {
			write_io(addr, value);
			return;
		} else {
			if (byte) {
				*byte = value;
			} else {
				memory_exception = BusException;
			}
		}
	}

	void write16(uint32_t addr, uint16_t value) {
		if (addr & 1) {
			memory_exception = MemoryException;
			return;
		}
		write8(addr, value & 0xFF);
		W_PAUSE_IF_EXCEPTION
		write8(addr + 1, value >> 8);
	}

	void write32(uint32_t addr, uint32_t value) {
		if (addr & 3) {
			memory_exception = MemoryException;
			return;
		}
		switch(addr) {
			case GP0_ADDRESS:
			case GP1_ADDRESS:
				write_gpu(addr == GP1_ADDRESS, value);
				return;
			default:
				break;
		}
		write8(addr, value & 0xFF);
		W_PAUSE_IF_EXCEPTION
		write8(addr + 1, (value >> 8)  & 0xFF);
		W_PAUSE_IF_EXCEPTION
		write8(addr + 2, (value >> 16) & 0xFF);
		W_PAUSE_IF_EXCEPTION
		write8(addr + 3, (value >> 24) & 0xFF);
	}

	uint32_t fetch(uint32_t addr) {
		if (addr & 3) {
			memory_exception = MemoryException;
			return 0x00;
		}
		uint32_t value = read32(addr);
		uint32_t physical_address = addr & 0x1FFFFFFF;
		if ((addr >> 29) >= 0b110 || in_range(physical_address, SCRATCHPAD_BASE, SCRATCHPAD_SIZE)) {
			memory_exception = BusException;
			return 0x00;
		}
		return value;
	}

	// memory_io.cpp
	void write_io(uint32_t addr, uint8_t value);
	uint32_t read_io(uint32_t addr);
	void write_gpu(bool is_gp1, uint32_t value);
	uint32_t read_gpu(bool is_gp1);

	void load_bios() {
		
		std::cout << "Loading PS1 BIOS...\n";
		std::ifstream file("ps1/bios/SCPH1001.BIN", std::ios::binary);

		if (!file) {
			std::cerr << "Missing PS1 BIOS.\n";
			std::cerr << "Please provide the PS1 BIOS, named SCPH1001.BIN, and provided inside the folder ps1/bios.\n";
			std::exit(EXIT_FAILURE);
		}

		file.read(
			reinterpret_cast<char*>(bios.data()),
			bios.size()
		);

		if (file.gcount() != BIOS_SIZE) {
			std::cerr << "The PS1 BIOS provided must be 512KB in size.\n";
			std::exit(EXIT_FAILURE);
		}

		std::cout << "Loaded PS1 BIOS.\n";

		/*int bytes = 0;
		for (uint8_t byte : bios) {
			std::cout << std::hex << std::setw(2) << std::setfill('0') << std::hex << (int)byte << " ";
			bytes++;
			if (bytes >= 16) {
				bytes = 0;
				std::cout << "\n";
			}
		}*/

		std::cout << "\n";
	}

	int get_penalty() {
		return cycle_penalty;
	}

	RWException get_exception() {
		return memory_exception;
	}

	MemoryRegion get_region() {
		return memory_region;
	}

	void reset_rw_details() {
		cycle_penalty = 0;
		memory_exception = NoException;
		memory_region = KSEG2;
	}

	bool running() {
		return !gpu.window_closed();
	}

	void sdl_loop() {
		gpu.sdl_loop();
	}

	void initialise() {
		gpu.initialise();
	}
	
private:

	int cycle_penalty = 0;
	RWException memory_exception = NoException;
	MemoryRegion memory_region = KSEG2;
	uint8_t rom_write_sink = 0;
	uint8_t open_bus_sink  = 0xFF;
	bool isolate_cache;

	// Assuming PS1 zero-initialises (will have to check this)
	std::vector<uint8_t> main_ram {}; 
	std::vector<uint8_t> dev0_expansion {};
	std::vector<uint8_t> scratchpad {};
	std::vector<uint8_t> io_ports {};
	std::vector<uint8_t> dev8_expansion {};
	std::vector<uint8_t> dev1_expansion {};
	std::vector<uint8_t> bios {};
	std::vector<uint8_t> cpu_control {};

	// FOR DETECTION OF REGION ACCESSED
	bool io_port_access;

	// IO
	uint32_t io_old_value, io_value_mask, io_new_value;

	// DMA and other components
	DMA dma;
	GPU gpu;

};


/*

constexpr uint32_t MAIN_RAM_SIZE       = 0x00200000;
constexpr uint32_t DEV0_EXPANSION_SIZE = 0x00800000;
constexpr uint32_t SCRATCHPAD_SIZE     = 0x00000400;
constexpr uint32_t IO_PORTS_SIZE       = 0x00001000;
constexpr uint32_t DEV8_EXPANSION_SIZE = 0x00002000;
constexpr uint32_t DEV1_EXPANSION_SIZE = 0x00200000;
constexpr uint32_t BIOS_SIZE           = 0x00080000;
constexpr uint32_t CPU_CONTROL_SIZE    = 0x00000200;

*/

/*
https://psx-spx.consoledev.net/ps1/system/memorymap/

Kernel memory:
KSEG1 -> normal physical memory, uncached
KSEG0 -> mirror of KSEG1, but cached
KSEG2 contains cache control hardware registers

User memory:
KUSEG -> mirror for first 512MB of KSEG1, will cause exception if accessing memory in remaining 1.5GB
KUSEG is also cached like KSEG0

Need way for KUSEG to cause the exception
*/