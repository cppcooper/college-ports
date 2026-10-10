#include <stdio.h>
#include <conio.h>
#include <thread>

#include <word/core/global.h>
#include <word/core/log.h>
#include <word/graphics/2d/sprite_batch.h>
#include <word/misc/util_func.h>
#include <word/systems/OGLheaders.h>
#include <word/systems/engine/game.h>


#ifdef _WIN32
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#define DEBUG_NEW new(_CLIENT_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif
#endif

///GLOBAL DATA
std::mutex View_Mutex;
glm::mat4 View_Matrix = glm::mat4( 1.0f );
glm::mat4 View_Angle = glm::mat4( 1.0f );
glm::mat4 View_Position = glm::mat4( 1.0f );

///Base Engine Singleton - OpenGL
engine_3d& get_cooper3d()
{
	return engine_3d::Get_Instance();
}

///Game Engine Singleton - Usage of engine and more
game& get_game_engine()
{
	static game engine;
	return engine;
}

//Wrapper functions for methods in an object
void DoInput( int key, int scancode, int action, int mods )
{
	get_game_engine().DoInput( key, scancode, action, mods );
}
void DoCursor( double x, double y )
{
	get_game_engine().SetCursor( x, y );
}
void DoMouseClick( int button, int action, int mods )
{
	get_game_engine().Set_MouseClicks( button, action, mods );
}

//Game Engine initialization
void game::Init()
{
	///This is the directory Meshes and Textures are found in
	File_Manager::Register_Directory( textures_dir );
	File_Manager::Register_Directory( sprites_dir );
	File_Manager::Register_Directory( meshes_dir );

	///Prepare the graphical assets & shaders
	Init_GFX();

	//data.Load_Data( "C:/Users/1039652/Desktop/Cheryl-Engine/word_data_1.wrd" );
	data.Load_Data( execution_path + "word_data_1.wrd" );
	wordsearch = new WordSearch( &data );
	wordsearch->Scan_WordBox();

	///Prepare the game controls
	Init_Input();
}

//Performs any necessary end of game clean-up
void game::Deinit()
{
	Controls_Input_Mgr.Stop();
	delete Fcout;
}

//Auto load art assets, and prepare shaders
void game::Init_GFX()
{
	/// We need to prepare our Shaders
	shader_3d = sManager.GetShader( "lighting.vert", "lighting.frag" );
	shader_2d = sManager.GetShader( "shader2d.vert", "shader2d.frag" );
	font_shader = sManager.GetShader( "font_shader.vert", "font_shader.frag" );

	/// Load Assets
	//################################
	txtr_mgr.LoadAll(); /// Textures

	sprite_mgr.set_shader( shader_2d );
	sprite_mgr.Load_All(); /// Sprites

	mesh_mgr.set_Shader( shader_3d );
	mesh_mgr.Load_All();  /// Meshes

	Fcout = new FFont( font_shader );
	//################################


	/// Enable Blending
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	/// Camera Setup
	View_Angle = glm::rotate( View_Angle, 180.0f, glm::vec3( 0, 0, 1 ) );
	View_Matrix = View_Angle * View_Position;
	glm::vec4 dir = View_Angle * glm::vec4( 0, 0, 1, 0 );
	player_straight_axis = glm::vec3( dir.x, dir.y, dir.z );
	dir = View_Angle * glm::vec4( 1, 0, 0, 0 );
	player_side_axis = glm::vec3( dir.x, dir.y, dir.z );


	/// Preparing Variable Values
	get_cooper3d().projectionMatrix = glm::perspective( 45.0f, (GLfloat) ( get_cooper3d().get_ScreenWidth() ) / (GLfloat) ( get_cooper3d().get_ScreenHeight() ), 0.1f, 10000.0f );
	glm::vec3 LightPosition = glm::vec3( 1.0f, 1.0f, 1.0f );
	glm::vec3 LightIntensity = glm::vec3( 1.0f, 1.0f, 1.0f );
	glm::vec3 Kd = glm::vec3( 1.0f, 1.0f, 0.2f );
	glm::vec3 Ka = glm::vec3( 0.1f, 0.1f, 0.2f );
	glm::vec3 Ks = glm::vec3( 1.0f, 1.0f, 1.0f );
	GLfloat Shininess = 1.0f;

	// 3D Section
	/// Binding Values to Variables within shader
	shader_3d->setUniform( "projectionMatrix", get_cooper3d().projectionMatrix );
	shader_3d->setUniform( "LightPosition", LightPosition );
	shader_3d->setUniform( "LightIntensity", LightIntensity );
	shader_3d->setUniform( "Kd", Kd );
	shader_3d->setUniform( "Ka", Ka );
	shader_3d->setUniform( "Ks", Ks );
	shader_3d->setUniform( "Shininess", Shininess );
	shader_3d->setUniform( "in_Alpha", 1.f );
	shader_3d->bindAttribLocation( 0, "in_Position" );
	shader_3d->bindAttribLocation( 1, "in_Normal" );
	shader_3d->bindAttribLocation( 2, "in_Texcoord" );
	shader_3d->printActiveUniforms();
	shader_3d->printActiveAttribs();

	get_cooper3d().projectionMatrix = glm::mat4( 1.f ) * glm::ortho( 0.f, (float)get_cooper3d().get_ScreenWidth(), 0.f, (float)get_cooper3d().get_ScreenHeight(), 0.f, 1.f );
	glm::mat4& projection = get_cooper3d().projectionMatrix;
	// 2D Section
	/// Binding Values to Variables within shader
	shader_2d->setUniform( "projectionMatrix", get_cooper3d().projectionMatrix );
	shader_2d->setUniform( "viewMatrix", glm::mat4( 1.f ) );
	shader_2d->setUniform( "in_Alpha", 1.0f );
	shader_2d->setUniform( "in_Scale", 1.0f );
	shader_2d->bindAttribLocation( 0, "in_Position" );
	shader_2d->bindAttribLocation( 1, "in_Texcoord" );
	shader_2d->printActiveUniforms();
	shader_2d->printActiveAttribs();
	

	// Font Section
	/// Binding Values to Variables within shader
	font_shader->setUniform( "projectionMatrix", get_cooper3d().projectionMatrix );
	font_shader->setUniform( "viewMatrix", glm::mat4( 1.f ) );
	font_shader->setUniform( "in_Alpha", 1.0f );
	font_shader->setUniform( "in_Scale", 1.0f );
	font_shader->bindAttribLocation( 0, "in_Position" );
	font_shader->bindAttribLocation( 1, "in_Texcoord" );
	font_shader->printActiveUniforms();
	font_shader->printActiveAttribs();
}

