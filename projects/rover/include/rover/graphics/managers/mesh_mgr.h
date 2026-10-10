#ifndef _S3DMESHLOADER_H_
#define _S3DMESHLOADER_H_

#include <rover/graphics/3d/mesh.h>
#include <rover/graphics/managers/texture_mgr.h>
#include <rover/systems/managers/file_mgr.h>

#include <map>

/******************************************************
* S3DMesh_Manager Class
* Imports S3D file and stores the information within the class.
* Created by:	Josh Cooper		Date:	Dec. 4, 2014
* Modified by:	Josh Cooper		Date:	Dec. 4, 2014
*******************************************************/
class s3d_mesh_mgr
{
private:
	GLSLProgram* shader_3d = nullptr;
	std::map<std::string, mesh*> mesh_map;
	std::map<std::string, mesh*>::iterator mesh_iter;

	void Free_Meshes(){}
											//Imports a single mesh from a file with an s3d format
	inline mesh* Import_S3D_File( std::string file_path );

public:
	//Prepares the object for use
	s3d_mesh_mgr()
	{
		///May move directory registers to corresponding managing objects
		//Until then this is just a reminder
		File_Manager::Register_Directory( "" );
	}
	//This object dynamically allocated many objects, it now must destroy them all before it can die
	~s3d_mesh_mgr()
	{
		mesh_iter = mesh_map.begin();
		while ( mesh_iter != mesh_map.end() )
		{
			///We must delete the objects, removal from the container is not necessary
			delete mesh_iter->second;
			mesh_iter++;
		}
		mesh_iter = mesh_map.begin();
	}

											//Sets the 3d shader to be used by any objects created after the method is invoked
	void set_Shader( GLSLProgram* shader_3d );
											//Loads all s3d files that can be found by the File Manager
	void Load_All();
											//Loads just one s3d file from a particular path
	void Load_Mesh( std::string file, std::string path );
											//Return a particular mesh if it has been loaded
	mesh* Get_Mesh( std::string file_name );
};

#endif
