#ifndef _SCREENMGR_H_
#define _SCREENMGR_H_

#include <space_invaders/core/Logs.h>
#include <space_invaders/core/STL.h>
#include <space_invaders/objects/screen_objs.h>

class Screen_Mgr
{
private:
	static std::mutex s_Lock;
	static std::vector<ScreenObject*> s_Array;
public:
	~Screen_Mgr();
	void Add( ScreenObject* obj );
	void Clean_Up( bool delete_all = false );
	void Update_Objects( double& seconds );
	void Draw();
};

#endif
