#ifndef ACTORCONTROLLER_H
#define ACTORCONTROLLER_H
#pragma once

#include <hypermaze/core/STL.h>

class ActorBase;
class Actors;
class World;

class ActorController
{
protected:
	World** m_World = nullptr;
	ActorBase* m_Actor = nullptr;
	Actors* m_Actors = nullptr;

public:
	ActorController( Actors* A, ActorBase* a, World** W );
	virtual bool Move_Up( uint32& x, uint32& y );
	virtual bool Move_Right( uint32& x, uint32& y );
	virtual bool Move_Down( uint32& x, uint32& y );
	virtual bool Move_Left( uint32& x, uint32& y );
};


class PlayerController : public ActorController
{
private:
	std::mutex m_InputLock;
public:
	PlayerController( Actors* A, ActorBase* a, World** W );
	virtual bool Move_Up( uint32& x, uint32& y ) final override;
	virtual bool Move_Right( uint32& x, uint32& y ) final override;
	virtual bool Move_Down( uint32& x, uint32& y ) final override;
	virtual bool Move_Left( uint32& x, uint32& y ) final override;
};

#endif
