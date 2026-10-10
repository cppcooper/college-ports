#include <hypermaze/world/World.h>
#include <GFX\gl_engine.h>


World::~World()
{
	m_WorldRoot.OrphanSelf();
	delete m_HyperMaze;
}

void World::Init()
{
	m_HyperMaze = new HyperMaze();
	m_WorldRoot.SetDrawable( m_HyperMaze->GetTileMap() );
	m_HyperMaze->Init();
}

void World::Deinit()
{
	DetachFromScene();
	delete m_HyperMaze;
	m_HyperMaze = nullptr;
}

void World::AttachToScene( RootNode& root )
{
	root.Adopt( &m_WorldRoot );
}

void World::DetachFromScene()
{
	m_WorldRoot.OrphanSelf();
}

void World::SetViewPort( int x1, int y1, int x2, int y2 )
{
	m_ViewPort.Set( (float)x1, (float)y2, (float)( x2 - x1 ), (float)( y2 - y1 ) );
}

const ViewPort& World::GetViewPort() const
{
	return m_ViewPort;
}

bool World::CanMove( int x, int y )
{
	return m_HyperMaze->CanMove( x, y );
}
