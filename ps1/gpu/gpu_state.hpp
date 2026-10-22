#pragma once
#include <vector>

enum GPUState {
	Idle,

	// Rendering commands

	// Polygon
	Command_Vertex_Vertex,

	CommandVertex_ColourVertex_FirstVertex,
	CommandVertex_ColourVertex_Colour,
	CommandVertex_ColourVertex_Vertex,

	Command_VertexUV_Vertex,
	Command_VertexUV_UV,

	CommandVertexUV_ColourVertexUV_FirstVertex,
	CommandVertexUV_ColourVertexUV_FirstUV,
	CommandVertexUV_ColourVertexUV_Colour,
	CommandVertexUV_ColourVertexUV_Vertex,
	CommandVertexUV_ColourVertexUV_UV,

	// Rectangle
	Rectangle_Vertex1,
	Rectangle_UV,
	Rectangle_Width_Height,

	// Quick Rectangle
	Quick_Rectangle_TopLeftCorner,
	Quick_Rectangle_WidthHeight,

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

struct Rectangle {
	uint8_t r, g, b;
	bool TGE, ABE, TME;
	uint8_t size;

	uint16_t width, height;

	uint16_t upper_left_x, upper_left_y;

	// TEXTURE STUFF NOT YET ADDED
};

struct Vertex {
	int16_t x, y;
	uint8_t r, g, b;
	uint16_t u, v;
};

enum PolygonFormat {
	NoFormat,
	Command_Vertex, // First take Command, then Vertex (3 or 4 times based on VTX)
	CommandVertex_ColourVertex, // First take Command, then Vertex, then Colour and Vertex (another 2 or 3 times based on VTX)
	Command_VertexUV, // First take Command, then Vertex and UV (3 or 4 times based on VTX)
	CommandVertexUV_ColourVertexUV, // First take Command Vertex UV, then Colour Vertex UV (another 2 or 3 times based on VTX)
};

enum PolygonRenderType {
	NoRenderType,
	FlatShadeNOTEX,
	GourardShadeNOTEX,
	FlatModTEX,
	GourardModTEX,
	RawTEX
};

struct Polygon {
	uint8_t r0, g0, b0;
	bool TGE, ABE, TME, VTX, IIP;
	PolygonFormat format = PolygonFormat::NoFormat;
	PolygonRenderType render_type = PolygonRenderType::NoRenderType;

	uint8_t uv_passed = 0;
	uint8_t vert_passed = 0;
	uint8_t max_verts = 0;

	// CLUT (updated in first UV word, most significant half-word)
	uint16_t CLX = 0;
	uint16_t CLY = 0;

	// TPage (updated in second UV word, most significant half-word)

	Vertex v;
	std::vector<Vertex> vertices;
};

struct QuickRectangle {
	uint8_t r, g, b;
	uint16_t x, y; // top left corner
	uint16_t width, height;
};

struct Weights {
	float a, b, c;
};