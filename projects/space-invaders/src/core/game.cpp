#include <space_invaders/core/Logs.h>

#include <space_invaders/core/game.h>
#include <space_invaders/core/global.h>

#ifdef _WIN32
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#define DEBUG_NEW new(_CLIENT_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif
#endif

void game::Run()
{
	Init();
	m_engine.Run(); //Main Thread Stays here
	//DeInit();
}

void game::Init()
{
	using namespace std::placeholders;
	m_engine.set_Draw_f( std::bind( &game::Draw, this ) );
	m_engine.set_Update_f( std::bind( &game::Update, this, _1 ) );

	m_engine.key_callback_f = std::bind( &game::DoInput, this, _1, _2, _3, _4 );
	m_engine.mouse_button_callback_f = std::bind( &game::DoMouse, this, _1, _2, _3 );
	m_engine.cursor_position_callback_f = std::bind( &game::DoCursor, this, _1, _2 );
	m_engine.Init();

	//enable blending
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	Asset_Factory<Texture>::Instance().TypeExtensions() = ".png";
	Asset_Factory<GLSLProgram>::Instance().TypeExtensions() = ".vert;.frag";
	Asset_Factory<GLSLProgram>::Instance().RecordExtension() = ".glslp";
	Asset_Factory<Sprite>::Instance().TypeExtensions() = ".sdat";
	Asset_Factory<Font>::Instance().TypeExtensions() = ".fdat";
	m_asset_faculties.RegisterAssetPath( texture_dir );
	m_asset_faculties.RegisterAssetPath( shader_dir );
	m_asset_faculties.RegisterAssetPath( sprite_dir );
	m_asset_faculties.RegisterAssetPath( font_dir );
	m_asset_faculties.LoadAssets();

	SpriteFrame& mal = ( *m_asset_faculties.GetAsset<Sprite>( invaders_sprite ) )[game_sprites::MAL];
	x_alignment = ( mal.m_Scale * mal.Width() ) + 5;
	
	m_engine.Resize( 900, 700 );
	m_engine.LockScreenSize();
	m_engine.SetClearColor( 0.f, 0.f, 0.f );
	m_engine.SetMode( GL_Engine::graphics::R2D );
	m_engine.m_projectionMatrix = glm::mat4( 1.f ) * glm::ortho( 0.f, (float)Screen::Width(), 0.f, (float)Screen::Height(), 0.f, 1.f );

	GLSLProgram* shader_2d = m_asset_faculties.GetAsset<GLSLProgram>( "2d_default.glslp" );
	GLSLProgram* font_shader = m_asset_faculties.GetAsset<GLSLProgram>( "font_default.glslp" );

	// 2D Section
	/// Binding Values to Variables within shader
	shader_2d->SetUniform( "projectionMatrix", m_engine.m_projectionMatrix );
	shader_2d->SetUniform( "viewMatrix", glm::mat4( 1.f ) );
	shader_2d->SetUniform( "in_Alpha", 1.0f );
	shader_2d->SetUniform( "in_Scale", 1.0f );
	shader_2d->BindAttributeLocation( 0, "in_Position" );
	shader_2d->BindAttributeLocation( 1, "in_Texcoord" );
	shader_2d->PrintActiveUniforms();
	shader_2d->PrintActiveAttribs();


	// Font Section
	/// Binding Values to Variables within shader
	font_shader->SetUniform( "projectionMatrix", m_engine.m_projectionMatrix );
	font_shader->SetUniform( "viewMatrix", glm::mat4( 1.f ) );
	font_shader->SetUniform( "in_Alpha", 1.0f );
	font_shader->SetUniform( "in_Scale", 1.0f );
	font_shader->BindAttributeLocation( 0, "in_Position" );
	font_shader->BindAttributeLocation( 1, "in_Texcoord" );
	font_shader->PrintActiveUniforms();
	font_shader->PrintActiveAttribs();

	//load a font set
	m_font = m_asset_faculties.GetAsset<Font>( font_data );
	m_font->m_FontSize = 28.f;

	//Prepare core game functionality
	Load_Score();
	invasion_mgr.SpawnNewWave();
	ThePlayer = new player();
	bullet_mgr.theplayer = ThePlayer;
	player_lives = 5;
	//timeslice = 0.015;
}

void game::DeInit( void )
{
	if ( t1.joinable() )
	{
		t1.join();
	}
	screen.Clean_Up( true ); //We need the quasi-overloaded version to synchronize each object with the main thread

	delete ThePlayer;
	//logger::FileLog_Mgr::Stop();
}

void game::Update( double& seconds )
{
	if ( t1.joinable() )
	{
		t1.join();
	}
	Updating = true;
	LOGFILE_1( _DEBUG1 ) << seconds;
	static double seconds_remainder = 0;
	seconds += seconds_remainder;
	enum { loops = 10 };
	if ( seconds > ( loops * timeslice ) )
	{
		seconds = ( loops * timeslice );
	}
	while ( seconds >= timeslice )
	{
		if ( !Drawing )
		{
			LOGFILE_1( _INFO ) << "Running Updates. Seconds: " << seconds;
			seconds -= timeslice;
			if ( moveL )
			{
				ThePlayer->MoveLeft( timeslice );
			}
			if ( moveR )
			{
				ThePlayer->MoveRight( timeslice );
			}
			if ( shoots )
			{
				bullet_mgr.Player_Shoots();
			}
			//These three lines update our world
			screen.Update_Objects( timeslice ); //Update individual entities
			if ( ThePlayer->playerbullet )
			{
				ThePlayer->playerbullet->Update( timeslice );
			}

			invasion_mgr.Update( timeslice ); //Update invaders as a whole
			bullet_mgr.Update( timeslice ); //Update bullet positions and perform collisions
			bullet_mgr.Clean_Up();
			invasion_mgr.Clean_Up();

			needsDraw = true;
			std::this_thread::sleep_for( std::chrono::milliseconds( 15 ) );
		}
	}
	Updating = false;
	while ( Drawing ){}
	shoots = false;
	seconds_remainder = seconds;
}

