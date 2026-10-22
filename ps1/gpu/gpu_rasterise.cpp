#include "gpu.hpp"
#include <algorithm>

const int8_t dither_table[16] = {
	-4,  0, -3,  1,
	 2, -2,  3, -1,
	-3,  1, -4,  0,
	 3, -1,  2, -2
};

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

Weights interpolate_barycentric(Vertex p, Vertex a, Vertex b, Vertex c) {
	float divisor = (float)(((b.y - c.y) * (a.x - c.x)) + ((c.x - b.x) * (a.y - c.y)));
	Weights w;
	w.a = (float)(((b.y - c.y) * (p.x - c.x)) + ((c.x - b.x) * (p.y - c.y))) / divisor;
	w.b = (float)(((c.y - a.y) * (p.x - c.x)) + ((a.x - c.x) * (p.y - c.y))) / divisor;
	w.c = 1 - w.a - w.b;
	return w;
}

uint32_t apply_weights(Weights w, uint32_t a, uint32_t b, uint32_t c) {
	return (w.a * a) + (w.b * b) + (w.c * c);
}

void dither(uint8_t& c, int8_t dither) {
	if (c + dither > 255) {
		c = 255;
	} else if (c + dither < 0) {
		c = 0;
	} else {
		c += dither;
	}
}

void dither_16(uint16_t& c, int8_t dither) {
	if (c + dither > 65535) {
		c = 65535;
	} else if (c + dither < 0) {
		c = 0;
	} else {
		c += dither;
	}
}

void GPU::rasterise_triangle(Vertex a, Vertex b, Vertex c) {
	Weights w;

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

	uint16_t pixel = 0x00;

	if (polygon.render_type == PolygonRenderType::FlatShadeNOTEX) {
		uint8_t col_r = polygon.r0 >> 3;
		uint8_t col_g = polygon.g0 >> 3;
		uint8_t col_b = polygon.b0 >> 3;
		pixel = (col_b << 10) | (col_g << 5) | col_r;
	}

	for (int x = min_x; x < max_x; x++) {
		for (int y = min_y; y < max_y; y++) {
			Vertex p;
			p.x = x;
			p.y = y;
			if (is_inside_triangle(p, a, b, c)) {
				if (p.x >= 0 && p.x < 1024 && p.y >= 0 && p.y < 512) {
					uint8_t col_r, col_g, col_b;
					uint32_t tex_u, tex_v;
					switch (polygon.render_type) {
						case PolygonRenderType::GourardShadeNOTEX: {
							w = interpolate_barycentric(p, a, b, c);
							uint16_t col_r_16 = apply_weights(w, a.r, b.r, c.r);
							uint16_t col_g_16 = apply_weights(w, a.g, b.g, c.g);
							uint16_t col_b_16 = apply_weights(w, a.b, b.b, c.b);

							int8_t dither_value = dither_table[((4 * (p.y & 3)) + (p.x & 3)) & 15];
							dither_16(col_r_16, dither_value);
							dither_16(col_g_16, dither_value);
							dither_16(col_b_16, dither_value);

							col_r = col_r_16 >> 3;
							col_g = col_g_16 >> 3;
							col_b = col_b_16 >> 3;

							pixel = (col_b << 10) | (col_g << 5) | col_r;
							break;
						}
						case PolygonRenderType::FlatModTEX:
						case PolygonRenderType::GourardModTEX:
						case PolygonRenderType::RawTEX:
							w = interpolate_barycentric(p, a, b, c);
							tex_u = apply_weights(w, a.u, b.u, c.u);
							tex_v = apply_weights(w, a.v, b.v, c.v);
							pixel = fetch_polygon_texture(tex_u, tex_v);
							break;
						default:
							break;
					}
					if (polygon.render_type == PolygonRenderType::FlatModTEX || polygon.render_type == PolygonRenderType::GourardModTEX) {
						col_r = pixel & 0x1F;
						col_g = (pixel >> 5)  & 0x1F;
						col_b = (pixel >> 10) & 0x1F;

						int8_t dither_value = dither_table[((4 * (p.y & 3)) + (p.x & 3)) & 15];
						dither(col_r, dither_value);
						dither(col_g, dither_value);
						dither(col_b, dither_value);

						pixel = ((col_b & 0x1F) << 10) | ((col_g & 0x1F) << 5) | (col_r & 0x1F);
					}
					if (pixel) {
						vram[p.y][p.x] = pixel;
					}
				}
			}
		}
	}
}

uint16_t apply_mask(uint16_t tex_coord, uint8_t mask, uint8_t offset) {
	tex_coord = (tex_coord & ~(mask << 3)) | ((offset & mask) << 3);
	return tex_coord;
}

// Seems to get visual results(?) But texture lookup still completely failing... (just receiving orange colour)3
uint16_t GPU::fetch_polygon_texture(uint16_t tex_u, uint16_t tex_v) {
	
	uint16_t u = apply_mask(tex_u, texture_window_mask_x, texture_window_offset_x);
	uint16_t v = apply_mask(tex_v, texture_window_mask_y, texture_window_offset_y);

	uint16_t page_x = TBX * 64;
	uint16_t page_y = TBY * 256;
	uint32_t clut_x = polygon.CLX * 16;
	uint32_t clut_y = polygon.CLY;
	
	int palette_index = 0;
	uint16_t texel = 0x00;
	uint16_t pixel = 0x00;
	
	switch (TPF) {
		case 0:
			texel = vram[page_y + v + (u / 4096)][(page_x + (u / 4)) & 1023];
			palette_index = (texel >> (u & 3) * 4) & 0xF;
			pixel = vram[clut_y + (palette_index / 1024)][(clut_x + palette_index) & 1023];
			break;
		case 1:
			texel = vram[page_y + v + (u / 2048)][(page_x + (u / 2)) & 1023];
			palette_index = (texel >> (u & 1) * 8) & 0xFF;
			pixel = vram[clut_y + (palette_index / 1024)][(clut_x + palette_index) & 1023];
			break;
		case 2:
			pixel = vram[page_y + v + (u / 1024)][(page_x + u) & 1023];
			break;
		default:
			if constexpr (SHOW_LOGS) std::cout << "RESERVED TPF VALUE\n";
			break;
	}

	return pixel;
}

/*
Get vram at that UV coord (considering 4-bit, 8-bit colour support)?

Unsure on exact formulae for handling 4-bit/8-bit textures...

*/