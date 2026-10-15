//This file is where a lot of the gameplay mechanics happen

#include <../../include/space_invaders/SpaceInvaders.h>
#include <space_invaders/core/global.h>
#include <space_invaders/objects/Play.h>

player::player()
{
	x = Screen::Width() / 2;
	y = 100;
	m_frameIndex = game_sprites::YOU;
	SpriteFrame& frame = ( *s_sprite )[m_frameIndex];
	width = frame.Width() * frame.m_Scale;
	height = frame.Height() * frame.m_Scale;
}

void player::Death()
{
	Sync();
	alive = false;
	OldSpaceInvaders::Instance().Player_Death();
}

void player::unDeath()
{
	alive = true;
}

void player::MoveLeft( double& seconds )
{
	// x > 50 ? true value : false value
	x = x > 100 ? x - ( seconds * xincrements ) : 50;
}

void player::MoveRight( double& seconds )
{
	// x < right edge ? true value : false value
	x = x < ( Screen::Width() - 100 ) ? x + ( seconds * xincrements ) : ( Screen::Width() - 50 );
}

void player::Draw()
{
	static unsigned int sprite_curIndex = 0;
	float thisx = x;
	float thisy = y;
	m_modelMatrix = glm::translate( glm::mat4( 1.f ), glm::vec3( thisx, thisy, 0.f ) );

	sprite_curIndex = alive ? m_frameIndex : m_frameIndex + 1;

	( *s_sprite )[sprite_curIndex].Draw( m_modelMatrix );
}
