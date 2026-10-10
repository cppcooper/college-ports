#include <hypermaze/core/STL.h>
#include <GF/game_faculties.h>
#pragma once

class debugger
{
protected:
	GameAssets::Font* m_Font;
	debugger();
public:
#ifdef _DEBUG
	bool m_Enabled = true;
#else
	bool m_Enabled = false;
#endif
	uint32 Player_X;
	uint32 Player_Y;
	uint32 Traversal_X;
	uint32 Traversal_Y;
	uint32 Terrain_SX;
	uint32 Terrain_SY;
	uint32 Travel_Dir;
	uint32 Traversal_Dir;
	uint64 Terrain;
	uint64 Node;
	uint64 NodeLink;

	static debugger& Instance();
	void Draw();
};
