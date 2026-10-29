#include "timer_manager.hpp"

uint32_t TimerManager::timer_read(uint32_t addr) {
	uint8_t timer_idx = (addr >> 4) & 0xF;
	if (timer_idx > 2) {
		if constexpr (SHOW_LOGS) {
			std::cout << "UNEXPECTED TIMER READ\n";
		}
		return 0x00;
	}
	Timer& timer = timers[timer_idx];

	switch (addr & 0xF) {
	case 0x0:
		return timer.value;
	case 0x4: {
		uint32_t value = timer.GATF        | (timer.GATM << 1)  | (timer.ZRET << 3) |
		                (timer.CMP << 4)   | (timer.OVFL << 5)  | (timer.REPT << 6) |
		                (timer.LEVL << 7)  | (timer.EXTC << 8)  | (timer.PSCL << 9) |
	                    (timer.INTF << 10) | (timer.EQUF << 11) | (timer.OVFF << 12);
	    timer.EQUF = false;
	    timer.OVFF = false;
	    return value;
	}
	case 0x8:
		return timer.target;
	default:
		if constexpr (SHOW_LOGS) {
			std::cout << "UNEXPECTED TIMER READ\n";
		}
		return 0x00;
	}
}

void TimerManager::timer_write(uint32_t addr, uint32_t value) {
	uint8_t timer_idx = (addr >> 4) & 0xF;
	if (timer_idx > 2) {
		if constexpr (SHOW_LOGS) {
			std::cout << "UNEXPECTED TIMER WRITE\n";
		}
		return;
	}
	Timer& timer = timers[timer_idx];

	switch (addr & 0xF) {
		case 0x0:
			timer.value = (value & 0xFFFF);
			break;
		case 0x4:
			timer.GATF = (value & 1);
			timer.GATM = ((value >> 1) & 3);
			timer.ZRET = (value >> 3)  & 1;
			timer.CMP  = (value >> 4)  & 1;
			timer.OVFL = (value >> 5)  & 1;
			timer.REPT = (value >> 6)  & 1;
			timer.LEVL = (value >> 7)  & 1;
			timer.EXTC = (value >> 8)  & 1;
			timer.PSCL = (value >> 9)  & 1;
			timer.INTF = true;
			break;
		case 0x8:
			timer.target = (value & 0xFFFF);
			break;
		default:
			if constexpr (SHOW_LOGS) {
				std::cout << "UNEXPECTED TIMER WRITE\n";
			}
			break;
	}
}
