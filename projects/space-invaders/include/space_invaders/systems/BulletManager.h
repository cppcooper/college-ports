#ifndef _BULLETMANAGER_H_
#define _BULLETMANAGER_H_
#include <space_invaders/objects/screen_objs.h>
#include <space_invaders/systems/invasion_mgr.h>

class player;

class bullet : public ScreenObject
{
	friend class bullet_manager;
private:
	float yincrements;
protected:
public:
	bullet( float x, float y, bool isplayer = false );
	virtual ~bullet(){}
	void deleteSelf() final override { delete this; }
	void Update( double& seconds );
	void Sync(){}
};

//TODO: Bullet_Manager should have a separate header
//Design-wise Invasion_Formation should not be apart of this
class bullet_manager : protected Invasion_Formation
{
private:
	Screen_Mgr screen;

	std::default_random_engine RNG;
	std::bernoulli_distribution yesno_distrib;
	std::uniform_int_distribution<int> column_distrib;
	std::piecewise_linear_distribution<double> shots_distrib;

	std::chrono::time_point<std::chrono::system_clock> last_time_shotsmade;
	std::mutex m_BulletsLock;
	std::vector<bullet*> m_Bullets;
protected:
	bool x_overlap_check( bullet* B, phys_box* obj );
	bool y_overlap_check( bullet* B, phys_box* obj );
	void Clear_Chamber();

public:
	bullet_manager();
	player* theplayer = nullptr;
	void Update( double& seconds );
	void Clean_Up();
	void Player_Shoots();
	void Enemies_Shoot();
	void EnemyCollisions();
	void PlayerCollisions();
};

#endif
