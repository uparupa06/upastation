#include "memory.hpp"
#include "dma.hpp"
#include <iostream>

// NEED TO REVIEW THIS METHOD OF UPDATING THE REGISTERS...
void Memory::write_io(uint32_t addr, uint8_t value) {
	uint32_t word_addr = (addr & ~(0x3));

	io_old_value  = read_io(addr);
	io_value_mask = io_old_value & ~(0xFF << (addr & 0x3));
	io_new_value |= (value << (addr & 0x3));

	switch (word_addr & 0xFFFFFF00) {
		case 0x1F801000: {
			if ((word_addr & 0xF0) >= 0x80 && (word_addr & 0xF0) <= 0xE0) {
				
				int chnum = (word_addr & 0xFF) - 8;

				switch (word_addr & 0xF) {
				case 0x0:
					dma.set_dmaddr(io_new_value, chnum);
					break;
				case 0x4:
					dma.set_dmabcr(io_new_value, chnum);
					break;
				case 0x8:
					dma.set_dmachcr(io_new_value, chnum);
					break;
				default:
					break;
				}

			} else if ((word_addr & 0xF4) == 0xF0) {
				dma.set_dpcr(io_new_value);
			} else if ((word_addr & 0xFF) == 0xF4) {
				dma.set_dicr(io_new_value);
			}

			return;
		}
	}
}

uint32_t Memory::read_io(uint32_t addr) {
	uint32_t word_addr = (addr & ~(0x3));

	switch (word_addr & 0xFFFFFF00) {
		case 0x1F801000: {
			if ((word_addr & 0xF0) >= 0x80 && (word_addr & 0xF0) <= 0xE0) {
				
				int chnum = (word_addr & 0xFF) - 8;

				switch (word_addr & 0xF) {
				case 0x0:
					return dma.get_dmaddr(chnum);
				case 0x4:
					return dma.get_dmabcr(chnum);
				case 0x8:
					return dma.get_dmachcr(chnum);
				default:
					break;
				}

			} else if ((word_addr & 0xF4) == 0xF0) {
				return dma.get_dpcr();
			} else if ((word_addr & 0xFF) == 0xF4) {
				return dma.get_dicr();
			}

			return 0x00;
		}
	}

	return 0x00;
}

void Memory::write_gpu(bool is_gp1, uint32_t value) {
	if (is_gp1) {
		gpu.gp1_write(value);
	} else {
		gpu.gp0_write(value);
	}
}

uint32_t Memory::read_gpu(bool is_gp1) {
	return is_gp1 ? gpu.gp1_read() : gpu.gp0_read();
}
