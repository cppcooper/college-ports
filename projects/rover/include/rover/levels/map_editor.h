#ifndef MAP_EDITOR_H
#define MAP_EDITOR_H

#include <rover/levels/tile_map.h>
#include <rover/levels/viewport.h>
#include <mutex>

class Map_Editor
{
private:
	std::mutex edit_lock;
	Tile_Map* map = nullptr;
	ushort tile_number = 0;
	std::string Loaded_File_Name;

	View_Port* view_port;
	bool paint_Region = false;
	ushort Spray_Paint_Size = 2;

public:
	void do_Input( double x, double y );
	void Load( std::string file_name );
	void Save( std::string file_name );
	void Save();

	void Scroll_Up();
	void Scroll_Left();
	void Scroll_Right();
	void Scroll_Down();

	bool set_ViewPort( ushort width, ushort height );
	void set_TileNumber( ushort paint );
	void Draw();

	void SprayPaint_IncreaseSize()
	{
		if ( Spray_Paint_Size < 6 )
		{
			Spray_Paint_Size += 1;
		}
	}
	void SprayPaint_DecreaseSize()
	{
		if ( Spray_Paint_Size > 2 )
		{
			Spray_Paint_Size -= 1;
		}
	}
	void SprayPaint_Toggle()
	{
		paint_Region = !paint_Region;
	}

	Map_Editor( Tile_Map* map, ushort screen_width, ushort screen_height )
	{
		this->map = map;
		view_port = new View_Port( map, screen_width, screen_height );
	}
	~Map_Editor()
	{
		delete view_port;
	}
};

#endif
