#include "gpu.hpp"

// For my own organisation, I am just commenting the section of the docs I have used
// The above never happened

// Command decoder
void GPU::receive_command(uint32_t value) {
	// GPU Memory Transfer Commands (Incomplete)

	switch (value >> 29) {
		case 0b100: gpu_state = GPUState::VRAM_VRAM_Source_Coord; return;
		case 0b101: gpu_state = GPUState::CPU_VRAM_Destination_Coord; return;
		case 0b110: gpu_state = GPUState::VRAM_CPU_Source_Coord; return;
	}

	// Rendering attribute commands (INCOMPLETE)
	switch (value >> 24) {
		case 0xE1:
			TBX = (value & 0xF) * 64;
			TBY = ((value >> 4) & 1) * 256;
			ABR = (value >> 5) & 3;
			TPF = (value >> 7) & 3;
			DTD = (value >> 9) & 1;
			DFE = (value >> 10) & 1;
			TBY2 = ((value >> 11) & 1) * 512;
			IX = (value >> 12) & 1;
			IY = (value >> 13) & 1;
			return;
		case 0xE2:
			texture_window_mask_x = value & 0x1F;
			texture_window_mask_y = (value >> 5) & 0x1F;
			texture_window_offset_x = (value >> 10) & 0x1F;
			texture_window_offset_y = (value >> 15) & 0x1F;
			return;
		case 0xE6: 
			PBW = (value & 1);
			PBC = (value & 2);
			return;

		// SPECIAL-CASING THESE AS THEY HAVE BIZARRE FIFO BEHAVIOUR
		case 0xE3:
			drawing_area_top_left_x = value & 0x1FF;
			drawing_area_top_left_y = (value >> 10) & 0x1FF;
			return;
		case 0xE4:
			drawing_area_bottom_right_x = value & 0x1FF;
			drawing_area_bottom_right_y = (value >> 10) & 0x1FF;
			return;
		case 0xE5:
			// SHOULD BE CORRECT, DOUBLE CHECK!!
			drawing_x_offset = static_cast<int16_t>((value & 0x7FF) | 0xF800);
			drawing_y_offset = static_cast<int16_t>(((value >> 11) & 0x7FF) | 0xF800);
			break;
	}

	switch (value >> 29) {
		case 3:
			receive_rectangle(value);
			break;
	}
}

void GPU::receive_rectangle(uint32_t value) {
	rectangle.r = value & 0xFF;
	rectangle.g = (value >> 8) & 0xFF;
	rectangle.b = (value >> 16) & 0xFF;
	rectangle.TGE  = (value >> 24) & 1;
	rectangle.ABE  = (value >> 25) & 1;
	rectangle.TME  = (value >> 26) & 1;
	rectangle.size = (value >> 27) & 3;
	gpu_state = GPUState::Rectangle_Vertex1;
}

void GPU::blit_cpu_vram(uint16_t pixel) {
	std::cout << "CPU-VRAM BLIT\n";
	if (blit.counter > 0) {
		if (PBW) {
			pixel |= 0x8000;
		}
		if (!PBC || !(vram[blit.y][blit.x] & 0x8000)) {
			vram[blit.y][blit.x] = pixel;
		}
	}

	blit.x++;
	if (blit.x == blit.dest.x + blit.size.width) {
		blit.y++;
	}
	if (blit.y == blit.dest.y + blit.size.height) {
		blit.y = blit.dest.y;
	}
	if (blit.counter > 0) {
		blit.counter--;
	} else {
		gpu_state = GPUState::Idle;
	}
}

uint16_t GPU::blit_vram_cpu() {
	std::cout << "VRAM-CPU BLIT\n";
	uint16_t data = 0x00;
	if (blit.counter > 0) {
		data = vram[blit.y][blit.x];
	}

	blit.x++;
	if (blit.x == blit.src.x + blit.size.width) {
		blit.y++;
	}
	if (blit.y == blit.src.y + blit.size.height) {
		blit.y = blit.src.y;
	}
	if (blit.counter > 0) {
		blit.counter--;
	} else {
		clear_gp1_bit(27);
		gpu_state = GPUState::Idle;
	}

	return data;
}

void GPU::blit_command(uint32_t value) {
	// INCOMPLETE
	switch (gpu_state) {

		// CPU-to-VRAM
		case GPUState::CPU_VRAM_Destination_Coord:
			blit.dest.x = (value & 0x3FF);
			blit.dest.y = ((value >> 16) & 0x1FF);
			blit.x = blit.dest.x;
			blit.y = blit.dest.y;
			gpu_state = GPUState::CPU_VRAM_Width_Height;
			break;
		case GPUState::CPU_VRAM_Width_Height:
			blit.size.width  = (value & 0x3FF);
			blit.size.height = ((value >> 16) & 0x1FF);
			blit.counter = blit.size.width * blit.size.height;
			gpu_state = GPUState::CPU_VRAM_Data;
			break;
		case GPUState::CPU_VRAM_Data:
			blit_cpu_vram(value & 0xFFFF);
			blit_cpu_vram(value >> 16);
			update_vram_viewer();
			break;

		case GPUState::VRAM_CPU_Source_Coord:
			set_gp1_bit(27);
			blit.src.x = (value & 0x3FF);
			blit.src.y = ((value >> 16) & 0x1FF);
			blit.x = blit.src.x;
			blit.y = blit.src.y;
			gpu_state = GPUState::VRAM_CPU_Width_Height;
			break;
		case GPUState::VRAM_CPU_Width_Height:
			blit.size.width  = (value & 0x3FF);
			blit.size.height = ((value >> 16) & 0x1FF);
			blit.counter = blit.size.width * blit.size.height;
			gpu_state = GPUState::VRAM_CPU_Data;
			break;
		case GPUState::VRAM_CPU_Data:
			// CARRIED OUT AS GP0 READS
			break;

		// VRAM-to-CPU

	}
}

