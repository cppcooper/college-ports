#ifndef _PLAYER_H_
#define _PLAYER_H_
#pragma once
#include <hypermaze/actors/ActorBase.h>
#include <GF/game_faculties.h>

class Player : public ActorBase
{
	GameAssets::Graphic* m_CharSprite = nullptr;
public:
	Player();
	~Player();
	void Make_Controller( Actors* A, World** W ) final override;
};

#endif
