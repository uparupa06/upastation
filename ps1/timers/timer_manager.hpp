#pragma once
#include "utility.hpp"

struct Timer {
	// COUNT
	uint16_t value = 0;
		
	// MODE
	bool GATF = false;
	uint8_t GATM = 0;
	bool ZRET = false;
	bool CMP  = false;
	bool OVFL = false;
	bool REPT = false;
	bool LEVL = false;
	bool EXTC = false;
	bool PSCL = false;
	bool INTF = false;
	bool EQUF = false;
	bool OVFF = false;

	// COMP/TARGET
	uint16_t target = 0;

	// Internal flags
	bool blank_reset = true; // For GATM synchronisation mode 1 and 2
	bool blank_occurred = true; // For GATM synchronisation mode 3

	bool delay = false;
};

class TimerManager {
public:

	TimerManager() {}

	// timer_manager_rw.cpp
	uint32_t timer_read(uint32_t addr);
	void timer_write(uint32_t addr, uint32_t value);

	// timer_manager.cpp
	void tick_timers();
	void tick_normal_timer(Timer& timer, bool use_vblank);
	void tick_prescaled_timer(Timer& timer);

	// timer_ext.cpp
	bool blank(bool use_vblank);
	void irq();

private:
	Timer timers[3];
};