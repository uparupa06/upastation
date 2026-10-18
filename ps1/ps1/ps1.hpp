#pragma once
#include "test_handler.hpp"
#include "utility.hpp"
#include "mips.hpp"

class PS1 {
public:
	PS1() { }

	// ps1_scheduling.cpp
	void run();
	void do_tick();

private:
	
	MIPS mips;
};