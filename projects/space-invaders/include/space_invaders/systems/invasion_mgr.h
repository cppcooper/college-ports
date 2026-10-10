#ifndef _INVASIONMGR_H_
#define _INVASIONMGR_H_

#include <space_invaders/core/STL.h>
#include <space_invaders/objects/enemies.h>
#include <space_invaders/objects/phys_obj.h>
#include <space_invaders/systems/screen_mgr.h>

class Invasion_Formation
{
	using invasion_columns = std::vector<phys_box*>;
	using invasion_forces = std::vector<invasion_columns>;
	using ushort = unsigned short;

protected:
	Screen_Mgr screen;

	void Add( invasion_columns col );
	inline void Flip_Direction();
	inline void Move_Invaders( double &seconds );
	static invasion_forces& GetV()
	{
		static invasion_forces Vector_of_Vectors_of_Aliens;
		return Vector_of_Vectors_of_Aliens;
	}
	std::mutex m_InvaderLock;
	float xincrements = 0;

public:
	virtual void Update( double& seconds );
	void Clean_Up( bool delete_all = false );
	void SpawnNewWave();
};

#endif
