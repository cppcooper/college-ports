#include <hypermaze/actors/Actors.h>

void Actors::ChangeWorld( World* world )
{
	m_World = world;
	m_ActorsRoot.OrphanSelf();
	world->m_WorldRoot.Adopt( &m_ActorsRoot );
	m_Player.x = 1;
	m_Player.y = 1;
	m_Player.Make_Controller( this, &m_World );
}

void Actors::Init()
{
	TranNode* player_displacement = new TranNode( &m_ActorsRoot );
	uint32 width = Screen::Width() / 2;
	width -= width % m_World->m_HyperMaze->TileWidth();
	uint32 height = Screen::Height() / 2;
	height -= height % m_World->m_HyperMaze->TileHeight();
	player_displacement->Move( glm::vec3( width, height, 0.f ) );
	player_displacement->Adopt( &m_Player.m_ActorRoot );
	ControlModule& Controls = ControlModule::Instance();
	Controls.Bind_Key( GLFW_KEY_W, std::bind( &Player::Move_Up, &m_Player ) );
	Controls.Bind_Key( GLFW_KEY_S, std::bind( &Player::Move_Down, &m_Player ) );
	Controls.Bind_Key( GLFW_KEY_A, std::bind( &Player::Move_Left, &m_Player ) );
	Controls.Bind_Key( GLFW_KEY_D, std::bind( &Player::Move_Right, &m_Player ) );
}

void Actors::Deinit()
{
	m_ActorsRoot.OrphanSelf();
}
