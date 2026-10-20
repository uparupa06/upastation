#include "dma.hpp"
#include "memory.hpp"

#define MAX_TRANSFER 0x10000
#define CHCR_START_TRANSFER 24

void DMA::do_dma() {
	std::vector<int> enabled_channels {};
	for (int i = 0; i < 7; i++) {
		bool master_enable  = get_bit(dpcr, (4 * i) + 3);
		bool start_transfer = get_bit(channels[i].chcr, CHCR_START_TRANSFER);

		if (master_enable && start_transfer && channel_condition(i)) {
			channels[i].priority = extract_bits(dpcr, 4 * i, (4 * i) + 2);
			enabled_channels.push_back(i);
		}
	}

	std::sort(enabled_channels.begin(), enabled_channels.end(), [this](const int& a, const int& b) {
		if (channels[a].priority == channels[b].priority) {
			return a > b;
		} else {
			return (channels[a].priority > channels[b].priority);
		}
	});

	for (auto i : enabled_channels) {
		switch(i) {
			case 0:	do_dma0_transfer(); break;
			case 1:	do_dma1_transfer(); break;
			case 2:	do_dma2_transfer(); break;
			case 3:	do_dma3_transfer(); break;
			case 4:	do_dma4_transfer(); break;
			case 5:	do_dma5_transfer(); break;
			case 6: do_dma6_transfer(); break;
			default: std::cout << "ATTEMPTING TRANSFER WITH NON-EXISTENT CHANNEL\n"; break;
		}
	}
}

bool DMA::channel_condition(int chnum) {
	if (chnum == 2) {
		return gpu->DREQ();
	}
	return true;
}

void DMA::do_dma0_transfer() {
	std::cout << "DOING DMA0 transfer\n";
	clear_bit(channels[0].chcr, CHCR_START_TRANSFER);
}

void DMA::do_dma1_transfer() {
	std::cout << "DOING DMA1 transfer\n";
	clear_bit(channels[1].chcr, CHCR_START_TRANSFER);
}

void DMA::do_dma2_transfer() {
	Channel& ch = channels[2];
	uint32_t transfer_mode = extract_bits(ch.chcr, 9, 10);
	switch (transfer_mode) {
		case 0:
		case 1: {
			std::cout << "BURST/SLICE MODE DMA\n";
			uint32_t address = extract_bits(ch.addr, 0, 23);
			uint32_t dma_len = extract_bits(ch.bcr, 0, 15);
			if (transfer_mode == 1) {
				dma_len *= extract_bits(ch.bcr, 16, 31);
			}
			for (int i = 0; i < dma_len; i++) {
				uint32_t data = memory->read32(address);
				gpu->gp0_write(data);
				address += 4;
			}
			break;
		}
		case 2: {
			std::cout << "LINKED LIST DMA2\n";

			uint32_t address = extract_bits(ch.addr, 0, 23);
			uint32_t next_address = 0;
			uint32_t transferred = 0;
			do {
				uint32_t header = memory->read32(address);
				uint32_t data_words = header >> 24;
				for (int i = 0; i < data_words; i++) {
					uint32_t data = memory->read32(address + 4 * (i + 1));
					gpu->gp0_write(data);
				}
				next_address = header & 0xFFFFFF;
				if (!(next_address & (1 << 23))) {
					address = next_address;
				}
				transferred++;
			} while (!(next_address & (1 << 23)) && transferred < MAX_TRANSFER);

			break;
		}
		default:
			std::cout << "CALLING RESERVED TRANSFER MODE FOR DMA2\n";
			break;
	}
	std::cout << "DID DMA2 transfer\n";
	clear_bit(ch.chcr, CHCR_START_TRANSFER);
}

void DMA::do_dma3_transfer() {
	std::cout << "DOING DMA3 transfer\n";
	clear_bit(channels[3].chcr, CHCR_START_TRANSFER);
}

void DMA::do_dma4_transfer() {
	std::cout << "DOING DMA4 transfer\n";
	clear_bit(channels[4].chcr, CHCR_START_TRANSFER);
}

void DMA::do_dma5_transfer() {
	std::cout << "DOING DMA5 transfer\n";
	clear_bit(channels[5].chcr, CHCR_START_TRANSFER);
}

void DMA::do_dma6_transfer() {
	Channel& ch = channels[6];
	uint32_t address = extract_bits(ch.addr, 0, 23);
	uint32_t dma_len = extract_bits(ch.bcr, 0, 15);
	if (dma_len == 0) {
		dma_len = 0x10000;
	}
	for (int i = 0; i < dma_len; i++) {
		uint32_t header;
		if (i == dma_len - 1) {
			header = 0xFFFFFF;
		} else {
			header = address - 4;
		}
		memory->write32(address, header);
		address -= 4;
	}
	std::cout << "DID DMA6 transfer\n";
	clear_bit(ch.chcr, CHCR_START_TRANSFER);
}

void DMA::tick_component() {
	dma_tick++;
	// FOR DELAYING IRQ
}