#include <rover/graphics/managers/texture_mgr.h>
#include <string>

//Loads all PNG files which the File Manager knows about
void Texture_Manager::LoadAll()
{
	/// prefix sl for string local type
	using slFile_Name = std::string;
	using slFile_Path = std::string;

	///Queries the File Manager for all PNG files
	std::map<slFile_Name, slFile_Path> path_map = File_Manager::Get_Files( ".png" );
	std::map<slFile_Name, slFile_Path>::iterator path_iter = path_map.begin();

	///iterates through the list of PNG files
	for ( ; path_iter != path_map.end(); ++path_iter )
	{
		///If we haven't already loaded a texture with a matching name
		if ( file_map.find( path_iter->first ) == file_map.end() )
		{
			///Load the texture from the following file path
			LoadTexture( path_iter->first, path_iter->second );
		}
	}
}

//Load a particular texture from a particular path
void Texture_Manager::LoadTexture( std::string file, std::string path )
{
	texture *newtex = new texture;

	///concat the two strings
	std::string fullpath;
	fullpath = path + file;
	newtex->refcount = 0;
	newtex->unload = true; ///currently setting all textures to unload when refcount = 0 (the next time);


	///Loaded Images file format
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
	FIBITMAP* FreeImage_texture_ptr = nullptr;
	BYTE* image_data = nullptr;
	unsigned int texture_width( 0 ), texture_height( 0 );

	///File Format from file structure & headers
	fif = FreeImage_GetFileType( fullpath.c_str(), 0 );
	///Might not have worked
	if ( fif == FIF_UNKNOWN )
	{
		///File Format from extension
		fif = FreeImage_GetFIFFromFilename( fullpath.c_str() );

		if ( fif == FIF_UNKNOWN )
		{
			///All attempts to determine format have failed
			goto ERROR_HANDLER;
		}
	}

	///Verify we can read the format
	if ( FreeImage_FIFSupportsReading( fif ) )
	{
		FreeImage_texture_ptr = FreeImage_Load( fif, fullpath.c_str() );
		if ( !FreeImage_texture_ptr )
		{
			///Couldn't load the image for unknown reason
			goto ERROR_HANDLER;
		}
	}
	else
	{
		///We can't read the format so a major problem exists
		goto ERROR_HANDLER;
	}

	if ( FreeImage_GetBPP( FreeImage_texture_ptr ) != 32 )
	{
		///Converts the image to 32-bit if it wasn't already
		FreeImage_texture_ptr = FreeImage_ConvertTo32Bits( FreeImage_texture_ptr );
	}

	///Retrieve the image data
	image_data = FreeImage_GetBits( FreeImage_texture_ptr );

	///Save the Image Dimensions for later use
	texture_width = FreeImage_GetWidth( FreeImage_texture_ptr );
	texture_height = FreeImage_GetHeight( FreeImage_texture_ptr );
	
	if ( ( image_data == 0 ) || ( texture_width == 0 ) || ( texture_height == 0 ) )
	{
		///Something went horribly wrong
		goto ERROR_HANDLER;
	}

	///We need to create an OpenGL texture ID to associate our data to
	glGenTextures( 1, &newtex->ID );

	///Map the File Name to the Texture ID
	this->file_map[file] = newtex->ID;
	///Map the Texture ID to the Texture Data
	this->texture_map[newtex->ID] = newtex;

	///Bind to the new texture ID
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, newtex->ID );
	Bound_IDs[0] = newtex->ID;

	///Set up some vars for OpenGL texturizing
	GLenum image_format = GL_RGBA;
	GLint internal_format = GL_RGBA;
	GLint level = 0;

	///Upload the image data for OpenGL to use
	glTexImage2D( GL_TEXTURE_2D, level, internal_format, texture_width, texture_height, 0, image_format, GL_UNSIGNED_BYTE, image_data );

	///Need to figure out what this does
	GLint swizzleMask[] = {GL_BLUE, GL_GREEN, GL_RED, GL_ALPHA};
	glTexParameteriv( GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask );
	//###########################################################


	///Do we ask OpenGL to optimize texture scaling or not?
	if ( useMipMaps )
	{
		glGenerateMipmap( GL_TEXTURE_2D );

		FreeImage_Unload( FreeImage_texture_ptr );
		newtex->width = texture_width;
		newtex->height = texture_height;
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST ); ///for when we are close
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );///when we are far away
	}
	else
	{
		FreeImage_Unload( FreeImage_texture_ptr );
		newtex->width = texture_width;
		newtex->height = texture_height;
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR ); ///for when we are close
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );///when we are far away
	}

	///The following turns on Anisotropy
	if ( GL_EXT_texture_filter_anisotropic )
	{
		GLfloat largest_supported_anisotropy;
		glGetFloatv( GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &largest_supported_anisotropy );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, largest_supported_anisotropy );
	}

	/// The texture stops at the edges with GL_CLAMP_TO_EDGE
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	return void( 0 );

ERROR_HANDLER:
	assert( 0 );
}

//Queries the manager for a texture name matching the string parameter `File`
GLuint Texture_Manager::GetID( std::string File, bool count_increase )
{
	///Search the std::map for a particular file name
	file_iter = file_map.find( File );
	if ( file_iter == file_map.end() )
	{
		///Didn't find the file - return an invalid ID number
		return 0;
	}
	else
	{
		///Found it, increase the reference count
		if ( count_increase ) texture_map[file_iter->second]->refcount++;
		///Return the ID
		return file_iter->second;
	}
}

//Retrieves the texture data associated to the ID passed in
texture* Texture_Manager::GetTexPtr( texid ID )
{
	///Search the std::map for a particular ID
	auto iter = texture_map.find( ID );
	if ( iter != texture_map.end() )
	{
		///Found it, return the pointer to the Texture Data
		return iter->second;
	}
	///Did not find it, return a null pointer
	return nullptr;
}

//A method to allow users to manually free textures from memory
void Texture_Manager::FreeTexture( GLuint id )
{
	///lookup this texture in our std::map
	texture_iter = texture_map.find( id );

	if ( texture_iter != texture_map.end() )
	{
		///We found it in the std::map
		///update the refcount
		texture_iter->second->refcount--; 

		if ( texture_iter->second->refcount <= 0 && texture_iter->second->unload )
		{
			///It was the last reference of this texture in use and it wants to be unloaded now

			///Time delete this texture from memory
			glDeleteTextures( 1, &id );

			///if this was the currently bound texture, set currentId to a bad ID value
			///that won't be matched by the next call to BindTexture()
			for ( int i = 0; i < TEXTURE_MANAGER_MAX_TEXTURES; ++i )
				if ( Bound_IDs[i] == texture_iter->second->ID )
					Bound_IDs[i] = 0;

			delete texture_iter->second; ///free the instance of a tex struct

			texture_iter->second = nullptr;
			///clear the texture from the std::map
			texture_map.erase( texture_iter );
		}
	}
}

//Bind the texture ID for rendering
void Texture_Manager::BindTexture( GLuint id, GLuint texture_unit )
{
	if ( Bound_IDs[texture_unit - GL_TEXTURE0] != id )
	{
		glActiveTexture( texture_unit ); ///needed for programmable shaders
		glBindTexture( GL_TEXTURE_2D, id );

		Bound_IDs[texture_unit - GL_TEXTURE0] = id;
	}
}
