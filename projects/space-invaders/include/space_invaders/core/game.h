#ifndef GAME_H_
#define GAME_H_

#include <GFX\gl_engine.h>
#include <GFX\OpenGL_Headers.h>
#include <GF\game_faculties.h>

#include <space_invaders/objects/Play.h>
#include <space_invaders/systems/BulletManager.h>
#include <space_invaders/systems/invasion_mgr.h>
#include <space_invaders/systems/screen_mgr.h>
#include <space_invaders/core/STL.h>

class game
{
private:
	glEngine& m_engine = glEngine::Instance();
	Asset_Faculties& m_asset_faculties = Asset_Faculties::Instance();
	double timeslice = 0.015;
	std::atomic<bool> needsDraw = false;
	std::atomic<bool> Updating = true;
	std::atomic<bool> Drawing = false;

	bool moveL = false;
	bool moveR = false;
	bool shoots = false;

	std::mutex Var_Mutex;
	unsigned short player_lives;
	unsigned int highscore;
	unsigned int score;

	std::thread t1;

	GameAssets::Texture* m_fontTex = nullptr;
	GameAssets::Font* m_font = nullptr;
	player* ThePlayer = nullptr;
	bullet_manager bullet_mgr;
	Invasion_Formation invasion_mgr;
	Screen_Mgr screen;
	game(){}

protected:
	void Load_Score();
	void Save_Score();

public:
	static game& Instance()
	{
		static game instance;
		return instance;
	}
	void DoInput( int& key, int& scancode, int& action, int& mods );
	void DoMouse( int& button, int& action, int& mod );
	void DoCursor( double& x, double& y );

	void Run();
	void Init();
	void DeInit();
	void Update( double& seconds );
	void Draw();

	void Increase_Score( int delta );
	void Player_Death();
	void Game_Over();
};

#endif
