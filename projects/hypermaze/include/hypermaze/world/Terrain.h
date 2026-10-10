#ifndef TERRAIN_H
#define TERRAIN_H

#include <hypermaze/core/STL.h>
#include <GFX/OpenGL_Headers.h>

enum Tile {
	Blank = 0,
	Tile1 = 1,
	Tile2 = 2,
	Wall1 = 3,
	Wall2 = 4,
	TLCorner = 5,
	TRCorner = 6,
	BLCorner = 7,
	BRCorner = 8,
	Stairs = 9,
	DoorOpen = 10,
	DoorClosed = 11,
	GateClosed = 12,
	PathHorizontal = 13,
	PathVertical = 14,
};

struct Terrain
{
	uint8 width;
	uint8 height;
	std::vector<short> m_TileValues;
	bool inBounds( const glm::vec2& position ) const;
	bool canMove( const glm::vec2& position );
};

#endif
