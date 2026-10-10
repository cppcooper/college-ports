#include <word/systems/managers/input_mgr.h>

void input_mgr::Add_CursorReader( std::function<void( double x, double y )> Cursor_Reader )
{
	Cursor_Readers.push_back( Cursor_Reader );
}

//Invoke the function or method associated to the value of `key_input_value`
void input_mgr::Run_Action(int key_input_value)
{
	map_mutex.lock();
	auto iter = Key_Action_Map.find( key_input_value );
	if ( iter != Key_Action_Map.end() )
	{
		///Invoke the method or function
		iter->second();
	}
	map_mutex.unlock();
}

//Associate a key to a function/method
void input_mgr::Register_Action(int key_input_value, std::function<void()> action)
{
	map_mutex.lock();
	Key_Action_Map[key_input_value] = action;
	map_mutex.unlock();
}

//Adds an input key to the input queue
void input_mgr::QueueKey(int key_input_value)
{
	///We have to lock the queue incase this is in a separate thread from the processing of the queue
	vector_mutex.lock();

	///No more than 3 keys can be buffered - to avoid complications
	if ( input_list.size() < 4 )
	{
		auto iter = input_list.begin();

		///Search the queue for this key
		for ( ; iter != input_list.end(); ++iter )
		{
			if ( key_input_value == *iter )
			{
				break;
			}
		}

		///If we didn't find the key in the queue already
		if ( iter == input_list.end() )
		{
			///  then add it
			input_list.push_back( key_input_value );
		}
	}
	vector_mutex.unlock();
}

//Takes queued input and executes any function(s) associated to a given input
void input_mgr::ProcessInput()
{
	///We might not want this threaded ergo do{}while()
	do
	{
		///We lock our queue incase this is in a separate thread from where input is added
		vector_mutex.lock();

		///Process input queue
		for ( auto iter = input_list.begin(); iter != input_list.end(); )
		{
			///Run the action associated to this key (ie. `*iter`)
			Run_Action( *iter );

			///Remove key from queue and refresh iterator
			input_list.erase( iter );
			iter = input_list.begin();
		}

		///Delay control by 250ms
		std::this_thread::sleep_for( std::chrono::milliseconds( 250 ) );
		vector_mutex.unlock();
		
		///If we are in a thread and that thread isn't required to stop, then just loop again
	} while ( thread_running );
}

//Spawns a thread to process input
void input_mgr::Start()
{
	///We don't want to spawn more than one
	if ( !thread_running )
	{
		thread_running = true;
		P1 = std::thread( &input_mgr::ProcessInput, this );
	}
}

//Kills the thread processing input
void input_mgr::Stop()
{
	///We don't want to join a thread that isn't joinable, else bad things
	if ( P1.joinable() )
	{
		thread_running = false;
		P1.join();
	}
}

//Updates the mouse state
void input_mgr::DoCursor( double x, double y )
{
	mouse_state.mutex.lock();
	mouse_state.cursor_x = x;
	mouse_state.cursor_y = y;
	if ( mouse_state.LeftMouseClick )
	{
		mouse_state.mutex.unlock();
		for ( auto func : Cursor_Readers )
		{
			func( x, y );
		}
		return;
	}
	mouse_state.mutex.unlock();
}

void input_mgr::Set_MouseClick( bool LeftClick )
{
	mouse_state.mutex.lock();
	mouse_state.LeftMouseClick = LeftClick;
	if ( mouse_state.LeftMouseClick )
	{
		for ( auto func : Cursor_Readers )
		{
			func( mouse_state.cursor_x, mouse_state.cursor_y );
		}
	}
	mouse_state.mutex.unlock();
}
