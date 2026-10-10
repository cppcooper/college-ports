#ifndef _PLAY_H_
#define _PLAY_H_

#include <space_invaders/core/STL.h>
#include <space_invaders/objects/screen_objs.h>
#include <space_invaders/systems/invasion_mgr.h>

class bullet;

class player : public ScreenObject
{
private:
	float xincrements = 200;
	bool alive = true;

protected:
public:
	player();
	virtual ~player()
	{
		if ( playerbullet )
		{
			delete playerbullet;
		}
	}

	void deleteSelf() final override { delete this; }
	void Death(); 
	void unDeath();
	float GetX() const { return x; }
	float GetY() const { return y; }
	void MoveLeft( double& seconds );
	void MoveRight( double& seconds );
	void Sync(){}
	void Draw();
	bullet* playerbullet = nullptr;
};

#endif
