#include "gpu.hpp"

void GPU::draw_rectangle() {
	switch (rectangle.size) {
		case 1:
			rectangle.width  = 1;
			rectangle.height = 1;
			break;
		case 2:
			rectangle.width  = 8;
			rectangle.height = 8;
			break;
		case 3:
			rectangle.width  = 16;
			rectangle.height = 16;
	}

	uint8_t r = rectangle.r >> 3;
	uint8_t g = rectangle.g >> 3;
	uint8_t b = rectangle.b >> 3;
	uint16_t pixel = (b << 10) | (g << 5) | r;

	for (int y = 0; y < rectangle.height; y++) {
		for (int x = 0; x < rectangle.width; x++) {
			if (rectangle.TGE) {
				// handle textures here
			}
			uint32_t rect_x = (rectangle.upper_left_x + x) & 1023;
			uint32_t rect_y = (rectangle.upper_left_y + y) & 511;

			vram[rect_y][rect_x] = pixel;
		}
	}
}

void GPU::draw_quick_rectangle() {
	uint8_t r = quick_rectangle.r >> 3;
	uint8_t g = quick_rectangle.g >> 3;
	uint8_t b = quick_rectangle.b >> 3;
	uint16_t pixel = (b << 10) | (g << 5) | r;

	for (int oy = 0; oy < quick_rectangle.height; oy++) {
		for (int ox = 0; ox < quick_rectangle.width; ox++) {
			uint32_t rect_x = (quick_rectangle.x + ox) & 1023;
			uint32_t rect_y = (quick_rectangle.y + oy) & 511;

			vram[rect_y][rect_x] = pixel;
		}
	}
}