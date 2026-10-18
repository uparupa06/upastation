#pragma once

enum GPUState {
	Idle,

	// Blitting commands

	// VRAM-VRAM
	VRAM_VRAM_Source_Coord,
	VRAM_VRAM_Destination_Coord,
	VRAM_VRAM_Width_Height,

	// CPU-VRAM
	CPU_VRAM_Destination_Coord,
	CPU_VRAM_Width_Height,
	CPU_VRAM_Data,

	// VRAM-CPU
	VRAM_CPU_Source_Coord,
	VRAM_CPU_Width_Height,
	VRAM_CPU_Data,
};


// Blitting commands
struct Coordinate {
	uint16_t x;
	uint16_t y;
};

struct Size {
	uint16_t width;
	uint16_t height;
};

struct Blitting {
	Coordinate src, dest;
	Size size;
	int counter; // Using this to track progress with the transfers
	int x, y;
};