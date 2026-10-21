#include "gpu.hpp"
#include <algorithm>

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

void GPU::draw_polygon() {
	rasterise_triangle(polygon.vertices[0], polygon.vertices[1], polygon.vertices[2]);
	if (polygon.VTX) {
		rasterise_triangle(polygon.vertices[1], polygon.vertices[2], polygon.vertices[3]);
	}
	return;
}

void GPU::add_offset(Vertex& v) {
	v.x += drawing_x_offset;
	v.y += drawing_y_offset;
}

int cross_product_z(Vertex a, Vertex b, Vertex c) {
	return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

bool triangle_condition(Vertex p, Vertex a, Vertex b) {
	return cross_product_z(a, b, p) < 0;
}

bool is_inside_triangle(Vertex p, Vertex a, Vertex b, Vertex c) {
	return !(triangle_condition(p, a, b) || triangle_condition(p, b, c) || triangle_condition(p, c, a));
}

void GPU::rasterise_triangle(Vertex a, Vertex b, Vertex c) {

	int cross_prod = cross_product_z(a, b, c);
	if (cross_prod < 0) {
		Vertex temp = a;
		a = b;
		b = temp;
	}

	add_offset(a);
	add_offset(b);
	add_offset(c);

	int16_t min_x = std::min(a.x, std::min(b.x, c.x));
	int16_t max_x = std::max(a.x, std::max(b.x, c.x));
	int16_t min_y = std::min(a.y, std::min(b.y, c.y));
	int16_t max_y = std::max(a.y, std::max(b.y, c.y));

	min_x = std::max(static_cast<int32_t>(min_x), static_cast<int32_t>(drawing_area_top_left_x));
	max_x = std::min(static_cast<int32_t>(max_x), static_cast<int32_t>(drawing_area_bottom_right_x));
	min_y = std::max(static_cast<int32_t>(min_y), static_cast<int32_t>(drawing_area_top_left_y));
	max_y = std::min(static_cast<int32_t>(max_y), static_cast<int32_t>(drawing_area_bottom_right_y));

	if (min_x > max_x || min_y > max_y) {
		return;
	}

	uint8_t col_r = polygon.r0 >> 3;
	uint8_t col_g = polygon.g0 >> 3;
	uint8_t col_b = polygon.b0 >> 3;
	uint16_t pixel = (col_b << 10) | (col_g << 5) | col_r;

	for (int x = min_x; x < max_x; x++) {
		for (int y = min_y; y < max_y; y++) {
			Vertex p;
			p.x = x;
			p.y = y;
			if (is_inside_triangle(p, a, b, c)) {
				if (p.x >= 0 && p.x < 1024 && p.y >= 0 && p.y < 512) {
					vram[p.y][p.x] = pixel;
				}
			}
		}
	}
}