#ifndef TILE_H
#define TILE_H

#include <rover/graphics/2d/sprite.h>
#include <mutex>

using uint32 = unsigned __int32;
#define regionWidth 4
#define regionHeight 4

class Region_Tile
{
private:
	uint32* Region;
	uint32 bitmask;
	__int8 bit_position;

public:
	Region_Tile( uint32* Region, __int8 index )
	{
		this->Region = Region;
		bit_position = index * 2;
		bitmask = 3 << bit_position;
	}
	//First two bits of assigned value are kept
	Region_Tile& operator=( __int8 tile_value )
	{
		*Region &= ~bitmask;
		*Region |= ( 3 & tile_value ) << bit_position;
		return *this;
	}
	__int8 Get_Tile_Value()
	{
		return ( ( *Region & bitmask ) >> bit_position );
	}
	bool canMove()
	{
		bitmask = 1 << ( bit_position + 1 );
		return bitmask & *Region;
	}
	bool haveBeenTo()
	{
		if ( Get_Tile_Value() == 3 )
		{
			return true;
		}
		return false;
	}
};

class Region
{
private:
	uint32 region_tiles = 2861864066;

public:
	Region_Tile Get_Tile( ushort x, ushort y );
	uint32& Region_Tiles()
	{
		return region_tiles;
	}
};

#endif
