#ifndef _SCREEN_OBJ_H_
#define _SCREEN_OBJ_H_

//#define DEATH_TEST

#include <GF\game_faculties.h>
using namespace GameAssets;
#include <space_invaders/core/STL.h>
#include <space_invaders/objects/phys_obj.h>

enum game_sprites {
	MAL = 0,
	ARCHIE = 2,
	LANA = 4,
	CHERYL = 6,
	CYRIL = 8,
	RAY = 10,
	KRIEGE = 12,
	WOODIE = 14,
	DEATH = 15,
	PEW = 16,
	YOU = 17
};
//enum game_sprites { ARCHIE = 0, MAL = 2, KRIEGE = 4, LANA = 6, CHERYL = 8, CYRIL = 10, RAY = 12, WOODIE = 14, DEATH = 16, PEW = 17, YOU = 18 };

class ScreenObject : protected phys_box
{
protected:
	static unsigned int reference_count;
	static Sprite* s_sprite;
	static Texture* s_texture;
	static GLSLProgram* s_shader;
	unsigned int m_frameIndex;

	float x_offset = 0;
	float y_offset = 0;
	glm::mat4 m_modelMatrix;

public:
	ScreenObject();
	virtual ~ScreenObject();

	bool isHidden() const { return deleteme; }
	virtual void deleteSelf() = 0;
	virtual void Update(double& seconds){}
	virtual void Draw();
	virtual void Sync() = 0;
	unsigned int getTYPEID() const { return m_frameIndex; } //shh don't tell anybody
};

class Enemy : public ScreenObject
{
private:
	bool flipped = false;
	bool dead = false;
	double elapsed = 0;
#ifdef DEATH_TEST
	time_t death_test = 0;
#endif

	std::thread t1;
	void set_deletion();

public:
	Enemy(){}
	virtual ~Enemy(){}

	void deleteSelf() override { delete this; }
	void Update(double& seconds);
	void Draw();
	void Death();
	void Sync();
};

#endif
