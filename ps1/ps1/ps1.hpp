#pragma once
#include "test_handler.hpp"
#include "utility.hpp"
#include "mips.hpp"
#include "dma.hpp"

class PS1 {
public:
	PS1() { }

	// ps1_scheduling.cpp
	void initialise();
	void run();
	void do_tick();

private:
	DMA* dma = nullptr;
	MIPS mips;
};