#ifndef _ACTORBASE_H_
#define _ACTORBASE_H_
#pragma once

#include <assert.h>
#include <GF/Game_Faculties/AssetTypes/SceneNodes.h>
#include <hypermaze/actors/ActorController.h>

class Actors;
class World;

class ActorBase
{
	friend class Actors;
	friend class ActorController;
	friend class PlayerController;
protected:
	ActorController* m_Controller = nullptr;
	ActorBase* m_TargetActor = nullptr;
	uint32 x = 0;
	uint32 y = 0;

public:
	SceneNode m_ActorRoot;
	virtual ~ActorBase(){
		m_ActorRoot.OrphanSelf();
		delete m_Controller;
	}
	virtual void Make_Controller( Actors* A, World** W ){
		delete m_Controller;
		m_Controller = new ActorController( A, this, W );
	}
	void Move_Up(){
		assert( m_Controller != nullptr ); m_Controller->Move_Up( x, y );
	}
	void Move_Right(){
		assert( m_Controller != nullptr ); m_Controller->Move_Right( x, y );
	}
	void Move_Down(){
		assert( m_Controller != nullptr ); m_Controller->Move_Down( x, y );
	}
	void Move_Left(){
		assert( m_Controller != nullptr ); m_Controller->Move_Left( x, y );
	}
};

#endif