void game::Draw( void )
{
	std::this_thread::sleep_for( std::chrono::milliseconds( 30 ) );
	do
	{
		if ( needsDraw )
		{
			needsDraw = false;
			Drawing = true;
			glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

			screen.Draw();
			ThePlayer->Draw();
			if ( ThePlayer->playerbullet )
			{
				ThePlayer->playerbullet->Draw();
			}

			glClear( GL_DEPTH_BUFFER_BIT );

			//Lives - Top Right
			Var_Mutex.lock();
			std::string text = std::to_string(player_lives) + "x                  ";
			Var_Mutex.unlock();

			float y = Screen::Height() - 50;
			float textWidth;
			textWidth = m_font->WidthText( text );
			m_font->Print( Screen::Width() - textWidth, y, text );

			//Score - Top Middle
			text = "      Score :     " + std::to_string( score ) ;
			textWidth = m_font->WidthText( text );
			m_font->Print( Screen::Width() / 2, y, text );

			//High Score - Top Left
			text = "High Score :    " + std::to_string( highscore ) + "    ";
			textWidth = m_font->WidthText( text );
			m_font->Print( 50.f, y, text );
			Drawing = false;
			
			glfwSwapBuffers( m_engine.m_window );
		}
	} while ( Updating );
	screen.Clean_Up();
}

//the key codes/actions/mods for DoInput are from GLFW: check its documentation for their values
void game::DoInput( int& key, int& scancode, int& action, int& mods )
{
	if ( action == GLFW_PRESS || action == GLFW_REPEAT )
	{
		switch ( key )
		{
			case GLFW_KEY_ESCAPE:
				Game_Over(); //start the shutdown sequence
				break;
			
			case GLFW_KEY_LEFT:
			case GLFW_KEY_A:
				moveL = true;
				break;

			case GLFW_KEY_RIGHT:
			case GLFW_KEY_D:
				moveR = true;
				break;

			case GLFW_KEY_LEFT_SHIFT:
			case GLFW_KEY_LEFT_CONTROL:
			case GLFW_KEY_SPACE:
				shoots = true;
				break;
		}
	}
	if ( action == GLFW_RELEASE )
	{
		switch ( key )
		{
			case GLFW_KEY_LEFT:
			case GLFW_KEY_A:
				moveL = false;
				break;

			case GLFW_KEY_RIGHT:
			case GLFW_KEY_D:
				moveR = false;
				break;

			case GLFW_KEY_LEFT_SHIFT:
			case GLFW_KEY_LEFT_CONTROL:
			case GLFW_KEY_SPACE:
				//shoots = false;
				break;
		}
	}
}

void game::DoMouse( int& button, int& action, int& mod )
{

}

void game::DoCursor( double& x, double& y )
{

}

void game::Load_Score()
{
	std::ifstream File_Handle;
	//Open file
	File_Handle.open("highscore.txt", std::ios::in);

	//Check if it opened
	if ( File_Handle.is_open() )
	{
		try
		{
			//Load data
			File_Handle >> highscore;
		
			//Check if loading data went bad
			if ( File_Handle.fail() )
				throw File_Handle.rdstate();

			File_Handle.close();
										//File has been loaded Okay
		}
		catch ( ... )
		{
			assert( "Problem loading data from file" );
		}
	}
	else
	{
		assert( "Problem opening file- check file path" );
	}
}

void game::Save_Score()
{
	std::ofstream File_Handle;
	//Open file
	File_Handle.open("highscore.txt", std::ios::out);

	//Check if it opened
	if ( File_Handle.is_open() )
	{
		try
		{
			//Save data
			File_Handle << score;

			//Check if saving went bad
			if ( File_Handle.fail() )
				throw File_Handle.rdstate();

			File_Handle.close();
										//File has been saved Okay
		}
		catch ( ... )
		{
			assert( "Problem loading data from file" );
		}
	}
	else
	{ 
		assert( "Problem opening file- check file path" );
	}
}

void game::Player_Death()
{
	if ( t1.joinable() )
	{
		t1.join();
	}
	t1 = std::thread //Makin' a Lambda Thread.. oya
	( [this]  //Capturing 'THIS' allows our lambda to access this object's members/methods
		{
			//Decrement player_lives
			Var_Mutex.lock();
			if ( --player_lives <= 0 )
			{
				Var_Mutex.unlock();
				Game_Over();
			}
			else
			{
				Var_Mutex.unlock();
				//We want the player's sprite to stay dead for 1000ms
				std::this_thread::sleep_for( std::chrono::milliseconds( 1000 ) );
				ThePlayer->unDeath();
			}
		}
	); //End of constructor
}

void game::Game_Over()
{
  	if ( score > highscore )
	{
		Save_Score();
	}
	m_engine.Quit();
}

void game::Increase_Score( int delta )
{
	static unsigned int d = 0;
	d += delta;
	Var_Mutex.lock();
	score += ( delta * player_lives );
	Var_Mutex.unlock();

	#define Threshold 4700
	if ( ( d / Threshold ) > 1 )
	{
		Var_Mutex.lock();
		if ( player_lives < 8000 )
		{
			player_lives += int( d / Threshold );
		}
		Var_Mutex.unlock();
		d = d % Threshold;
	}
}
