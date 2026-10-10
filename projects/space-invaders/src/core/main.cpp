#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include <functional>
#include <iostream>

#include <GFX\gl_engine.h>
#include <space_invaders/core/game.h>
#include <space_invaders/core/global.h>


string invaders_sprite = "invaders.sdat";
string font_data = "whitefont.fdat";

extern void InitializeDirectories();

int main( int argc, char *argv[] )
{
	//the line below turns on memory leak detection in debug mode
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	_CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_DEBUG );
	//_CrtSetBreakAlloc( 4357 );

	logger::LogFile_Manager::RegisterLog( "SpaceInvaders.log", false );
	InitializeDirectories();

	game::Instance().Run();
	return 0;
}
