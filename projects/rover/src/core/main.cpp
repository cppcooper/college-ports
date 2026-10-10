/*		ToDo List:
			***	Write comment blocks for each file and class
			***	Revise maze generation logging
			**	Implement Texture Manager logging
			**	Implement Key Frame animations for 3d Objects
			**	Learn Shader programming and Revisit Shaders
			**	Make Scene Node code easier to read
			*		Implement Basic RPG Stat System
			*		Implement base engine logging
			*		Implement game engine logging
			*		get_exec_path is fixed, reimplement art asset directories
			*		replace input queue (std::vector) with a simple array
			*		Test dynamic text objects and ensure thread safety
			*		Review engine_3d::SetMode methods
*/
#include <rover/graphics/interface/ffont.h>
#include <rover/misc/util_func.h>
#include <rover/systems/engine/3d_engine.h>
#include <rover/systems/engine/game.h>

#include <rover/core/global.h>
#include <rover/core/log.h>

string assets_dir = "Assets\\";
string fonts_dir = assets_dir + "Font\\";
string meshes_dir = assets_dir + "Mesh\\";
string sprites_dir = assets_dir + "Sprite\\";
string shaders_dir = assets_dir + "Shader\\";
string textures_dir = assets_dir + "Texture\\";


//What this project is and has been to me:
/*
		First it should be noted, this is my first real attempt at creating a general purpose engine. With that said this project
		has grown organically along side my knowledge and understanding of OpenGL.

		This project has been a labour of my love to learn. I spent 5 months learning OpenGL from nothing more than a
		pre-existing engine which an Instructor made. I had studied his code and remade much of the functionality he
		had provided, leaving out a few and adding a number of features he did not have. A feature I added was the
		ability to automatically load art assets from directories for example. Once I had recreated all the higher functions of his
		engine I focused on replacing the engine itself. All this while juggling much more stressful and	demanding
		course workloads. The majority of this code base I have created in my spare time, the full working prototype
		was made with a few classes I made through the term and the rest I created during one week of
		my Christmas break.


		It had started out as a team project in second year of college, but after a term of working on it I discovered I was
		the only one who had provided any functional code to our repository for integration with the engine.
		I had been in charge of the engine and all its functions whereas my team was in charge of making dungeon
		crawling gameplay. We had trouble creating a three dimensional representation of a 2d maze. We were unable 
		to complete a stat system.

*/

///this code sets up memory leak detection
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#define DEBUG_NEW new(_CLIENT_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif



///GLOBAL DATA
FFont* Fcout = nullptr;

void main(int argc, char *argv[])
{
	get_exec_path( 0 );
	std::cout << execution_path << std::endl;
	if ( execution_path.find( "build" ) != std::string::npos )
	{
		get_exec_path( 4 );
		std::cout << execution_path << std::endl;
	}
	fonts_dir.insert( 0, execution_path );
	meshes_dir.insert( 0, execution_path );
	sprites_dir.insert( 0, execution_path );
	shaders_dir.insert( 0, execution_path );
	textures_dir.insert( 0, execution_path );
	system( "pause" );

	///the line below turns on memory leak detection in debug mode
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	//_CrtSetBreakAlloc( 657 );

	dbg_log::FileLog_Mgr::RegisterNewLog( execution_path + "Memory Issues.log", true );
	dbg_log::FileLog_Mgr::Start();

	/// Initialize the OpenGL engine
	get_cooper3d().Init();
	/// Initialize the game assets, controls, shaders, etc
	get_game_engine().Init();

	///Setting the update and draw functions to call
	get_cooper3d().set_Update_f( std::bind( &game::Update, &get_game_engine() ) );
	get_cooper3d().set_Draw_f( std::bind( &game::Draw, &get_game_engine() ) );

	/// Initiate game model
	get_cooper3d().Run();
	get_game_engine().Deinit();

	dbg_log::FileLog_Mgr::CloseLogs();
	exit(0);
}
