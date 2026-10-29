#include "timer_manager.hpp"


// REVIEW HOW IRQ WORKS (OVERSIMPLIFICATION?)

void TimerManager::tick_normal_timer(Timer& timer, bool use_vblank) {
	if (timer.GATF) {
		switch (timer.GATM) {
			case 0:
				if (blank(use_vblank)) {
					return;
				}
				break;
			case 1:
				if (!blank(use_vblank)) {
					timer.blank_reset = true;
				} else if (timer.blank_reset) {
					timer.value = 0;
					timer.blank_reset = false;
				}
				break;
			case 2:
				if (!blank(use_vblank)) {
					return;
				} else if (timer.blank_reset) {
					timer.value = 0;
					timer.blank_reset = false;
				}
				break;
			case 3:
				if (!blank(use_vblank)) {
					timer.blank_occurred = false;
					return;
				} else if (timer.blank_occurred) {
					timer.blank_occurred = true;
					timer.GATF = false;
					return;
				}
				break;
			default:
				break;
		}
	}

	if ((timer.CMP && timer.value == timer.target) || (timer.OVFL && timer.value == 0xFFFF)) {
		irq(); // TO-DO: REPT, LEVL
	}

	if (timer.value == timer.target) {
		timer.EQUF = true;
	}

	if (timer.value == 0xFFFF) {
		timer.OVFF = true;
	}

	if (timer.ZRET && timer.value == timer.target) {
		timer.value = 0;
		timer.delay = true;
		return;
	}

	if (!timer.delay) {
		timer.value++; // TO DO: EXTC
	} else {
		timer.delay = false;
	}
	// CONTINUE FROM ZRET
}

void TimerManager::tick_prescaled_timer(Timer& timer) {
	if (timer.GATF && (timer.GATM == 0 || timer.GATM == 3)) {
		return;
	}

	if ((timer.CMP && timer.value == timer.target) || (timer.OVFL && timer.value == 0xFFFF)) {
		irq(); // TO-DO: REPT, LEVL
	}

	if (timer.ZRET && timer.value == timer.target) {
		timer.value = 0;
		timer.delay = true;
		return;
	}

	if (!timer.delay) {
		timer.value++; // TO-DO: PSCL
	} else {
		timer.delay = false;
	}
}

void TimerManager::tick_timers() {
	tick_normal_timer(timers[0], false);
	tick_normal_timer(timers[1], true);
	tick_prescaled_timer(timers[2]);
	return;
}