#include "gpu.hpp"

void GPU::update_vram_viewer() {

}

void GPU::create_window() {
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);

	main_window = SDL_CreateWindow(
		"PS1 Main",
		MAIN_WIDTH,
		MAIN_HEIGHT,
		0
	);

	if (main_window) {
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

	} else {
		std::cerr << "Main window failed to open.\n";
		std::abort();
	}

	if constexpr (SHOW_VRAM_VIEWER) {
		vram_viewer = SDL_CreateWindow(
			"PS1 VRAM",
			VRAM_WIDTH,
			VRAM_HEIGHT,
			0
		);

		if (vram_viewer) {
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
		} else {
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
				main_window_closed = false;
				vram_viewer_closed = false;
				break;
		}
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
