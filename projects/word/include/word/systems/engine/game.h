#ifndef GAME_H_
#define GAME_H_

#include <word/gameplay/WordSearch.h>
#include <word/graphics/fundamentals/scene_nodes.h>
#include <word/graphics/interface/ffont.h>
#include <word/graphics/managers/Shader_Manager.h>
#include <word/graphics/managers/mesh_mgr.h>
#include <word/graphics/managers/sprite_mgr.h>
#include <word/graphics/managers/texture_mgr.h>
#include <word/systems/OGLheaders.h>
#include <word/systems/managers/input_mgr.h>

#include <atomic>
#include <mutex>





class game
{
private:
	ShaderManager sManager;
	GLSLProgram* shader_3d = nullptr;
	GLSLProgram* shader_2d = nullptr;
	GLSLProgram* font_shader = nullptr;

	glm::vec3 player_straight_axis;
	glm::vec3 player_side_axis;
	std::mutex mouseMutex;

	WordBox data;
	WordSearch* wordsearch = nullptr;

	sprite* p = nullptr;

protected:
	void Init_GFX();
	void Init_Input();
	void Rotate_Camera( float angle );
	void Rotate_Left();
	void Rotate_Right();
	void Move_Forward();
	void Move_Backward();
	void Move_Left();
	void Move_Right();

public:
	~game()
	{

	}
	void Init();
	void Deinit();
	void Update();
	void Draw();
	void DoInput( int& key, int& scancode, int& action, int& mods );
	void SetCursor( double x, double y );
	void Set_MouseClicks( int button, int actions, int mods );

	Texture_Manager txtr_mgr;
	Sprite_Manager sprite_mgr;
	s3d_mesh_mgr mesh_mgr;
	input_mgr Controls_Input_Mgr;
};

#endif
