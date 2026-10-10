#include <rover/core/global.h>
#include <rover/levels/viewport.h>

View_Port::View_Port( ushort screen_width, ushort screen_height )
{
	tile_set = get_game_engine().sprite_mgr.Get_Sprite( "tileset" );
	this->screen_width = screen_width;
	this->screen_height = screen_height;
}

View_Port::View_Port( Tile_Map* map, ushort screen_width, ushort screen_height )
{
	tile_set = get_game_engine().sprite_mgr.Get_Sprite( "tileset" );
	this->map = map;
	this->screen_width = screen_width;
	this->screen_height = screen_height;
}

void View_Port::Set_Map( Tile_Map* map )
{
	this->map = map;
	Validate_ViewPort();
}

bool View_Port::Set_Size( ushort width, ushort height )
{
	if ( width <= map->Get_Width() && height <= map->Get_Height() )
	{
		this->width = width;
		this->height = height;
		if ( Validate_ViewPort() )
		{
			return true;
		}
		else
		{
			assert( 0 );
		}
	}
	return false;
}

bool View_Port::Validate_ViewPort()
{
	ushort max_x = x_pos + width;
	ushort max_y = y_pos + height;
	if ( max_x < map->Get_Width() && max_y < map->Get_Height() && width != 0 && height != 0 )
	{
		tile_width = screen_width / width;
		tile_height = screen_height / height;
		return true;
	}
	return false;
}

bool View_Port::Scroll_Up( ushort distance )
{
	settings_lock.lock();
	y_pos += distance;
	if ( !Validate_ViewPort() )
	{
		y_pos -= distance;
		settings_lock.unlock();
		return false;
	}
	settings_lock.unlock();
	return true;
}

bool View_Port::Scroll_Left( ushort distance )
{
	settings_lock.lock();
	if ( x_pos != 0 )
	{
		x_pos -= distance;
		if ( !Validate_ViewPort() )
		{
			x_pos += distance;
			settings_lock.unlock();
			return false;
		}
	}
	settings_lock.unlock();
	return true;
}

bool View_Port::Scroll_Down( ushort distance )
{
	settings_lock.lock();
	if ( y_pos != 0 )
	{
		y_pos -= distance;
		if ( !Validate_ViewPort() )
		{
			y_pos += distance;
			settings_lock.unlock();
			return false;
		}
	}
	settings_lock.unlock();
	return true;
}

bool View_Port::Scroll_Right( ushort distance )
{
	settings_lock.lock();
	x_pos += distance;
	if ( !Validate_ViewPort() )
	{
		x_pos -= distance;
		settings_lock.unlock();
		return false;
	}
	settings_lock.unlock();
	return true;
}

Region_Tile View_Port::Get_Tile( ushort x, ushort y )
{
	settings_lock.lock();
	if ( x >= 0 && y >= 0 )
	{
		if ( x < width && y < height )
		{
			settings_lock.unlock();
			return map->Get_Tile( x + x_pos, y + y_pos );
		}
	}
	assert( 0 );
}

void View_Port::Scroll_Up()
{
	ushort distance = width / 5;
	while ( !Scroll_Up( distance ) )
	{
		distance -= 1;
	}
}

void View_Port::Scroll_Left()
{
	ushort distance = width / 5;
	while ( !Scroll_Left( distance ) )
	{
		distance -= 1;
	}
}

void View_Port::Scroll_Down()
{
	ushort distance = width / 5;
	while ( !Scroll_Down( distance ) )
	{
		distance -= 1;
	}
}

void View_Port::Scroll_Right()
{
	ushort distance = width / 5;
	while ( !Scroll_Right( distance ) )
	{
		distance -= 1;
	}
}

void View_Port::Draw()
{
	settings_lock.lock();
	sprite& Tile = *tile_set;

	for ( ushort y = y_pos; y < ( y_pos + height ); ++y )
	{
		for ( ushort x = x_pos; x < ( x_pos + width ); ++x )
		{
			tile_set->dest_x = ( x - x_pos ) * tile_width;
			tile_set->dest_y = ( y - y_pos ) * tile_height;
			switch ( map->Get_Tile( x, y ).Get_Tile_Value() )
			{
				case 0:
					//Rock
					Tile[2].Draw();
					break;

				case 1:
					//Hole
					Tile[1].Draw();
					break;

				case 2:
					//Dirt Not Visited
					Tile[0].Draw();
					break;

				case 3:
					//Dirt Visited
					Tile[3].Draw();
					break;
			}
		}
	}
	settings_lock.unlock();
}
