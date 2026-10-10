#include <hypermaze/actors/ActorController.h>
#include <hypermaze/actors/Actors.h>
#include <hypermaze/core/Debugger.h>
#include <hypermaze/world/World.h>
#include <iostream>

inline int Move( HyperMaze* hmaze, uint32 &x, uint32 &y, const glm::vec2& dir )
{
	assert( hmaze != nullptr );
	HyperMaze& h = *hmaze;
	LogStream LogLine = LogFile_Manager::Get( 0, _DEBUG2 );
	LogLine << "Move().."
		<< newl2 << "Direction: " << dir.x << ", " << dir.y
		<< newl2 << "Position: " << x << ", " << y;
	if ( h.CanMove( x + (int8)dir.x, y + (int8)dir.y ) )
	{
		x += (int8)dir.x;
		y += (int8)dir.y;
		LogLine << newl2 << "Can Move - Return Value: " << 1;
		return 1;
	}
	else if ( !h.InBounds( x + (int8)dir.x, y + (int8)dir.y ) )
	{
		x += (int8)dir.x;
		y += (int8)dir.y;
		LogLine << newl2 << "Out Of Bounds - Return Value: " << 2;
		return 2;
	}
	LogLine << newl2 << "In Bounds and Can't Move - Return Value: " << 0;
	return 0;
}

ActorController::ActorController( Actors* A, ActorBase* a, World** W )
{
	assert( A != nullptr );
	assert( a != nullptr );
	assert( W != nullptr );
	m_World = W;
	m_Actor = a;
	m_Actors = A;
}

bool ActorController::Move_Up( uint32 &x, uint32 &y )
{
	assert( m_World );
	World& w = **m_World;
	const glm::vec2 dir( 0, 1 );
	return 0 != Move( w.m_HyperMaze, x, y, dir );
}

bool ActorController::Move_Right( uint32 &x, uint32 &y )
{
	assert( m_World );
	World& w = **m_World;
	const glm::vec2 dir( 1, 0 );
	return 0 != Move( w.m_HyperMaze, x, y, dir );
}

bool ActorController::Move_Down( uint32 &x, uint32 &y )
{
	assert( m_World );
	World& w = **m_World;
	const glm::vec2 dir( 0, -1 );
	return 0 != Move( w.m_HyperMaze, x, y, dir );
}

bool ActorController::Move_Left( uint32 &x, uint32 &y )
{
	assert( m_World );
	World& w = **m_World;
	const glm::vec2 dir( -1, 0 );
	return 0 != Move( w.m_HyperMaze, x, y, dir );
}


PlayerController::PlayerController( Actors* A, ActorBase* a, World** W )
	: ActorController( A, a, W )
{}

bool PlayerController::Move_Up( uint32 &x, uint32 &y )
{
	assert( m_World );
	World& w = **m_World;
	const glm::vec2 dir( 0, 1 );
	m_InputLock.lock();
	int moved = Move( w.m_HyperMaze, x, y, dir );
	if ( moved )
	{
		debugger::Instance().Player_X = x;
		debugger::Instance().Player_Y = y;
		if ( moved == 2 )
		{
			w.m_HyperMaze->Traverse( x, y, Graph::GD_U );
		}
		w.m_HyperMaze->Scroll( dir );
	}
	m_InputLock.unlock();
	return moved == 0 ? false : true;
}

bool PlayerController::Move_Right( uint32 &x, uint32 &y )
{
	assert( m_World );
	World& w = **m_World;
	const glm::vec2 dir( 1, 0 );
	m_InputLock.lock();
	int moved = Move( w.m_HyperMaze, x, y, dir );
	if ( moved )
	{
		debugger::Instance().Player_X = x;
		debugger::Instance().Player_Y = y;
		if ( moved == 2 )
		{
			w.m_HyperMaze->Traverse( x, y, Graph::GD_R );
		}
		w.m_HyperMaze->Scroll( dir );
	}
	m_InputLock.unlock();
	return moved == 0 ? false : true;
}

bool PlayerController::Move_Down( uint32 &x, uint32 &y )
{
	assert( m_World );
	World& w = **m_World;
	const glm::vec2 dir( 0, -1 );
	m_InputLock.lock();
	int moved = Move( w.m_HyperMaze, x, y, dir );
	if ( moved )
	{
		debugger::Instance().Player_X = x;
		debugger::Instance().Player_Y = y;
		if ( moved == 2 )
		{
			w.m_HyperMaze->Traverse( x, y, Graph::GD_D );
		}
		w.m_HyperMaze->Scroll( dir );
	}
	m_InputLock.unlock();
	return moved == 0 ? false : true;
}

bool PlayerController::Move_Left( uint32 &x, uint32 &y )
{
	assert( m_World );
	World& w = **m_World;
	const glm::vec2 dir( -1, 0 );
	m_InputLock.lock();
	int moved = Move( w.m_HyperMaze, x, y, dir );
	if ( moved )
	{
		debugger::Instance().Player_X = x;
		debugger::Instance().Player_Y = y;
		if ( moved == 2 )
		{
			w.m_HyperMaze->Traverse( x, y, Graph::GD_L );
		}
		w.m_HyperMaze->Scroll( dir );
	}
	m_InputLock.unlock();
	return moved == 0 ? false : true;
}
