#pragma warning( push )
#pragma warning( disable: 4005 )
#include <hypermaze/actors/Actors.h>
#include <hypermaze/actors/Player.h>
#include <hypermaze/world/World.h>
#include <GFX/gl_engine.h>
#pragma warning( pop )

using namespace GameAssets;

Player::Player()
{
	m_CharSprite = new Graphic();
	m_CharSprite->Load( "player.png" );
	m_ActorRoot.SetDrawable( m_CharSprite );
}

Player::~Player()
{
	delete m_CharSprite;
}

void Player::Make_Controller( Actors* A, World** W )
{
	delete m_Controller;
	m_Controller = new PlayerController( A, this, W );
}
