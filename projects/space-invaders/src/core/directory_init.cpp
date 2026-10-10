#include <tools_winfs.h>
#define _SKIP_GF_HEADER_
#include <space_invaders/core/global.h>
#undef _SKIP_GF_HEADER_

string texture_dir;
string shader_dir;
string font_dir;
string sprite_dir;

void InitializeDirectories()
{
	string root_dir;
	string asset_dir;
	get_exec_path( 4 );
	root_dir = execution_path;
	asset_dir = root_dir + "Assets\\";
	texture_dir = asset_dir + "Textures\\";
	shader_dir = asset_dir + "Shader\\";
	font_dir = asset_dir + "Data\\Fonts\\";
	sprite_dir = asset_dir + "Data\\Sprites\\";
}
