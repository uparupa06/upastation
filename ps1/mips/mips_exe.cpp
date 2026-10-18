#include "mips.hpp"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <cstdint>

uint64_t MIPS::read_from_exe(uint32_t start, uint32_t end) {
	if (start > end) {
		return read_from_exe(end, start);
	}
	uint64_t value;
	for (int i = end; i >= start; i--) {
		value <<= 8;
		value |= exe[i];
	}
	return value;
}

void MIPS::exe_sideload(const std::string& exe_file) {
	std::string path = "ps1/exe/" + exe_file;
	std::cout << "EXE path: " << path << "\n";

	std::filesystem::path p { path };

	std::ifstream file(p, std::ios::binary);

	if (!file) {
		std::cerr << "The EXE file " << exe_file << " does not exist.\n";
		std::cerr << "Please place the relevant EXE file in ps1/exe.\n";
		std::exit(EXIT_FAILURE);
	}

	exe.resize(std::filesystem::file_size(p));

	file.read(
		reinterpret_cast<char*>(exe.data()),
		std::filesystem::file_size(p)
	);

	std::cout << "EXE file " << exe_file << " of size " << exe.size() << " bytes loaded.\n";

	uint32_t initial_pc   = read_from_exe(0x10, 0x13);
	uint32_t initial_r28  = read_from_exe(0x14, 0x17);
	uint32_t copy_address = read_from_exe(0x18, 0x1B);
	uint32_t exe_size     = read_from_exe(0x1C, 0x1F);
	uint32_t initial_sp   = read_from_exe(0x30, 0x33);

	for (int i = 0; i < exe_size; i++) {
		write8(copy_address, exe[i + 2048]); // GOT SOME ISSUES OCCURRING HERE! ADD MAGIC NUMBER 0x1f801814 to 0x5E800000
		copy_address++;
	}
	
	write_register(28, initial_r28);
	if (initial_sp != 0) {
		write_register(29, initial_sp);
		write_register(30, initial_sp);
	}

	pc_0 = initial_pc;
	pc_1 = initial_pc + 4;

	return;
}
