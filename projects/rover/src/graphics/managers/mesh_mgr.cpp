#include <rover/core/global.h>
#include <rover/core/log.h>
#include <rover/graphics/fundamentals/vbo_data.h>
#include <rover/graphics/managers/mesh_mgr.h>

//Sets the 3d shader to be used by any objects created after the method is invoked
void s3d_mesh_mgr::set_Shader( GLSLProgram* shader_3d )
{
	this->shader_3d = shader_3d;
}

//Loads all the s3d files the File Manager knows about
void s3d_mesh_mgr::Load_All()
{
	/// prefix sl for string local type
	using slFile_Name = std::string;
	using slFile_Path = std::string;

	///Queries the File Manager for all S3D files
	std::map<slFile_Name, slFile_Path> path_map = File_Manager::Get_Files( ".s3d" );
	std::map<slFile_Name, slFile_Path>::iterator path_iter = path_map.begin();

	///iterates through the list of S3D files
	for ( ; path_iter != path_map.end(); ++path_iter )
	{
		///If we haven't already loaded a mesh with a matching name
		if ( mesh_map.find( path_iter->first ) == mesh_map.end() )
		{
			///Load the mesh from the following file path
			Load_Mesh( path_iter->first, path_iter->second );
		}
	}
}

//Load a particular mesh from a particular path
void s3d_mesh_mgr::Load_Mesh( std::string file, std::string path )
{
	///Load data from the file into a mesh object
	mesh* mymesh = Import_S3D_File( path + file );
	if ( mymesh )
	{
		///The load was successful so we need to save the mesh for use
		mesh_map.emplace( file, mymesh );
	}
}

//Imports a single mesh from a file with an s3d format
mesh* s3d_mesh_mgr::Import_S3D_File( std::string file_path )
{
	///Just below are all the variables we will need
	GLuint total_vertices = 0;
	GLuint total_indices = 0;
	float* Vertices = nullptr;
	unsigned int* Indices = nullptr;
	std::string texture_file;


	///It is time to open and load the file
	ifstream File_Handle;
	File_Handle.open( file_path, std::ios::in );

	if ( File_Handle.is_open() )
	{
		try
		{
			///First number in the file should be the total vertices
			File_Handle >> total_vertices;

			///We need an array for all them vertices /// which have 8 data points each
			Vertices = new float[total_vertices * 8];
			for ( int i = 0; i < total_vertices * 8; ++i )
			{
				File_Handle >> Vertices[i];
			}

			///Immediately after the last vertex data point we will find the number of indices (ie. instructions for rendering the polygons)
			File_Handle >> total_indices;

			///We need an array for the indices
			Indices = new unsigned int[total_indices];
			for ( int i = 0; i < total_indices; ++i )
			{
				File_Handle >> Indices[i];
			}

			///Once we're done all that we should have a texture name
			File_Handle >> texture_file;

			///Something could have gone wrong
			if ( File_Handle.fail() )
				throw File_Handle.rdstate();

			///Nothing went wrong so we close the file
			File_Handle.close();

			///Let's put all this data into a single structure
			mesh_data data( total_vertices, total_indices, Vertices, Indices, texture_file );

			///The mesh constructor is thus much smaller and easier to read
			mesh* mymesh = new mesh( shader_3d, data );

			///Unless something catastrophic just went down we should have a mesh
			return mymesh;
		}
		catch ( ... )
		{
			///The assert says it all
			File_Handle.close();
			delete Vertices;
			delete Indices;
			assert( "Critical Failure Loading S3D File " );
		}
	}
	else
	{
		assert( "Could not Open S3D File" );
	}
	///It should not be possible to get here
	return nullptr;
}

//Queries the manager for a mesh matching the string parameter `file_name`
mesh* s3d_mesh_mgr::Get_Mesh( std::string file_name )
{
	mesh_iter = mesh_map.find( file_name );
	if ( mesh_iter != mesh_map.end() )
	{
		return mesh_iter->second;
	}
	return nullptr;
}
