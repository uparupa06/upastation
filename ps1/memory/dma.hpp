#pragma once
#include "utility.hpp"
#include <cstdint>
#include <iostream>

constexpr uint32_t INITIAL_DPCR = 0x7654321;

struct DMAChannel {
	uint32_t dmaddr, dmabcr, dmachcr;

	uint32_t start_address; // DMADDR
	uint32_t bcbs, ba; // DMABCR

	// DMACHCR
	bool ram_to_device, madr_decrement, special_behaviour;
	uint8_t transfer_mode;
	uint16_t chopping_dma_window_size, chopping_cpu_window_size;
	bool force_transfer_start, override, bus_snooping;

	// DPCR
	bool priority, master_enable;

	// DICR
	bool interrupt_behaviour;
	bool interrupt;

	void reset_interrupt(bool value) {
		if (value) {
			// TO-DO LATER: RESET BEHAVIOUR (FIND OUT LATER HOW THIS WORKS)

		}
	}
};

class DMA {
public:
	DMA() { }

	void initialise() {
		set_dpcr(INITIAL_DPCR);
	}

	void set_dmaddr(uint32_t value, int chnum) {
		DMAChannel& ch = channels[chnum];
		ch.dmaddr = value;
		ch.start_address = extract_bits(value, 0, 23);
		std::cout << "DMADDR " << (int)chnum << "\n";
	}

	void set_dmabcr(uint32_t value, int chnum) {
		DMAChannel& ch = channels[chnum];
		ch.dmabcr = value;
		ch.bcbs = extract_bits(value, 0, 15);
		ch.ba = extract_bits(value, 16, 31);
		std::cout << "DMABCR " << (int)chnum << "\n";
	}

	void set_dmachcr(uint32_t value, int chnum) {
		DMAChannel& ch = channels[chnum];
		ch.dmachcr = value;
		
		ch.ram_to_device = get_bit(value, 0);
		ch.madr_decrement = get_bit(value, 1);
		ch.special_behaviour = get_bit(value, 8);
		
		ch.transfer_mode = extract_bits(value, 9, 10);
		
		ch.chopping_dma_window_size = (1u << extract_bits(value, 16, 18));
		ch.chopping_cpu_window_size = (1u << extract_bits(value, 20, 22));

		ch.force_transfer_start = get_bit(value, 28);
		ch.override = get_bit(value, 29);
		ch.bus_snooping = get_bit(value, 30);
		std::cout << "DMACHCR " << (int)chnum << "\n";
	}

	uint32_t get_dmaddr(int chnum) {
		return channels[chnum].dmaddr;
	}

	uint32_t get_dmabcr(int chnum) {
		return channels[chnum].dmabcr;
	}

	uint32_t get_dmachcr(int chnum) {
		return channels[chnum].dmachcr;
	}

	void set_dpcr(uint32_t value) {
		dpcr = value;
		uint32_t temp = value;

		for (auto& ch : channels) {
			ch.priority = extract_bits(temp, 0, 2);
			ch.master_enable = get_bit(temp, 3);
			temp >>= 4;
		}
		
		cpu_memory_access_priority = extract_bits(value, 28, 30);
	}

	void set_dicr(uint32_t value) {
		dicr = (value & ~(0xFF << 24));

		uint32_t temp;

		temp = extract_bits(dicr, 0, 6);
		for (auto& ch : channels) {
			ch.interrupt_behaviour = (temp & 1u);
			temp >>= 1;
		}

		bus_error_flag = get_bit(value, 15);
		interrupt_mask = extract_bits(value, 16, 22);
		master_channel_interrupt_enable = get_bit(value, 23);

		temp = extract_bits(value, 24, 30);
		for (auto& ch : channels) {
			ch.reset_interrupt(temp & 1u);
			temp >>= 1;
		}

		master_interrupt_flag = (bus_error_flag || (master_channel_interrupt_enable && (temp > 0)));
		dicr |= (master_interrupt_flag << 31);

	}

	uint32_t get_dpcr() {
		return dpcr;
	}

	uint32_t get_dicr() {
		dicr &= ~(0x7F << 24);

		uint32_t temp = 0;
		for (int i = 7; i >= 0; i--) {
			if (interrupt_mask & (1u << i)) {
				temp |= channels[i].interrupt;
			}
			temp <<= 1;
		}

		dicr |= (temp << 24);

		return dicr;

	}

private:

	DMAChannel channels[7] {};

	uint32_t dpcr, dicr; // DICR will be bitwise OR'ed to include values of bits 24-30

	// DPCR
	uint32_t cpu_memory_access_priority;

	// DICR
	bool bus_error_flag;
	uint32_t interrupt_mask;
	bool master_channel_interrupt_enable;
	bool master_interrupt_flag;

};