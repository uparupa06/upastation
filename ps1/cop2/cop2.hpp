#pragma once
#include <cstdint>
#include "utility.hpp"

class MIPS;

class COP2 {
public:

	COP2() {
		initialise();
	}

	// cop2.cpp
	void initialise();

	// cop2_rw.cpp
	uint32_t read(uint32_t r);
	void write(uint32_t r, uint32_t value);
	
	// cop2_func.cpp
	void do_cofunc(uint32_t cofunc);
};