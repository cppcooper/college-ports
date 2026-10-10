#ifndef TILE_MAP_H
#define TILE_MAP_H

#include <rover/levels/region.h>
#include <mutex>

class Tile_Map
{
	friend class Map_Editor;
private:
	std::mutex map_lock;
	Region* map = nullptr;
	ushort width = 0; //Width of map in Tiles
	ushort height = 0; //Height of map in Tiles

public:
	Region_Tile Get_Tile( ushort x, ushort y );
	void Load( std::string file_name );

	ushort Get_Width() const
	{
		return width;
	}
	ushort Get_Height() const
	{
		return height;
	}

	~Tile_Map()
	{
		delete[] map;
	}
	Tile_Map(){}
	Tile_Map( ushort width, ushort height )
	{
		//Division and then multiplication appears unnecessary but is not
		///We divide to an integer, to ensure our size is valid
		width /= regionWidth;
		height /= regionHeight;
		this->width = width * regionWidth;
		this->height = height * regionHeight;
		delete map;
		map = new Region[width * height];
	}
};


#endif
