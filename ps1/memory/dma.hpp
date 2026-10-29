#pragma once
#include "utility.hpp"
#include "gpu.hpp"
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <vector>
#include <algorithm>

class Memory;

struct Channel {
	uint32_t addr = 0;
	uint32_t bcr  = 0;
	uint32_t chcr = 0;
	uint8_t priority = 0;
};

class DMA {
public:
	DMA() { }

	void initialise() {
		dma_tick = 0;
	}

	void set_dmaddr(uint32_t value, int chnum)  {
		channels[chnum].addr = value;
		if constexpr (SHOW_LOGS) {		
			std::cout << "DMA Channel " << (int)chnum << " ADDR WRITE ";
			log_32(channels[chnum].addr);
			std::cout << "\n";
		}
		do_dma(); 
	}

	void set_dmabcr(uint32_t value, int chnum) {
		channels[chnum].bcr = value;
		if constexpr (SHOW_LOGS) {
			std::cout << "DMA Channel " << (int)chnum << " BCR  WRITE ";
			log_32(channels[chnum].bcr);
			std::cout << "\n";
		}
		do_dma();
	}
	
	void set_dmachcr(uint32_t value, int chnum) {
		if (chnum == 6) {
			channels[chnum].chcr = value & ((1u << 24) | (1u << 28) | (1u << 30));
		} else {
			channels[chnum].chcr = value;
		}
		if constexpr (SHOW_LOGS) {
			std::cout << "DMA Channel " << (int)chnum << " CHCR WRITE ";
			log_32(channels[chnum].chcr);
			std::cout << "\n";
		}
		do_dma();
	}

	uint32_t get_dmaddr(int chnum)  {
		if constexpr (SHOW_LOGS) {
			std::cout << "DMA Channel " << (int)chnum << " ADDR READ ";
			log_32(channels[chnum].addr);
			std::cout << "\n";
		}
		return channels[chnum].addr;
	}

	uint32_t get_dmabcr(int chnum)  {
		if constexpr (SHOW_LOGS) {
			std::cout << "DMA Channel " << (int)chnum << " BCR  READ "; 
			log_32(channels[chnum].bcr);
			std::cout << "\n";
		}
		return channels[chnum].bcr; 
	}

	uint32_t get_dmachcr(int chnum) {
		if constexpr (SHOW_LOGS) {
			std::cout << "DMA Channel " << (int)chnum << " ADDR READ ";
			log_32(channels[chnum].chcr);
			std::cout << "\n";
		}
		return channels[chnum].chcr;
	}

	void set_dpcr(uint32_t value) {
		dpcr = value;
		if constexpr (SHOW_LOGS) {
			std::cout << "DPCR WRITE ";
			log_32(dpcr);
			std::cout << "\n";
		}
		do_dma(); 
	}

	void set_dicr(uint32_t value) {
		dicr = value;
		if constexpr (SHOW_LOGS) {
			std::cout << "DICR WRITE ";
			log_32(dicr);
			std::cout << "\n";
		}
		do_dma();
	}

	uint32_t get_dpcr() {
		if constexpr (SHOW_LOGS) {
			std::cout << "DPCR READ ";
			log_32(dpcr);
			std::cout << "\n";
		}
		return dpcr;
	}

	uint32_t get_dicr() {
		if constexpr (SHOW_LOGS) {
			std::cout << "DICR READ ";
			log_32(dpcr);
			std::cout << "\n";
		}
		return dicr;
	}

	void connect_gpu(GPU* gpu) {
		this->gpu = gpu;
	}

	void connect_memory(Memory* memory) {
		this->memory = memory;
	}

	// dma.cpp
	void do_dma();
	bool channel_condition(int chnum);

	void do_dma0_transfer();
	void do_dma1_transfer();
	void do_dma2_transfer();
	void do_dma3_transfer();
	void do_dma4_transfer();
	void do_dma5_transfer();
	void do_dma6_transfer();

	void tick_component();

private:
	Channel channels[7] {};

	GPU* gpu = nullptr;
	Memory* memory = nullptr;

	uint64_t dma_tick = 0;

	uint32_t dpcr = 0;
	uint32_t dicr = 0;

};