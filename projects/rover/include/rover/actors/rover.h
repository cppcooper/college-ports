#ifndef ROVER_H
#define ROVER_H

#include <rover/graphics/2d/sprite.h>
#include <rover/levels/tile_map.h>
#include <rover/levels/viewport.h>
#include <atomic>
#include <mutex>
#include <stack>
#include <deque>


#define EXAMPLE1

using ushort = unsigned short;

class Rover
{
private:
	Tile_Map* environment = nullptr;
	View_Port* view_port = nullptr;

	sprite* rover_sprite = nullptr;
	ushort sprite_frame = 0;
	std::mutex rover_lock;

	volatile unsigned int Recursion_Level = 0;
	volatile unsigned int Total_Moves = 0;
	bool stuck = false;
	ushort dest_x;
	ushort dest_y;

	std::mutex* Position_Lock;
	ushort current_x;
	ushort current_y;
	std::deque<std::pair<ushort, ushort>> path;
	std::stack<std::pair<ushort, ushort>> path_segment;

protected:
	bool Move( ushort start_x, ushort start_y );
	inline bool Move_Forward();
	__int8* Get_Search_Order();
	void Face_Dir( __int8 direction );

public:
	Rover( Tile_Map* map, View_Port* view_port );
	~Rover();
	void FindPath( ushort start_x, ushort start_y, ushort dest_x, ushort dest_y );
	void Update_ViewPort();
	void Draw();
};

#endif
