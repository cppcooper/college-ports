#ifndef GAME_H_
#define GAME_H_

#include <rover/graphics/interface/ffont.h>
#include <rover/graphics/managers/Shader_Manager.h>
#include <rover/graphics/managers/mesh_mgr.h>
#include <rover/graphics/managers/sprite_mgr.h>
#include <rover/graphics/managers/texture_mgr.h>
#include <rover/graphics/fundamentals/scene_nodes.h>
#include <rover/levels/map_editor.h>
#include <rover/systems/OGLheaders.h>
#include <rover/systems/managers/input_mgr.h>
#include <rover/actors/rover.h>

#include <atomic>
#include <mutex>





class game
{
private:
	Root_SceneNode SceneGraph;
	ShaderManager sManager;
	GLSLProgram* shader_3d = nullptr;
	GLSLProgram* shader_2d = nullptr;
	GLSLProgram* font_shader = nullptr;

	glm::vec3 player_straight_axis;
	glm::vec3 player_side_axis;
	std::mutex mouseMutex;

	Map_Editor* editor = nullptr;
	View_Port* view_port = nullptr;
	Tile_Map* map = nullptr;
	Rover* myrover = nullptr;

	sprite* G = nullptr;

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
		if ( editor )
		{
			editor->Save();
		}
		delete editor;
		delete map;
		delete view_port;
		delete myrover;
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