//Prepare game controls and begin the processing queue
void game::Init_Input()
{
	/// Keyboard Example - Controls_Input_Mgr.Register_Action( GLFW_KEY_W, std::bind( &Map_Editor::Scroll_Up, editor ) );
	/// Cursor Example - Controls_Input_Mgr.Add_CursorReader( std::bind( &Map_Editor::do_Input, editor, std::placeholders::_1, std::placeholders::_2 ) );
	/*Controls_Input_Mgr.Register_Action( GLFW_KEY_W, std::bind( &Traversable_Maze::Move_Up, maze ) );
	Controls_Input_Mgr.Register_Action( GLFW_KEY_A, std::bind( &Traversable_Maze::Move_Left, maze ) );
	Controls_Input_Mgr.Register_Action( GLFW_KEY_S, std::bind( &Traversable_Maze::Move_Down, maze ) );
	Controls_Input_Mgr.Register_Action( GLFW_KEY_D, std::bind( &Traversable_Maze::Move_Right, maze ) );*/
	Controls_Input_Mgr.Register_Action( GLFW_KEY_LEFT, std::bind( &WordSearch::SelectLast, wordsearch ) );
	Controls_Input_Mgr.Register_Action( GLFW_KEY_RIGHT, std::bind( &WordSearch::SelectNext, wordsearch ) );
	Controls_Input_Mgr.Register_Action( GLFW_KEY_UP, std::bind( &WordSearch::SelectLast, wordsearch ) );
	Controls_Input_Mgr.Register_Action( GLFW_KEY_DOWN, std::bind( &WordSearch::SelectNext, wordsearch ) );
	Controls_Input_Mgr.Register_Action( GLFW_KEY_SPACE, std::bind( &WordSearch::ToggleSolution, wordsearch ) );
	Controls_Input_Mgr.Start();
}

//Game logic / mechanics
void game::Update()
{
	///View_Matrix = glm::rotate( View_Matrix, 0.5f, glm::vec3( 0, 0, 1 ) );
	//SceneGraph.Update( seconds );
	std::this_thread::sleep_for( std::chrono::milliseconds( 150 ) );
}

//Render objects
void game::Draw( void )
{
	///Prepare scene
	glClearColor( 0.8f, 0.6f, 0.7f, 0.0f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	///Prepare 2D perspective (projection matrix primarily)
	get_cooper3d().SetMode( engine_RenderMode::R2D );

	///Sprite Section
	/////
	wordsearch->Draw();
	std::this_thread::sleep_for( std::chrono::milliseconds( 150 ) );
}

//Input happens - Adds to queue for processing in separate thread
inline void game::DoInput( int& key, int& scancode, int& action, int& mods )
{
	if ( key == GLFW_KEY_ESCAPE && action == GLFW_PRESS )
	{
		get_cooper3d().Quit(); ///start the shutdown sequence
	}
	else if ( action == GLFW_PRESS )
	{
		Controls_Input_Mgr.QueueKey( key );
	}
}

inline void game::SetCursor( double x, double y )
{
	if ( x >= 0 && y >= 1 )
	{
		if ( x < get_cooper3d().get_ScreenWidth() && y < get_cooper3d().get_ScreenHeight() )
		{
			Controls_Input_Mgr.DoCursor( x, get_cooper3d().get_ScreenHeight() - y );
		}
	}
}

inline void game::Set_MouseClicks( int button, int actions, int mods )
{
	if ( button == GLFW_MOUSE_BUTTON_LEFT )
	{
		if ( actions == GLFW_PRESS )
		{
			Controls_Input_Mgr.Set_MouseClick( true );
		}
		else if ( actions == GLFW_RELEASE )
		{
			Controls_Input_Mgr.Set_MouseClick( false );
		}
	}
}
