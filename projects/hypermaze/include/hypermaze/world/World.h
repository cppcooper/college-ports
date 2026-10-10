#ifndef WORLD_H
#define WORLD_H

#include <GF/GameFramework.h>
#include <hypermaze/world/HyperMaze.h>

class World : public GameModule
{
private:
	ViewPort m_ViewPort;

public:
	SceneNode m_WorldRoot;
	HyperMaze* m_HyperMaze = nullptr;
	~World();
	void Init() final override;
	void Deinit() final override;
	void AttachToScene( RootNode& root );
	void DetachFromScene();

	void SetViewPort( int x1, int y1, int x2, int y2 );
	const ViewPort& GetViewPort() const;
	inline bool CanMove( int x, int y );

#pragma region "Empty Overrides"
	void Process() final override {}
	void Update( double& seconds ) final override {}
	void PostProcess() final override {}
	void Buffer() final override {}
	void Draw() final override {}
#pragma endregion
};

#endif
