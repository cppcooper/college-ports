#include <hypermaze/core/Debugger.h>
#include <GFX/gl_engine.h>

using namespace GameAssets;
debugger::debugger()
{
	m_Font = Asset_Faculties::Instance().GetAsset<Font>( "fontwhite.fdat" );
	m_Font->m_FontSize = 28.f;
}

debugger& debugger::Instance()
{
	static debugger instance;
	return instance;
}

void debugger::Draw()
{
	if ( m_Enabled )
	{
		static glm::mat4 matrix( 1.f );
		static const uint16 start_y = Screen::Height() - 15;
		static const uint16 start_x = 10;
		std::stringstream data;
		data
			<< std::endl << "Player:                " << Player_X << ", " << Player_Y
			<< std::endl << "Traversal:          " << Traversal_X << ", " << Traversal_Y
			<< std::endl << "Terrain:               " << Player_X - Terrain_SX << ", " << Player_Y - Terrain_SY
			<< std::endl << "Travel Dir:         " << Travel_Dir
			<< std::endl << "Traversal Dir:    " << Traversal_Dir
			<< std::endl << "Terrain ID:         " << (void*)Terrain
			<< std::endl << "Node ID:             " << (void*)Node
			<< std::endl << "NodeLink ID:    " << (void*)NodeLink;
		m_Font->Print( (float)start_x, (float)start_y, data.str() );
	}
}
