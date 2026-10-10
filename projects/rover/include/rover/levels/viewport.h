#ifndef VIEWPORT_H
#define VIEWPORT_H

#include <rover/levels/tile_map.h>
#include <assert.h>
#include <mutex>

using ushort = unsigned short;

class View_Port
{
private:
	sprite* tile_set = nullptr;
	Tile_Map* map = nullptr;

	std::mutex settings_lock;

	ushort x_pos = 0;
	ushort y_pos = 0;
	ushort width = 0; //View Port width in Tiles
	ushort height = 0; //View Port height in Tiles
	ushort tile_width = 0;
	ushort tile_height = 0;
	ushort screen_width = 0;
	ushort screen_height = 0;

protected:
	bool Validate_ViewPort();
	bool Scroll_Up( ushort distance );
	bool Scroll_Left( ushort distance );
	bool Scroll_Down( ushort distance );
	bool Scroll_Right( ushort distance );

public:
	void Set_Map( Tile_Map* map );
	bool Set_Size( ushort width, ushort height );
	Region_Tile Get_Tile( ushort x, ushort y );
	void Scroll_Up();
	void Scroll_Left();
	void Scroll_Down();
	void Scroll_Right();
	void Draw();

	View_Port( ushort screen_width, ushort screen_height );
	View_Port( Tile_Map* map, ushort screen_width, ushort screen_height );

	ushort Get_View_X() const
	{
		return x_pos;
	}
	ushort Get_View_Y() const
	{
		return y_pos;
	}
	ushort Get_View_Width() const
	{
		return width;
	}
	ushort Get_View_Height() const
	{
		return height;
	}
	ushort Get_Tile_Width() const
	{
		return tile_width;
	}
	ushort Get_Tile_Height() const
	{
		return tile_height;
	}
};

#endif
