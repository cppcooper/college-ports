#include <../../include/space_invaders/SpaceInvaders.h>
#include <space_invaders/core/global.h>
#include <space_invaders/objects/screen_objs.h>
#include <space_invaders/systems/screen_mgr.h>

Sprite* ScreenObject::s_sprite = nullptr;
Texture* ScreenObject::s_texture = nullptr;
GLSLProgram* ScreenObject::s_shader = nullptr;
unsigned int ScreenObject::reference_count = 0;

ScreenObject::ScreenObject()
{
	reference_count++;
	if ( !s_sprite )
	{
		Asset_Faculties& gf = Asset_Faculties::Instance();
		s_sprite = gf.GetAsset<Sprite>( invaders_sprite );
	}
}

ScreenObject::~ScreenObject()
{
	if ( --reference_count <= 0 )
	{
		//delete sprite;
		//delete texture;
		//delete shader;
	}
}

void ScreenObject::Draw()
{
	float thisx = x;
	float thisy = y;
	m_modelMatrix = glm::translate( glm::mat4( 1.f ), glm::vec3( thisx, thisy, 0.f ) );

	( *s_sprite )[m_frameIndex].Draw( m_modelMatrix );
}

void Enemy::Update(double& seconds)
{	
	elapsed = elapsed + seconds;
	if ( deleteme )
	{
		return void( 0 );
	}
	if ( elapsed >= 1.0 )
	{
		elapsed = 0;
		flipped = !flipped;
#ifdef DEATH_TEST
		if ( death_test == 0 )
			death_test = last;
		else if ( ( current - death_test ) >= 5 )
			Death();
#endif
	}
}

void Enemy::Draw()
{
	static unsigned int sprite_curIndex = 0;
	float thisx = x;
	float thisy = y;
	m_modelMatrix = glm::translate( glm::mat4( 1.f ), glm::vec3( thisx, thisy, 0.f ) );
	
	if ( m_frameIndex != game_sprites::WOODIE )
		sprite_curIndex = dead ? game_sprites::DEATH : m_frameIndex + ( flipped ? 1 : 0 );
	else
		sprite_curIndex = dead ? game_sprites::DEATH : m_frameIndex;
	
	( *s_sprite )[sprite_curIndex].Draw( m_modelMatrix );
}

void Enemy::Death()
{
	if ( !dead )
	{
		dead = true;
		x = x - x_offset;
		OldSpaceInvaders::Instance().Increase_Score( m_frameIndex * 100 );
		t1 = std::thread( &Enemy::set_deletion, this );
	}
}

void Enemy::set_deletion()
{
	std::this_thread::sleep_for( std::chrono::milliseconds( 250 ) );
	deleteme = true;
}

void Enemy::Sync()
{
	if ( t1.joinable() )
	{
		t1.join();
	}
}
