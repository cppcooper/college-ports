#include <rover/actors/rover.h>
#include <rover/core/global.h>

#ifdef EXAMPLE1
#define PAUSE_INTERVAL 15
#define PAUSE_DURATION1 15
#define PAUSE_DURATION2 300
#define MAX_RECURSION 1500
#define SHORT_SEARCHES
#endif

#ifdef EXAMPLE2
#define PAUSE_INTERVAL 150
#define PAUSE_DURATION1 100
#define PAUSE_DURATION2 45
#define MAX_RECURSION 1500
#define SHORT_PATHS
#endif

#ifdef EXAMPLE3
#define PAUSE_INTERVAL 1
#define PAUSE_DURATION1 30
#define PAUSE_DURATION2 30
#define MAX_RECURSION 1500
#define SHORT_SEARCHES
#endif

#ifdef EXAMPLE4
#define PAUSE_INTERVAL 5000
#define PAUSE_DURATION1 1000
#define PAUSE_DURATION2 450
#define MAX_RECURSION 2
#define SHORT_PATHS
#endif




Rover::Rover( Tile_Map* map, View_Port* view_port )
{
	rover_sprite = get_game_engine().sprite_mgr.Get_Sprite( "rover" );
	Position_Lock = new std::mutex();
	environment = map;
	view_port->Set_Map( map );
	if ( !view_port->Set_Size( 100, 60 ) )
	{
		assert( 0 );
	}
	this->view_port = view_port;
}

Rover::~Rover()
{
	delete Position_Lock;
}

void Rover::FindPath( ushort start_x, ushort start_y, ushort dest_x, ushort dest_y )
{
	rover_lock.lock();
	this->dest_x = dest_x;
	this->dest_y = dest_y;
	rover_lock.unlock();
	current_x = start_x;
	current_y = start_y;
	Total_Moves = 0;

	//We we have invoked Rover::Move as many times as there are tiles on the map.. then we probably can't find a valid path
	while ( Total_Moves < ( environment->Get_Width() * environment->Get_Height() ) )
	{
		Recursion_Level = 0;
		if ( !path.empty() )
		{
			// This takes our search's starting coordinates off the path stack
			/// Thus allowing it to be added back on by the search algorithm when it unwinds
			path.pop_front();
		}

		// Perform Search
		Move( current_x, current_y );

		// If progress has been made transfer it to the FULL path thus far
		if ( !path_segment.empty() )
		{
			//Pop and push one coordinate at a time until there are no more
			do
			{
				path.push_front( path_segment.top() );
				path_segment.pop();
			} while ( !path_segment.empty() );
		}

		// We do need to check if we made it to the destination
		if ( current_x == dest_x && current_y == dest_y )
		{
			//Path is complete ( Start -> Destination )
			break;
		}

		// Check of the search algorithm became stuck in a dead end
		if ( stuck )
		{
			// Backtrack if we can still do so
			if ( !path.empty() )
			{
				current_x = path.front().first;
				current_y = path.front().second;
				// Try searching again
				continue;
			}
			else
			{
				//If we are stuck, and the path is empty.. then we have no options left to explore
				break;
			}
		}
	}
	Recursion_Level = 0;
	path;
}

// Algorithm Related ( Commented )
__int8* Rover::Get_Search_Order()
{
	static int static_stack_data;
	
	// Above we make a static variable here we turn it into an array of smaller variables
	__int8* search_order = (__int8*)( &static_stack_data );


	enum { Down = 0, Up = 2, Right = 1, Left = 3};

#ifdef SHORT_PATHS
	// Should we prioritize X or Y ( delta X > delta Y )
	if ( abs( current_x - dest_x ) > abs( current_y - dest_y ) )
	{
		search_order[0] = current_x > dest_x ? Left		: Right;		// X Towards Goal
		search_order[1] = current_y > dest_y ? Down	: Up;			// Y Towards Goal
		search_order[2] = current_x > dest_x ? Right	: Left;		// X Away from Goal
		search_order[3] = current_y > dest_y ? Up		: Down;	// Y Away from Goal
	}
	else
	{
		search_order[0] = current_y > dest_y ? Down	: Up;			// Y Towards Goal
		search_order[1] = current_x > dest_x ? Left		: Right;		// X Towards Goal
		search_order[2] = current_y > dest_y ? Up		: Down;	// Y Away from Goal
		search_order[3] = current_x > dest_x ? Right	: Left;		// X Away from Goal
	}
#else
#ifdef SHORT_SEARCHES
	// Should we prioritize X or Y ( delta X > delta Y )
	if ( abs( current_x - dest_x ) > abs( current_y - dest_y ) )
	{
		search_order[0] = current_x > dest_x ? Left		: Right;		// X Towards Goal
		search_order[1] = current_y > dest_y ? Down	: Up;			// Y Towards Goal
		search_order[2] = current_y > dest_y ? Up		: Down;	// Y Away from Goal
		search_order[3] = current_x > dest_x ? Right	: Left;		// X Away from Goal
	}
	else
	{
		search_order[0] = current_y > dest_y ? Down	: Up;			// Y Towards Goal
		search_order[1] = current_x > dest_x ? Left		: Right;		// X Towards Goal
		search_order[2] = current_x > dest_x ? Right	: Left;		// X Away from Goal
		search_order[3] = current_y > dest_y ? Up		: Down;	// Y Away from Goal
	}
#endif
#endif

	//Return the array we made
	return search_order;
}

