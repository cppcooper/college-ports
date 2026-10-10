#include <ctime>
#include <space_invaders/core/global.h>
#include <space_invaders/objects/enemies.h>
#include <GFX\gl_engine.h>


//These objects are written in stone
//For all instructional purposes

/*
//Each constructor sets the:
///////

//size

//sprite index

//Scale?
//width of sprite

//height of sprite

//count of this-> particular object type

// x offset of sprite image
// y offset of sprite image
// x position
// y position
*/

/*
// EACH destructor bothers to decrease the count of this-> particular object type
*/

unsigned int Archie::count = 0;
unsigned int Mal::count = 0;
unsigned int Kriege::count = 0;
unsigned int Lana::count = 0;
unsigned int Cheryl::count = 0;
unsigned int Cyril::count = 0;
unsigned int Ray::count = 0;
unsigned int Woodie::count = 0;

float height_woodie = 0.f;
float height_kriege = 0.f;
float height_ray = 0.f;
float height_cyril = 0.f;
float height_cheryl = 0.f;
float height_lana = 0.f;
float height_archie = 0.f;
float x_alignment = 0.f;

Woodie::Woodie()
{
	m_frameIndex = game_sprites::WOODIE;
	count++;
	SpriteFrame& frame = ( *s_sprite )[m_frameIndex];
	x_offset = frame.m_Offset.x;
	y_offset = frame.m_Offset.y;
	width = frame.m_Scale * frame.Width();
	height = frame.m_Scale * frame.Height();
	x = 50 + ( x_alignment * count );
	y = Screen::Height() - ( height * 3 );
	height_woodie = y;
	return;
}

Woodie::~Woodie()
{
	count--;
}

Kriege::Kriege()
{
	m_frameIndex = game_sprites::KRIEGE;
	count++;
	SpriteFrame& frame = ( *s_sprite )[m_frameIndex];
	x_offset = frame.m_Offset.x;
	y_offset = frame.m_Offset.y;
	width = frame.m_Scale * frame.Width();
	height = frame.m_Scale * frame.Height();
	x = 50 + ( x_alignment * count );
	y = height_woodie - height - 5;
	height_kriege = y;
}

Kriege::~Kriege()
{
	count--;
}

Ray::Ray()
{
	m_frameIndex = game_sprites::RAY;
	count++;
	SpriteFrame& frame = ( *s_sprite )[m_frameIndex];
	x_offset = frame.m_Offset.x;
	y_offset = frame.m_Offset.y;
	width = frame.m_Scale * frame.Width();
	height = frame.m_Scale * frame.Height();
	x = 50 + ( x_alignment * count );
	y = height_kriege - height - 5;
	height_ray = y;
}

Ray::~Ray()
{
	count--;
}

Cyril::Cyril()
{
	m_frameIndex = game_sprites::CYRIL;
	count++;
	SpriteFrame& frame = ( *s_sprite )[m_frameIndex];
	x_offset = frame.m_Offset.x;
	y_offset = frame.m_Offset.y;
	width = frame.m_Scale * frame.Width();
	height = frame.m_Scale * frame.Height();
	x = 50 + ( x_alignment * count );
	y = height_ray - height - 5;
	height_cyril = y;
}

Cyril::~Cyril()
{
	count--;
}

Cheryl::Cheryl()
{
	m_frameIndex = game_sprites::CHERYL;
	count++;
	SpriteFrame& frame = ( *s_sprite )[m_frameIndex];
	x_offset = frame.m_Offset.x;
	y_offset = frame.m_Offset.y;
	width = frame.m_Scale * frame.Width();
	height = frame.m_Scale * frame.Height();
	x = 50 + ( x_alignment * count );
	y = height_cyril - height - 5;
	height_cheryl = y;
}

Cheryl::~Cheryl()
{
	count--;
}

Lana::Lana()
{
	m_frameIndex = game_sprites::LANA;
	count++;
	SpriteFrame& frame = ( *s_sprite )[m_frameIndex];
	x_offset = frame.m_Offset.x;
	y_offset = frame.m_Offset.y;
	width = frame.m_Scale * frame.Width();
	height = frame.m_Scale * frame.Height();
	x = 50 + ( x_alignment * count );
	y = height_cheryl - height - 5;
	height_lana = y;
}

Lana::~Lana()
{
	count--;
}

Archie::Archie()
{
	m_frameIndex = game_sprites::ARCHIE;
	count++;
	SpriteFrame& frame = ( *s_sprite )[m_frameIndex];
	x_offset = frame.m_Offset.x;
	y_offset = frame.m_Offset.y;
	width = frame.m_Scale * frame.Width();
	height = frame.m_Scale * frame.Height();
	x = 50 + ( x_alignment * count );
	y = height_lana - height - 5;
	height_archie = y;
}

Archie::~Archie()
{
	count--;
}

Mal::Mal()
{
	m_frameIndex = game_sprites::MAL;
	count++;
	SpriteFrame& frame = ( *s_sprite )[m_frameIndex];
	x_offset = frame.m_Offset.x;
	y_offset = frame.m_Offset.y;
	width = frame.m_Scale * frame.Width();
	height = frame.m_Scale * frame.Height();
	x = 50 + ( x_alignment * count );
	y = height_archie - height - 5;
}

Mal::~Mal()
{
    	count--;
}
