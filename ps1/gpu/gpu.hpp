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
	uint8_t convert_to_8bit(uint8_t value);
	uint32_t convert_to_rgba(uint16_t colour);
	std::vector<uint32_t> vram_framebuffer();
	void update_vram_viewer();
	void create_window();
	void close_window();
	void sdl_loop();

	// gpu_rasterise.cpp
	void draw_rectangle();
	void draw_quick_rectangle();
	void draw_polygon();
	void add_offset(Vertex& v);
	void rasterise_triangle(Vertex a, Vertex b, Vertex c);

	// gpu.cpp
	void receive_command(uint32_t value);
	void blit_cpu_vram(uint16_t value);
	uint16_t blit_vram_cpu();
	void blit_command(uint32_t value);

	void receive_polygon(uint32_t value);
	void handle_polygon(uint32_t value);

	void receive_rectangle(uint32_t value);
	void handle_rectangle(uint32_t value);
	void pass_clut(uint16_t clut);
	void pass_page(uint16_t page);

	void receive_quick_rectangle(uint32_t value);
	void handle_quick_rectangle(uint32_t value);

	uint32_t gp0_read();
	void gp0_write(uint32_t value);
		
	void set_gp1(uint32_t value);
	void set_gp1_bit(int bit);
	void clear_gp1_bit(int bit);
	uint32_t gp1_read();
	void gp1_write(uint32_t value);

	bool window_closed() {
		return main_window_closed || vram_viewer_closed;
	}

	bool DREQ();

private:

	std::vector<std::vector<uint16_t>> vram;

	GPUState gpu_state = GPUState::Idle;
	Blitting blit;

	// Rendering buffers
	Rectangle rectangle;
	Polygon polygon;
	QuickRectangle quick_rectangle;

	bool gp1_bypass = false;
	uint32_t gp1_bypass_value = 0;


	// Rendering Attributes

	uint32_t TBX = 0;
	uint32_t TBY = 0;
	uint32_t ABR = 0;
	uint32_t TPF = 0;
	uint32_t TBY2 = 0;
	bool DTD = false;
	bool DFE = false;
	bool IX = false;
	bool IY = false;

	uint8_t texture_window_mask_x = 0;
	uint8_t texture_window_mask_y = 0;
	uint8_t texture_window_offset_x  = 0;
	uint8_t texture_window_offset_y = 0;
	uint16_t drawing_area_top_left_x = 0;
	uint16_t drawing_area_top_left_y = 0;
	uint16_t drawing_area_bottom_right_x = 0;
	uint16_t drawing_area_bottom_right_y = 0;
	int16_t drawing_x_offset = 0;
	int16_t drawing_y_offset = 0;
	bool PBW = false; // if on, force bit 1 for writes and transfers
	bool PBC = false; // if on, only draw if bit 15 of the old pixel is 0

	uint8_t HDS = 0;
	bool VDS = false;
	bool NPB = false;
	bool LBS = false;
	bool IRS = false;
	bool HDS2 = false;
	bool REV = false;
	bool DMSK = false;
	bool IRQ = false;
	bool IDLE = false;
	bool RFFL = false;
	bool WFEP = false;
	bool ODE = false;
	uint8_t DMD = 0;
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
