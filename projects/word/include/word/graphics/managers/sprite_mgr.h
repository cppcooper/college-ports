#ifndef GRAPHIC_MGR_H
#define GRAPHIC_MGR_H

#include <word/graphics/2d/sprite.h>
#include <word/systems/managers/file_mgr.h>
#include <map>
#include <string>

class Sprite_Manager
{
private:
	GLSLProgram* shader_2d;
	std::map<std::string, sprite*> sprite_map;
	std::map<std::string, sprite*>::iterator sprite_iter;

protected:
	//Loads one or more animated sprites, or two or more unanimated sprites
	void Load_Sprite_Sheet( std::string png_file, std::string sprite_data_file );
	//Loads a non animated sprite
	void Load_Sprite( std::string png_file, std::string sprite_name );

public:
	//Delete all the sprites before the application terminates
	~Sprite_Manager()
	{
		sprite_iter = sprite_map.begin();
		while ( sprite_iter != sprite_map.end() )
		{
			///We need to delete them, we don't have to clean up the messy list though
			delete sprite_iter->second;
			sprite_iter++;
		}
		sprite_iter = sprite_map.begin();
	}
							//Sets the 2d shader to be used by any objects created after the method is invoked
	void set_shader( GLSLProgram* shader );
							//Loads all png files, that the File Manager knows about, which are within the sprite root directory
	void Load_All();
							//Return a particular sprite if it has been loaded
	sprite* Get_Sprite( std::string sprite_name );
};

#endif
