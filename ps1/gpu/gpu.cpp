#include "gpu.hpp"

// For my own organisation, I am just commenting the section of the docs I have used
// The above never happened

// Command decoder
void GPU::receive_command(uint32_t value) {
	// GPU Memory Transfer Commands (Incomplete)

	switch (value >> 29) {
		case 0b001: receive_polygon(value); return;
		case 0b100: gpu_state = GPUState::VRAM_VRAM_Source_Coord; return;
		case 0b101: gpu_state = GPUState::CPU_VRAM_Destination_Coord; return;
		case 0b110: gpu_state = GPUState::VRAM_CPU_Source_Coord; return;
		case 0b011: receive_rectangle(value); return;
		default:
			std::cout << "(POSSIBLE) UNHANDLED TOP 3 BITS FOR GP0 COMMAND\n";
			break;
	}

	// Rendering attributgp1_read(e commands (INCOMPLETE)
	switch (value >> 24) {
		case 0xE1:
			TBX = (value & 0xF);
			TBY = ((value >> 4) & 1);
			ABR = (value >> 5) & 3;
			TPF = (value >> 7) & 3;
			DTD = (value >> 9) & 1;
			DFE = (value >> 10) & 1;
			TBY2 = ((value >> 11) & 1);
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
			drawing_area_top_left_x = value & 0x3FF;
			drawing_area_top_left_y = (value >> 10) & 0x3FF;
			return;
		case 0xE4:
			drawing_area_bottom_right_x = value & 0x3FF;
			drawing_area_bottom_right_y = (value >> 10) & 0x3FF;
			return;
		case 0xE5:
			// SHOULD BE CORRECT, DOUBLE CHECK!!
			drawing_x_offset = sign_extend_11(value & 0x7FF);
			drawing_y_offset = sign_extend_11((value >> 11) & 0x7FF);
			return;

		// Drawing commands (quick rectangle fill)
		case 0x02:
			receive_quick_rectangle(value);
			return;

		default:
			std::cout << "(POSSIBLE) UNHANDLED TOP 2 BYTES FOR GP0 COMMAND\n";
			return;
	}
}

void GPU::receive_polygon(uint32_t value) {
	polygon.r0 = value & 0xFF;
	polygon.g0 = (value >> 8) & 0xFF;
	polygon.b0 = (value >> 16) & 0xFF;
	polygon.TGE = get_bit(value, 24);
	polygon.ABE = get_bit(value, 25);
	polygon.TME = get_bit(value, 26);
	polygon.VTX = get_bit(value, 27);
	polygon.IIP = get_bit(value, 28);
	polygon.uv_passed = 0;
	polygon.vert_passed = 0;
	polygon.CLX = 0;
	polygon.CLY = 0;
	polygon.vertices.clear();

	polygon.max_verts = polygon.VTX ? 4 : 3;

	if (polygon.TME) {
		if (polygon.IIP) {
			polygon.format = PolygonFormat::CommandVertexUV_ColourVertexUV;
			gpu_state = GPUState::CommandVertexUV_ColourVertexUV_FirstVertex;
		} else {
			polygon.format = PolygonFormat::Command_VertexUV;
			gpu_state = GPUState::Command_VertexUV_Vertex;
		}
	} else {
		if (polygon.IIP) {
			polygon.format = PolygonFormat::CommandVertex_ColourVertex;
			gpu_state = GPUState::CommandVertex_ColourVertex_FirstVertex;
		} else {
			polygon.format = PolygonFormat::Command_Vertex;
			gpu_state = GPUState::Command_Vertex_Vertex;
		}
	}
}

void GPU::pass_clut(uint16_t clut) {
	// TO DO
}

void GPU::pass_page(uint16_t page) {
	// TO DO
}

