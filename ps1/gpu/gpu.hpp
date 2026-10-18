#pragma once
#include "utility.hpp"
#include "gpu_state.hpp"
#include "utility.hpp"
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <vector>
#include <SDL3/SDL.h>

constexpr bool SHOW_VRAM_VIEWER = true;

constexpr int MAIN_WIDTH  = 320;
constexpr int MAIN_HEIGHT = 240;
constexpr int VRAM_WIDTH  = 1024;
constexpr int VRAM_HEIGHT = 512;
constexpr uint32_t GPU_VERSION = 2;

class GPU {
public:
	GPU() {
		vram.assign(VRAM_HEIGHT, {});
		for (auto& v : vram) {
			v.assign(VRAM_WIDTH, 0);
		}
	}

	~GPU() {
		close_window();
	}

	void initialise() {
		create_window();
	}

	// gpu_display.cpp
	void update_vram_viewer();
	void create_window();
	void close_window();
	void sdl_loop();

	// gpu.cpp
	void receive_command(uint32_t value);

	void blit_cpu_vram(uint16_t value);
	uint16_t blit_vram_cpu();
	void blit_command(uint32_t value);

	uint32_t gp0_read();
	void gp0_write(uint32_t value);
		
	void set_gp1(uint32_t value);
	void set_gp1_bit(int bit);
	void clear_gp1_bit(int bit);
	uint32_t gp1_read();
	void gp1_write(uint32_t value);

	bool window_closed() {
		return main_window_closed && vram_viewer_closed;
	}

private:

	std::vector<std::vector<uint16_t>> vram;

	GPUState gpu_state = GPUState::Idle;
	Blitting blit;

	bool gp1_bypass = false;
	uint32_t gp1_bypass_value;

	bool PBW = false; // if on, force bit 1 for writes and transfers
	bool PBC = false; // if on, only draw if bit 15 of the old pixel is 0

	uint32_t gp1 = 0x00; // MIGHT CHANGE

	// Main Window
	bool main_window_closed = true;
	SDL_Window* main_window = nullptr;
	SDL_Renderer* main_renderer = nullptr;
	SDL_Texture* main_texture = nullptr;
	SDL_FRect main_rect;

	// VRAM Viewer
	bool vram_viewer_closed = true;
	SDL_Window* vram_viewer = nullptr;
	SDL_Renderer* vram_renderer = nullptr;
	SDL_Texture* vram_texture = nullptr;
	SDL_FRect vram_rect;
};
