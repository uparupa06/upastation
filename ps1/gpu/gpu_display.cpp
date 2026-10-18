#include "gpu.hpp"

uint8_t GPU::convert_to_8bit(uint8_t value) {
	return static_cast<uint8_t>(static_cast<float>(value) * 255.0f / 31.0f);
}

uint32_t GPU::convert_to_rgba(uint16_t pixel) {
	uint8_t r = convert_to_8bit(pixel & 0x1F);
	uint8_t g = convert_to_8bit((pixel >> 5) & 0x1F);
	uint8_t b = convert_to_8bit((pixel >> 10) & 0x1F);

	return (r << 24) | (g << 16) | (b << 8) | 0xFF;
}

std::vector<uint32_t> GPU::vram_framebuffer() {
	std::vector<uint32_t> framebuffer {};

	for (int y = 0; y < VRAM_HEIGHT; y++) {
		for (int x = 0; x < VRAM_WIDTH; x++) {
			uint16_t pixel = vram[y][x];
			framebuffer.push_back(convert_to_rgba(pixel));
		}
	}

	return framebuffer;
}

void GPU::update_vram_viewer() {
	int window_width;
	int window_height;

	SDL_GetWindowSize(vram_viewer, &window_width, &window_height);

	int scale = std::max(1, std::min(
		window_width / VRAM_WIDTH,
		window_height / VRAM_HEIGHT
	));

	float render_width = static_cast<float>(VRAM_WIDTH * scale);
	float render_height = static_cast<float>(VRAM_HEIGHT * scale);

	SDL_UpdateTexture(
		vram_texture,
		nullptr,
		vram_framebuffer().data(),
		VRAM_WIDTH * sizeof(uint32_t)
	);

	SDL_RenderClear(vram_renderer);

	vram_rect = {
		(window_width - render_width) * 0.5f,
		(window_height - render_height) * 0.5f,
		render_width,
		render_height
	};

	SDL_RenderTexture(
		vram_renderer,
		vram_texture,
		nullptr,
		&vram_rect
	);

	SDL_RenderPresent(vram_renderer);
}

void GPU::create_window() {
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);

	bool created_main_window = false;

	if (!main_window) {
		main_window = SDL_CreateWindow(
			"PS1 Main",
			MAIN_WIDTH,
			MAIN_HEIGHT,
			SDL_WINDOW_RESIZABLE
		);
		created_main_window = true;
	}

	if (main_window && created_main_window) {
		main_window_closed = false;
		main_renderer = SDL_CreateRenderer(main_window, nullptr);
		main_texture = SDL_CreateTexture(
			main_renderer,
			SDL_PIXELFORMAT_RGBA8888,
			SDL_TEXTUREACCESS_STREAMING,
			MAIN_WIDTH,
			MAIN_HEIGHT
		);

		SDL_SetTextureScaleMode(
			main_texture,
			SDL_SCALEMODE_NEAREST
		);

	} else if (!main_window) {
		std::cerr << "Main window failed to open.\n";
		std::abort();
	}

	if constexpr (SHOW_VRAM_VIEWER) {
		bool created_vram_viewer = false;
		if (!vram_viewer) {
			vram_viewer = SDL_CreateWindow(
				"PS1 VRAM",
				VRAM_WIDTH,
				VRAM_HEIGHT,
				SDL_WINDOW_RESIZABLE
			);
			created_vram_viewer = true;
		}

		if (vram_viewer && created_vram_viewer) {
			vram_viewer_closed = false;
			vram_renderer = SDL_CreateRenderer(vram_viewer, nullptr);
			vram_texture = SDL_CreateTexture(
				vram_renderer,
				SDL_PIXELFORMAT_RGBA8888,
				SDL_TEXTUREACCESS_STREAMING,
				VRAM_WIDTH,
				VRAM_HEIGHT
			);

			SDL_SetTextureScaleMode(
				vram_texture,
				SDL_SCALEMODE_NEAREST
			);
		} else if (!vram_viewer) {
			std::cerr << "VRAM Viewer failed to open.\n";
			std::abort();
		}
	}
}

void GPU::sdl_loop() {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_EVENT_QUIT:
			case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
				std::cout << "WINDOW CLOSED\n";
				main_window_closed = true;
				vram_viewer_closed = true;
				break;
		}
	}

	if constexpr (SHOW_VRAM_VIEWER) {
		update_vram_viewer();
	}
}

void GPU::close_window() {
	if (main_texture) {
		SDL_DestroyTexture(main_texture);
		main_texture = nullptr;
	}
	if (main_renderer) {
		SDL_DestroyRenderer(main_renderer);
		main_renderer = nullptr;
	}
	if (main_window) {
		SDL_DestroyWindow(main_window);
		main_window = nullptr;
	}

	if (vram_texture) {
		SDL_DestroyTexture(vram_texture);
		vram_texture = nullptr;
	}
	if (vram_renderer) {
		SDL_DestroyRenderer(vram_renderer);
		vram_renderer = nullptr;
	}
	if (vram_viewer) {
		SDL_DestroyWindow(vram_viewer);
		vram_viewer = nullptr;
	}
}