void GPU::handle_polygon(uint32_t value) {
	switch (gpu_state) {
		// Format 1
		case GPUState::Command_Vertex_Vertex:
			polygon.v.x = sign_extend_11(value & 0xFFFF);
			polygon.v.y = sign_extend_11((value >> 16) & 0xFFFF);
			polygon.vertices.push_back(polygon.v);
			polygon.vert_passed++;
			if (polygon.vert_passed == polygon.max_verts) {
				polygon.render_type = polygon.TME ? PolygonRenderType::FlatModTEX : PolygonRenderType::FlatShadeNOTEX;
				draw_polygon();
				gpu_state = GPUState::Idle;
			}
			break;

		// Format 2
		case GPUState::CommandVertex_ColourVertex_FirstVertex:
			polygon.v.x = sign_extend_11(value & 0xFFFF);
			polygon.v.y = sign_extend_11((value >> 16) & 0xFFFF);
			polygon.v.r = polygon.r0;
			polygon.v.g = polygon.g0;
			polygon.v.b = polygon.b0;
			polygon.vertices.push_back(polygon.v);
			polygon.vert_passed++;
			gpu_state = GPUState::CommandVertex_ColourVertex_Colour;
			break;
		case GPUState::CommandVertex_ColourVertex_Colour:
			polygon.v.r = value & 0xFF;
			polygon.v.g = (value >> 8) & 0xFF;
			polygon.v.b = (value >> 16) & 0xFF;
			gpu_state = GPUState::CommandVertex_ColourVertex_Vertex;
			break;
		case GPUState::CommandVertex_ColourVertex_Vertex:
			polygon.v.x = sign_extend_11(value & 0xFFFF);
			polygon.v.y = sign_extend_11((value >> 16) & 0xFFFF);
			polygon.vertices.push_back(polygon.v);
			polygon.vert_passed++;
			if (polygon.vert_passed == polygon.max_verts) {
				polygon.render_type = polygon.TME ? PolygonRenderType::GourardModTEX : PolygonRenderType::GourardShadeNOTEX;
				draw_polygon();
				gpu_state = GPUState::Idle;
			} else {
				gpu_state = GPUState::CommandVertex_ColourVertex_Colour;
			}
			break;

		// Format 3
		case GPUState::Command_VertexUV_Vertex:
			polygon.v.x = sign_extend_11(value & 0xFFFF);
			polygon.v.y = sign_extend_11((value >> 16) & 0xFFFF);
			gpu_state = GPUState::Command_VertexUV_UV;
			break;
		case GPUState::Command_VertexUV_UV:
			if (polygon.uv_passed == 0) {
				pass_clut(value >> 16);
			}
			if (polygon.uv_passed == 1) {
				pass_page(value >> 16);
			}
			polygon.v.u = value & 0xFF;
			polygon.v.v = (value >> 8) & 0xFF;
			polygon.vertices.push_back(polygon.v);
			polygon.vert_passed++;
			polygon.uv_passed++;
			if (polygon.vert_passed == polygon.max_verts) {
				polygon.render_type = polygon.TME ? PolygonRenderType::RawTEX : PolygonRenderType::FlatShadeNOTEX;
				draw_polygon();
				gpu_state = GPUState::Idle;
			} else {
				gpu_state = GPUState::Command_VertexUV_Vertex;
			}
			break;

		// Format 4
		case GPUState::CommandVertexUV_ColourVertexUV_FirstVertex:
			polygon.v.x = sign_extend_11(value & 0xFFFF);
			polygon.v.y = sign_extend_11((value >> 16) & 0xFFFF);
			polygon.v.r = polygon.r0;
			polygon.v.g = polygon.g0;
			polygon.v.b = polygon.b0;
			gpu_state = GPUState::CommandVertexUV_ColourVertexUV_FirstUV;
			break;
		case GPUState::CommandVertexUV_ColourVertexUV_FirstUV:
			pass_clut(value >> 16);
			polygon.v.u = value & 0xFF;
			polygon.v.v = (value >> 8) & 0xFF;
			polygon.vertices.push_back(polygon.v);
			polygon.vert_passed++;
			polygon.uv_passed++;
			gpu_state = GPUState::CommandVertexUV_ColourVertexUV_Colour;
			break;
		case GPUState::CommandVertexUV_ColourVertexUV_Colour:
			polygon.v.r = value & 0xFF;
			polygon.v.g = (value >> 8) & 0xFF;
			polygon.v.b = (value >> 16) & 0xFF;
			gpu_state = GPUState::CommandVertexUV_ColourVertexUV_Vertex;
			break;
		case GPUState::CommandVertexUV_ColourVertexUV_Vertex:
			polygon.v.x = sign_extend_11(value & 0xFFFF);
			polygon.v.y = sign_extend_11((value >> 16) & 0xFFFF);
			gpu_state = GPUState::CommandVertexUV_ColourVertexUV_UV;
			break;
		case GPUState::CommandVertexUV_ColourVertexUV_UV:
			if (polygon.uv_passed == 1) {
				pass_page(value >> 16);
			}
			polygon.v.u = value & 0xFF;
			polygon.v.v = (value >> 8) & 0xFF;
			polygon.vertices.push_back(polygon.v);
			polygon.vert_passed++;
			polygon.uv_passed++;
			if (polygon.vert_passed == polygon.max_verts) {
				polygon.render_type = polygon.TME ? PolygonRenderType::RawTEX : PolygonRenderType::GourardShadeNOTEX;
				draw_polygon();
				gpu_state = GPUState::Idle;
			} else {
				gpu_state = GPUState::CommandVertexUV_ColourVertexUV_Colour;
			}
			break;

		default:
			std::cout << "UNEXPECTED POLYGON STATE\n";
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

void GPU::receive_quick_rectangle(uint32_t value) {
	quick_rectangle.r = value & 0xFF;
	quick_rectangle.g = (value >> 8) & 0xFF;
	quick_rectangle.b = (value >> 16) & 0xFF;
	gpu_state = GPUState::Quick_Rectangle_TopLeftCorner;
}

void GPU::blit_cpu_vram(uint16_t pixel) {
	//std::cout << "CPU-VRAM BLIT\n";
	if (blit.counter == 0) {
		return;
	}

	uint32_t px = blit.x & 0x3FF;
	uint32_t py = blit.y & 0x1FF;

	if (PBW) pixel |= 0x8000;
	if (!PBC || !(vram[py][px] & 0x8000)) {
		vram[py][px] = pixel;
	}

	blit.counter--;
	blit.x++;

	if (blit.x == blit.dest.x + blit.size.width) {
		blit.x = blit.dest.x;
		blit.y++;
	}
}

uint16_t GPU::blit_vram_cpu() {
	if (blit.counter == 0) {
		return 0;
	}

	uint16_t data = vram[blit.y & 0x1FF][blit.x & 0x3FF];

	blit.counter--;
	blit.x++;
	if (blit.x == blit.src.x + blit.size.width) {
		blit.x = blit.src.x;
		blit.y++;
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
			blit.size.width  = value & 0x3FF;
			blit.size.height = (value >> 16) & 0x1FF;
			blit.counter = blit.size.width * blit.size.height;
			gpu_state = GPUState::CPU_VRAM_Data;
			break;
		case GPUState::CPU_VRAM_Data:
			blit_cpu_vram(value & 0xFFFF);
			if (blit.counter > 0) {
				blit_cpu_vram(value >> 16);
			}
			if (blit.counter == 0) {
				std::cout << "CPU-VRAM\n";
				gpu_state = GPUState::Idle;
				update_vram_viewer();
			}
			break;

		case GPUState::VRAM_CPU_Source_Coord:
			blit.src.x = (value & 0x3FF);
			blit.src.y = ((value >> 16) & 0x1FF);
			blit.x = blit.src.x;
			blit.y = blit.src.y;
			gpu_state = GPUState::VRAM_CPU_Width_Height;
			break;
		case GPUState::VRAM_CPU_Width_Height:
			set_gp1_bit(27);
			blit.size.width  = ((value & 0x3FF) - 1 & 0x3FF) + 1;
			blit.size.height = (((value >> 16) & 0x1FF) - 1 & 0x1FF) + 1;
			blit.counter = blit.size.width * blit.size.height;
			gpu_state = GPUState::VRAM_CPU_Data;
			break;
		case GPUState::VRAM_CPU_Data:
			// CARRIED OUT AS GP0 READS
			break;

		case GPUState::VRAM_VRAM_Source_Coord:
			blit.src.x = value & 0x3FF;
			blit.src.y = (value >> 16) & 0x1FF;
			gpu_state = GPUState::VRAM_VRAM_Destination_Coord;
			break;
		case GPUState::VRAM_VRAM_Destination_Coord:
			blit.dest.x = value & 0x3FF;
			blit.dest.y = (value >> 16) & 0x1FF;
			gpu_state = GPUState::VRAM_VRAM_Width_Height;
			break;
		case GPUState::VRAM_VRAM_Width_Height: {
			std::cout << "VRAM-VRAM\n";
			int w = value & 0x3FF;
			int h = (value >> 16) & 0x1FF;
			for (int y = 0; y < h; y++) {
				for (int x = 0; x < w; x++) {
					vram[(blit.dest.y + y) & 0x1FF][(blit.dest.x + x) & 0x3FF] = vram[(blit.src.y + y) & 0x1FF][(blit.src.x + x) & 0x3FF];
				}
			}
			gpu_state = GPUState::Idle;
			break;
		}

		default:
			std::cout << "UNEXPECTED GPU STATE FOR BLIT COMMANDS\n";
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

	if (gpu_state != GPUState::VRAM_CPU_Data) {
		return 0x00;
	}
	
	uint32_t data = blit_vram_cpu();
	data |= (uint32_t)(blit_vram_cpu()) << 16;
	
	if (blit.counter == 0) {
		clear_gp1_bit(27);
		gpu_state = GPUState::Idle;
	}

	return data;
}

void GPU::gp0_write(uint32_t value) {
	//std::cout << "GP0 WRITE " << std::hex << (int)value << "\n";;
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
		case GPUState::Quick_Rectangle_TopLeftCorner:
		case GPUState::Quick_Rectangle_WidthHeight:
			handle_quick_rectangle(value);
			break;
		case GPUState::Command_Vertex_Vertex:
		case GPUState::CommandVertex_ColourVertex_FirstVertex:
		case GPUState::CommandVertex_ColourVertex_Colour:
		case GPUState::CommandVertex_ColourVertex_Vertex:
		case GPUState::Command_VertexUV_Vertex:
		case GPUState::Command_VertexUV_UV:
		case GPUState::CommandVertexUV_ColourVertexUV_FirstVertex:
		case GPUState::CommandVertexUV_ColourVertexUV_FirstUV:
		case GPUState::CommandVertexUV_ColourVertexUV_Colour:
		case GPUState::CommandVertexUV_ColourVertexUV_Vertex:
		case GPUState::CommandVertexUV_ColourVertexUV_UV:
			handle_polygon(value);
			break;
		default:
			std::cout << "UNEXPECTED GPU STATE\n";
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
		default:
			std::cout << "UNEXPECTED STATE (handle_rectangle)\n";
			break;
	}
}

void GPU::handle_quick_rectangle(uint32_t value) {
	switch (gpu_state) {
		case GPUState::Quick_Rectangle_TopLeftCorner:
			quick_rectangle.x = value & 0xFFFF;
			quick_rectangle.y = (value >> 16) & 0xFFFF;
			gpu_state = GPUState::Quick_Rectangle_WidthHeight;
			break;
		case GPUState::Quick_Rectangle_WidthHeight:
			quick_rectangle.width  = value & 0xFFFF;
			quick_rectangle.height = (value >> 16) & 0xFFFF;
			draw_quick_rectangle();
			gpu_state = GPUState::Idle;
			break;
	}
}

// GP1

void GPU::set_gp1(uint32_t value) {
	// TO DO
	TBX = value & 15;
	TBY = (value >> 4) & 1;
	ABR = (value >> 5) & 3;
	TPF = (value >> 7) & 3;
	DTD = (value >> 9) & 1;
	DFE = (value >> 10) & 1;
	PBW = (value >> 11) & 1;
	PBC = (value >> 12) & 1;
	//ODE2 = (value >> 13) & 1;
	REV = (value >> 14) & 1;
	TBY2 = (value >> 15) & 1;
	HDS2 = (value >> 16) & 1;
	HDS = (value >> 17) & 3;
	VDS = (value >> 19) & 1;
	NPB = (value >> 20) & 1;
	LBS = (value >> 21) & 1;
	IRS = (value >> 22) & 1;
	DMSK = (value >> 23) & 1;
	IRQ = (value >> 24) & 1;
	// DREQ
	IDLE = (value >> 26) & 1;
	RFFL = (value >> 27) & 1;
	WFEP = (value >> 28) & 1;
	DMD = (value >> 29) & 3;
	ODE = (value >> 31) & 1;
}

void GPU::set_gp1_bit(int bit) {
	gp1 |= (1u << bit);
}

void GPU::clear_gp1_bit(int bit) {
	gp1 &= ~(1u << bit);
}

bool GPU::DREQ() {
	switch (DMD) {
		case 0:
			return false;
		case 1:
			return true; // TEMPORARY SOLUTION
		case 2:
			return WFEP;
		case 3:
			return RFFL;
		default:
			std::cout << "UNEXPECTED DMD VALUE\n";
			return false;
	}
}

uint32_t GPU::gp1_read() {
	std::cout << "GP1 READ\n";
	return 0x5E800000; // MAGIC NUMBER (REMOVE WHEN THIS FEATURE HAS BEEN WRITTEN)
	/*gp1 =  (TBX << 0)   | (TBY << 4)  | (ABR  << 5)  | (TPF  << 7)  | (DTD  << 9)  | (DFE << 10)  |
	       (PBW << 11)  | (PBC << 12) | (~IRS << 13) | (REV  << 14) | (TBY2 << 15) | (HDS2 << 16) | (HDS << 17) | (VDS << 19) | (NPB << 20) | (LBS << 21) | (IRS << 22) |
	       (DMSK << 23) | (IRQ << 24) | (DREQ() << 25) | (IDLE << 26) | (RFFL << 27) | (WFEP << 28) | (DMD << 29) | (ODE << 31);
	std::cout << std::hex << std::setfill('0') << std::setw(8) << (int)gp1 << "\n";
	return gp1;*/
}

void GPU::gp1_write(uint32_t value) {
	std::cout << "GP1 WRITE\n" << std::hex << std::setfill('0') << std::setw(8) << (int)value << "\n";;
	switch (value >> 24) {
		case 0x00: // NEED TO DO MORE HERE!
			set_gp1(0x14802000);
			break;
		case 0x01:
			gpu_state = GPUState::Idle;
			break;
		case 0x02:
			IRQ = 0;
			break;
		case 0x03:
			DMSK = value & 1;
			break;
		case 0x04:
			DMD = value & 3;
			break;
		case 0x05:
			// INCOMPLETE
			break;
		case 0x06:
			// INCOMPLETE
			break;
		case 0x07:
			// INCOMPLETE
			break;
		case 0x08:
			HDS  = value & 3;
			VDS  = (value >> 2) & 1;
			NPB  = (value >> 3) & 1;
			LBS  = (value >> 4) & 1;
			IRS  = (value >> 5) & 1;
			HDS2 = (value >> 6) & 1;
			REV  = (value >> 7) & 1;
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
		default:
			std::cout << "NOT HANDLING GP1 WRITE\n";
			break;
	}

	//std::cout << "GP1 WRITE " << std::hex << std::setw(8) << std::setfill('0') << (int)value << "\n";
}