void Rover::Face_Dir( __int8 direction )
{
#ifdef PAUSE_INTERVAL
	if ( Total_Moves % PAUSE_INTERVAL == 0 )
	{
		std::this_thread::sleep_for( std::chrono::milliseconds( PAUSE_DURATION1 ) );
	}
#endif
	rover_lock.lock();
	sprite_frame = direction % 4;
	rover_lock.unlock();
	
#ifdef PAUSE_INTERVAL
	if ( Total_Moves % PAUSE_INTERVAL == 0 )
	{
		std::this_thread::sleep_for( std::chrono::milliseconds( PAUSE_DURATION2 ) );
	}
#endif
}

// Algorithm Related ( Commented )
inline bool Rover::Move_Forward()
{
	__int8 offset[] = { 0, 0 };

	rover_lock.lock();

	// Calculate our movement offsets depending upon which direction we are facing
	switch ( sprite_frame )
	{
		case 0:
			//Facing Down
			offset[1] = -1;
			break;

		case 1:
			//Facing Right
			offset[0] = 1;
			break;

		case 2:
			//Facing Up
			offset[1] = 1;
			break;

		case 3:
			//Facing Left
			offset[0] = -1;
			break;
	}
	rover_lock.unlock();

	// We can't unlock our mutex before we would be finished using these variables, so we must copy the values before continuing
	Position_Lock->lock();
	ushort current_x = this->current_x;
	ushort current_y = this->current_y;
	Position_Lock->unlock();

	// If the coordinates are within the bounds of the map
	if ( ( offset[0] < 0 && current_x > 0 )
			|| ( offset[1] < 0 && current_y > 0 )
			|| ( offset[0] > 0 && current_x +1 < environment->Get_Width() )
			|| ( offset[1] > 0 && current_y +1 < environment->Get_Height() ) )
	{
		// If the coordinates are to somewhere traversable
		if ( environment->Get_Tile( current_x + offset[0], current_y + offset[1] ).canMove() )
		{
			// If we haven't previously been to these coordinates
			if ( !environment->Get_Tile( current_x + offset[0], current_y + offset[1] ).haveBeenTo() )
			{
				// Perform the move to these coordinates and continue searching
				if ( Move( current_x + offset[0], current_y + offset[1] ) )
				{
					//Stack needs to fully unwind for one of two reasons
					return true;
				}
			}
		}
	}
	return false;
}

// Algorithm Related ( Commented )
bool Rover::Move( ushort start_x, ushort start_y )
{
	++Total_Moves;
	++Recursion_Level;

	//Before we do anything, we set this tile as having been visited (ie. = 3)
	current_x = start_x;
	current_y = start_y;
	environment->Get_Tile( current_x, current_y ) = 3;

	//We need to know if this is our destination, so that we can stop
	if ( start_x == dest_x && start_y == dest_y )
	{
		//The recursion is going to start unwinding, so decrease the recursion counter
		--Recursion_Level;

		//We add the destination onto a stack of coordinates
		///Stack will represent the path from the starting position to the destination
		path_segment.push( std::make_pair( start_x, start_y ) );
		return true;
	}
	else if ( Recursion_Level <= MAX_RECURSION )
	{
		//Move outwards in search of destination
		for ( __int8 iteration = 0; iteration < 4; ++iteration )
		{
			//Block thread until current position is available to read & write
			Position_Lock->lock();
			current_x = start_x;
			current_y = start_y;
			//Unlock current position for the next thread to access
			Position_Lock->unlock();

			///Change direction of rover in accordance to the current search weights
			//( Up/Left/Right/Down vs Left/Up/Down/Right vs other permutations )
			Face_Dir( Get_Search_Order()[iteration] );

			//Move the rover forward if possible
			if ( Move_Forward() )
			{
				//The stack is unwinding because a valid path has been found or max recursion has been reached
				path_segment.push( std::make_pair( start_x, start_y ) );
				return true;
			}
		}
		//We've exhausted our search options time to unwind the stack til we find valid options to explore
		--Recursion_Level;
		stuck = true;
		return false;
	}
	//We've reached max recursion threshold time to have the stack fully unwind and try again from here
	stuck = false;
	path_segment.push( std::make_pair( start_x, start_y ) );
	return true;
}

void Rover::Update_ViewPort()
{
	Position_Lock->lock();
	ushort EX = view_port->Get_View_X() + view_port->Get_View_Width();
	ushort EY = view_port->Get_View_Y() + view_port->Get_View_Height();
	ushort i = 0;
	do
	{
		if ( ( current_x + 2 ) >= EX )
		{
			view_port->Scroll_Right();
			continue;
		}
		else if ( ( current_x - 2 ) <= view_port->Get_View_X() )
		{
			view_port->Scroll_Left();
			continue;
		}
		else if ( ( current_y + 2 ) >= EY )
		{
			view_port->Scroll_Up();
			continue;
		}
		else if ( ( current_y - 2 ) <= view_port->Get_View_Y() )
		{
			view_port->Scroll_Down();
			continue;
		}
		break;
	} while ( ++i <= 2 );
	Position_Lock->unlock();
}

void Rover::Draw()
{
	rover_lock.lock();
	Position_Lock->lock();

	rover_sprite->dest_x = ( current_x - view_port->Get_View_X() ) * view_port->Get_Tile_Width();
	rover_sprite->dest_y = ( current_y - view_port->Get_View_Y() ) * view_port->Get_Tile_Height();
	

	Position_Lock->unlock();

	( *rover_sprite )[sprite_frame].Draw();
	
	rover_lock.unlock();
	
	get_game_engine().sprite_mgr.Get_Sprite( "white_button" )->alpha = 0.7;
	get_game_engine().sprite_mgr.Get_Sprite( "white_button" )->Draw( 0, 0 );
	Fcout->set_properties( 50, 50, 28.f );
	Fcout->print( "Recursion Level:  " + std::to_string( Recursion_Level ) );
	Fcout->set_properties( 50, 30, 28.f);
	Fcout->print( "Total Moves:  " + std::to_string( Total_Moves ) );/**/
}
