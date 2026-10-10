#ifndef _INPUT_H_
#define _INPUT_H_

#include <atomic>
#include <thread>
#include <mutex>
#include <vector>
#include <map>
#include <functional>
#include <Windows.h>
#include <iostream>

struct Mouse_State
{
	std::mutex mutex;
	double cursor_x;
	double cursor_y;
	bool LeftMouseClick;
	bool RightMouseClick;
};

//This class manages the association between controls and game actions
class input_mgr
{
private:
	std::mutex vector_mutex;
	std::mutex map_mutex;
	std::thread P1;
	std::vector<int> input_list;
	std::map<int, std::function<void()>> Key_Action_Map;
	std::vector<std::function<void( double x, double y )>> Cursor_Readers;

	Mouse_State mouse_state;
	

protected:
	bool thread_running = false;
											//Invoke the function or method associated to the value of `key_input_value`
	inline void Run_Action(int key_input_value);

public:
	void Add_CursorReader( std::function<void( double x, double y )> Cursor_Reader );
											//Associate a key to a function/method
	void Register_Action(int key_input_value, std::function<void()> action);
											//Adds an input key to the input queue
	void QueueKey(int key_input_value);
											//Takes queued input and executes any function(s) associated to a given input
	void ProcessInput();

						//Spawns a thread to process input
	void Start();
						//Kills the thread processing input
	void Stop();
						//Not fully integrated
	void DoCursor( double x, double y );

	void Set_MouseClick( bool LeftClick );
};

#endif