// GP0

uint32_t GPU::gp0_read() {
	if (gp1_bypass) {
		gp1_bypass = false;
		return gp1_bypass_value;
	}
	switch (gpu_state) {
		case GPUState::VRAM_CPU_Data: {
			uint32_t data = (blit_vram_cpu() << 16);
			data |= blit_vram_cpu();
			return data;
			break;
		}
		default:
			return 0x00;
	}
}

void GPU::gp0_write(uint32_t value) {
	switch (gpu_state) {
		case GPUState::Idle:
			receive_command(value);
			break;

		// Memory transfer/blitting commands
		case GPUState::VRAM_VRAM_Source_Coord:
		case GPUState::VRAM_VRAM_Destination_Coord:
		case GPUState::VRAM_VRAM_Width_Height:
		case GPUState::CPU_VRAM_Destination_Coord:
		case GPUState::CPU_VRAM_Width_Height:
		case GPUState::CPU_VRAM_Data:
		case GPUState::VRAM_CPU_Source_Coord:
		case GPUState::VRAM_CPU_Width_Height:
		case GPUState::VRAM_CPU_Data:
			blit_command(value);
			break;
		case GPUState::Rectangle_Vertex1:
		case GPUState::Rectangle_UV:
		case GPUState::Rectangle_Width_Height:
			handle_rectangle(value);
			break;
	}
}

void GPU::handle_rectangle(uint32_t value) {
	switch (gpu_state) {
		case GPUState::Rectangle_Vertex1:
			rectangle.upper_left_x = value & 0xFFFF;
			rectangle.upper_left_y = (value >> 16);
			if (rectangle.TGE) {
				gpu_state = GPUState::Rectangle_UV;
			} else if (rectangle.size == 0) {
				gpu_state = GPUState::Rectangle_Width_Height;
			} else {
				draw_rectangle();
				gpu_state = GPUState::Idle;
			}
			break;
		case GPUState::Rectangle_UV:
			// INCOMPLETE, HANDLE UV HERE
			if (rectangle.size == 0) {
				gpu_state = GPUState::Rectangle_Width_Height;
			} else {
				draw_rectangle();
				gpu_state = GPUState::Idle;
			}
			break;
		case GPUState::Rectangle_Width_Height:
			rectangle.width  = value & 0x3FF;
			rectangle.height = (value >> 16) & 0x1FF;
			draw_rectangle();
			gpu_state = GPUState::Idle;
			break;
	}
}

// GP1

void GPU::set_gp1(uint32_t value) {
	gp1 = value;
}

void GPU::set_gp1_bit(int bit) {
	gp1 |= (1u << bit);
}

void GPU::clear_gp1_bit(int bit) {
	gp1 &= ~(1u << bit);
}

uint32_t GPU::gp1_read() {
	//std::cout << "GP1 READ\n";
	//return 0x5E800000; // MAGIC NUMBER (REMOVE WHEN THIS FEATURE HAS BEEN WRITTEN)
	return gp1;
}

void GPU::gp1_write(uint32_t value) {
	switch (value >> 24) {
		case 0x00: // NEED TO DO MORE HERE!
			set_gp1_bit(26); // HARDCODED (NOT CORRCT JUST YET)
			set_gp1_bit(28); // HARDCODED (NOT CORRECT JUST YET)
			break;
		case 0x01:
			gpu_state = GPUState::Idle;
			break;

		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
		case 0x14:
		case 0x15:
		case 0x16:
		case 0x17:
		case 0x18:
		case 0x19:
		case 0x1A:
		case 0x1B:
		case 0x1C:
		case 0x1D:
		case 0x1E:
		case 0x1F: {
			// INCOMPLETE
			// Using GPU version 2
			int register_index = (value & 0xFFFFFF) & 7;
			switch (register_index) {
			case 0:
			case 1:
			case 6:
				break;
			case 2: // INCOMPLETE
				break;
			case 3: // INCOMPLETE
				break;
			case 4: // INCOMPLETE
				break;
			case 5: // INCOMPLETE
				break;
			case 7:
				gp1_bypass_value = GPU_VERSION;
				break;
			case 8:
				gp1_bypass_value = 0x00000000;
				break;
			}
			gp1_bypass = true;
			break;
		}
	}

	//std::cout << "GP1 WRITE " << std::hex << std::setw(8) << std::setfill('0') << (int)value << "\n";
}