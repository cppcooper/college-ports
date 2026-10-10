#ifndef _TEXTURE_MGR_H_
#define _TEXTURE_MGR_H_

#define TEXTURE_MANAGER_MAX_TEXTURES 31

///Platform Specific
#include <Windows.h>

///Third Party / Tool box
#include <winfs_tools.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <FreeImage.h>

///C++ STL
#include <unordered_set>
#include <map>

///Project
#include <rover/systems/managers/file_mgr.h>

using texid = GLuint;

struct texture
{
	texid ID;
	bool unload;
	unsigned int refcount;
	unsigned int width, height;
};

class Texture_Manager
{
	private:
		///Associates file paths to texture IDs given by OpenGL
		std::map<std::string, texid> file_map;
		std::map<std::string, texid>::iterator file_iter;

		///Associates texture IDs to texture data
		std::map<texid, texture*> texture_map;
		std::map<texid, texture*>::iterator texture_iter;

		///Which textures are bound and to which texture unit
		GLuint Bound_IDs[TEXTURE_MANAGER_MAX_TEXTURES];/// = { -1 }; ///currently bound texture
		///Optimize texture scaling
		bool useMipMaps = true;

	public:
		//Deletes textures before application terminates
		~Texture_Manager()
		{
			///We have to delete each Texture which has been allocated and placed into our Texture map
			for ( auto T : texture_map )
			{
				delete T.second;
			}
		}

													//Loads all PNG files which the File Manager knows about
		void LoadAll();
													//Load a particular texture from a particular path
		void LoadTexture(std::string file, std::string path);
													//Queries the manager for a texture name matching the string parameter `File`
		GLuint GetID(std::string File, bool count_increase = true);
													//Retrieves the texture data associated to the ID passed in
		texture* GetTexPtr( texid ID );

													//A method to allow users to manually free textures from memory
		void FreeTexture(GLuint texid);
													//Bind the texture ID for rendering
		void BindTexture(GLuint texid, GLuint texture_unit = GL_TEXTURE0);
};

#endif
