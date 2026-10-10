#include <mutex>
#include <GF\GameFramework.h>
#include <hypermaze/actors/Actors.h>
#include <hypermaze/core/Debugger.h>
#include <hypermaze/world/World.h>

void InitializeDirectories()
{
	string root_dir;
	string asset_dir;
	get_exec_path( 4 );
	root_dir = execution_path;
	asset_dir = root_dir + "Assets\\";
	Asset_Faculties& assets = Asset_Faculties::Instance();
	assets.RegisterAssetPath( asset_dir + "Textures\\" );
	assets.RegisterAssetPath( asset_dir + "Shaders\\" );
	assets.RegisterAssetPath( asset_dir + "Data\\Fonts\\" );
	assets.RegisterAssetPath( asset_dir + "Data\\Graphics\\2D\\" );
}

class Scene : public GameModule
{
private:
	std::mutex m_LazyThreadSafety;
	glEngine& m_engine = glEngine::Instance();
	ControlModule& m_Controls = ControlModule::Instance();
	RootNode m_Root;
	World m_World;
	Actors* m_Actors = nullptr;

	//TODO: GUI

public:
	Scene(){ 
		m_World.AttachToScene( m_Root );
		m_Controls.Disable_MouseInput();
	}

	void Init() final override
	{
		m_engine.SetMode( GL_Engine::graphics::R2D );
		m_engine.m_projectionMatrix = glm::mat4( 1.f ) * glm::ortho( 0.f, (float)Screen::Width(), 0.f, (float)Screen::Height(), 0.f, 1.f );
		GLSLProgram* shader_2d = Asset_Faculties::Instance().GetAsset<GLSLProgram>( "2d_default.glslp" );
		GLSLProgram* font_shader = Asset_Faculties::Instance().GetAsset<GLSLProgram>( "font_default.glslp" );

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
		m_World.Init();
		assert( m_Actors == nullptr );
		m_Actors = new Actors();
		m_Actors->ChangeWorld( &m_World );
		m_Actors->Init();
	}
	void Deinit() final override
	{
		m_Actors->Deinit();
		m_World.Deinit();
		delete m_Actors;
	}
	void Process() final override
	{
		//m_Actors.Process();
		//m_World.Process();
	}
	void Update( double& seconds ) final override
	{
		m_LazyThreadSafety.lock();
		m_Controls.Process();
		m_Controls.Update( seconds );
		//m_Actors.Update( seconds );
		//m_World.Update( seconds );
		//m_Root.Update( seconds );
		m_LazyThreadSafety.unlock();
	}
	void PostProcess() final override
	{
		//m_Actors.PostProcess();
		//m_World.PostProcess();
	}
	void Buffer() final override
	{
		//m_Actors.Buffer();
		//m_World.Buffer();
	}
	void Draw() final override
	{
		m_LazyThreadSafety.lock();
		static TileSet& m_TileSet = *Asset_Factory<TileSet>::Instance().GetAsset( "hypermaze.tdat" );
		m_Root.Draw();
		debugger::Instance().Draw();
		m_LazyThreadSafety.unlock();
	}
};

void main()
{
	logger::LogFile_Manager::RegisterLog( "Hyper-Maze.log", false, logger::LogLevel::_DEBUG4 );
	
	Game& game = Game::Instance();
	glEngine& engine = glEngine::Instance();
	
	engine.SetClearColor( 0.35f, 0.4f, 0.15f );
	engine.LockScreenSize();

	Scene scene;
	game.Attach( &scene );
	game.Detach( &ControlModule::Instance() );

	InitializeDirectories();
	game.Init();
	game.Run();
}
