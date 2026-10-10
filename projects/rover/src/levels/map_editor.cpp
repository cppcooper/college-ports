#include <rover/levels/map_editor.h>
#include <fstream>

void Map_Editor::do_Input( double x, double y )
{
	edit_lock.lock();
	ushort tile_x, tile_y;

	tile_x = (ushort)x / view_port->Get_Tile_Width();
	tile_y = (ushort)y / view_port->Get_Tile_Height();
	if ( !paint_Region )
	{
		view_port->Get_Tile( tile_x, tile_y ) = tile_number;
	}
	else
	{
		for ( ushort ty = tile_y - Spray_Paint_Size; ty < tile_y + Spray_Paint_Size; ++ty )
		{
			for ( ushort tx = tile_x - Spray_Paint_Size; tx < tile_x + Spray_Paint_Size; ++tx )
			{
				if ( tx >= 0 && ty >= 0 )
				{
					if ( tx < view_port->Get_View_Width() && ty < view_port->Get_View_Height() )
					{
						view_port->Get_Tile( tx, ty ) = tile_number;
					}
				}
			}
		}
	}
	edit_lock.unlock();
}

void Map_Editor::Load( std::string file_name )
{
	Loaded_File_Name = file_name;
	map->Load( file_name );
}

void Map_Editor::Save( std::string file_name )
{
	std::ofstream file_handle;
	file_handle.open( file_name );
	if ( file_handle.is_open() )
	{
		try
		{
			file_handle << ( map->width / regionWidth ) << " ";
			file_handle << ( map->height / regionHeight ) << std::endl;

			int total_regions = ( ( map->width / regionWidth ) * ( map->height / regionHeight ) );
			for ( int i = 0; i < total_regions; ++i )
			{
				file_handle << (unsigned int)map->map[i].Region_Tiles() << " ";
			}

			if ( file_handle.fail() )
			{
				throw file_handle.rdstate();
			}
		}
		catch ( ... )
		{
			assert( 0 );
		}
	}
	else
	{
		assert( 0 );
	}
}

void Map_Editor::Save()
{
	if ( Loaded_File_Name.empty() )
	{
		Save( "Tile_Map" );
	}
	else
	{
		Save( Loaded_File_Name );
	}
}

void Map_Editor::Scroll_Up()
{
	view_port->Scroll_Up();
}

void Map_Editor::Scroll_Left()
{
	view_port->Scroll_Left();
}

void Map_Editor::Scroll_Down()
{
	view_port->Scroll_Down();
}

void Map_Editor::Scroll_Right()
{
	view_port->Scroll_Right();
}

void Map_Editor::set_TileNumber( ushort paint )
{
	edit_lock.lock();
	tile_number = paint;
	edit_lock.unlock();
}

bool Map_Editor::set_ViewPort( ushort width, ushort height )
{
	if ( view_port->Set_Size( width, height ) )
	{
		return true;
	}
	return false;
}

void Map_Editor::Draw()
{
	edit_lock.lock();
	view_port->Draw();
	edit_lock.unlock();
}
