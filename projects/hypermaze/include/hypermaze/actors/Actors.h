#ifndef _ACTORS_H_
#define _ACTORS_H_
#pragma once
#include <GF/GameFramework.h>
#include <hypermaze/actors/Player.h>
#include <hypermaze/world/World.h>

class Actors : public GameModule
{
private:
	Player m_Player;
	SceneNode m_ActorsRoot;
	World* m_World = nullptr;

public:
	void Init() final override;
	void Deinit() final override;

	void ChangeWorld( World* world );

#pragma region "Empty Overrides"
	void Process() final override {}
	void Update( double& seconds ) final override {}
	void PostProcess() final override {}
	void Buffer() final override {}
	void Draw() final override {}
#pragma endregion
};

#endif
