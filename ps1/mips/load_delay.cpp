#include "mips.hpp"

void MIPS::update_load_delay() {
	for (int i = 0; i < NUM_MAIN_CPU_REGISTERS; i++) {
		if (load_pending[i] != NO_DELAY) {
			switch (load_pending[i]) {
				case START_DELAY:
					load_pending[i] = DELAYING;
					break;
				case DELAYING:
					load_pending[i] = NO_DELAY;
					if (i != 0) {
						r[i] = load_value[i];
					}
					break;
				default:
					break;
			}
		}
	}
}

void MIPS::load(int register_number, uint32_t value) {
	

	load_value[register_number]   = value;
	load_pending[register_number] = START_DELAY;
}

void MIPS::write_register(int register_number, uint32_t value) {
	if (register_number == 0) {
		return;
	}
	r[register_number] = value;
	load_pending[register_number] = NO_DELAY;
}
