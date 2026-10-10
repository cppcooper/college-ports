#include <rover/levels/tile_map.h>
#include <fstream>

Region_Tile Tile_Map::Get_Tile(ushort x, ushort y)
{
	map_lock.lock();
	if ( ( x < width ) && ( y < height ) )
	{
		unsigned int index = ( ( y / regionHeight ) * ( width / regionWidth ) ) + ( x / regionWidth );
		map_lock.unlock();
		return map[index].Get_Tile( x % regionWidth, y % regionHeight );
	}
	assert( 0 );
}

void Tile_Map::Load( std::string file_name )
{
	std::ifstream file_handle;
	file_handle.open( file_name );
	if ( file_handle.is_open() )
	{
		try
		{
			file_handle >> width;
			file_handle >> height;

			delete map;
			map = new Region[this->width * this->height];

			for ( int i = 0; i < ( width * height ); ++i )
			{
				file_handle >> (unsigned int)map[i].Region_Tiles();
			}

			if ( file_handle.fail() )
			{
				throw file_handle.rdstate();
			}

			width *= regionWidth;
			height *= regionHeight;
			
		}
		catch ( ... )
		{
			delete map;
			assert( 0 );
		}
	}
}
