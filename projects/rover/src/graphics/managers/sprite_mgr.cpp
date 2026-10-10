#include <fstream>
#include <rover/core/global.h>
#include <rover/graphics/managers/sprite_mgr.h>

//Sets the 2d shader to be used by any objects created after the method is invoked
void Sprite_Manager::set_shader( GLSLProgram* shader )
{
	shader_2d = shader;
}

//Loads all png files, that the File Manager knows about, which are within the graphic root directory
void Sprite_Manager::Load_All()
{
	/// prefix sl for string local type
	using slFile_Name = std::string;
	using slFile_Path = std::string;

	///Queries the File Manager for all PNG & DAT files
	//Map of all pngs, this will have all our sprites in it
	std::map<slFile_Name, slFile_Path> png_map = File_Manager::Get_Files( ".png" );
	//Iterates through PNG files
	std::map<slFile_Name, slFile_Path>::iterator png_iter = png_map.begin();

	//All animated sprites have a .dat file listing the coordinates of each frame
	std::map<slFile_Name, slFile_Path> data_map = File_Manager::Get_Files( ".dat" );
	//Points to a sprite data file if one was found for our current PNG
	std::map<slFile_Name, slFile_Path>::iterator data_iter = png_map.begin();

	///iterates through the list of PNG files
	for ( ; png_iter != png_map.end(); ++png_iter )
	{
		///We need to make sure this is a sprite and not just a texture
		if ( png_iter->second.find( sprites_dir ) != std::string::npos )
		{
			std::string sprite_name = png_iter->first.substr( 0, png_iter->first.find( ".png" ) );
			data_iter = data_map.find( sprite_name + ".dat" );

			if ( data_iter != data_map.end() )
			{
				///Animated
				std::string data_file = data_iter->second + data_iter->first;
				Load_Sprite_Sheet( png_iter->first, data_file );
			}
			else
			{
				///Non animated
				//Load the png as a sprite
				Load_Sprite( png_iter->first, sprite_name );
			}
		}
	}
}

//Loads one or more animated sprites, or two or more unanimated sprites
void Sprite_Manager::Load_Sprite_Sheet( std::string png_file, std::string sprite_data_file )
{
	///Request the OpenGL texture ID from the texture manager
	//Meaning the graphic file must have been loaded into the textures already
	GLuint texture_id = get_game_engine().txtr_mgr.GetID( png_file );
	///Request all the texture data
	texture* texture = get_game_engine().txtr_mgr.GetTexPtr( texture_id );
	float png_width = texture->width;
	float png_height = texture->height;

	std::ifstream sprite_data;
	sprite_data.open( sprite_data_file );
	if ( sprite_data.is_open() )
	{
		try
		{
			ushort total_sprites = 0;
			sprite_data >> total_sprites;

			//Loop until we've created each sprite
			for ( ushort i = 0; i < total_sprites; ++i )
			{
				ushort frames = 0;
				sprite_data >> frames;
				float* vertices = new float[20 * frames];

				ushort name_length = 0;
				sprite_data >> name_length;
				char* sprite_name_buffer = new char[name_length + 1];
				memset( sprite_name_buffer, 0, name_length + 1 );
				sprite_data.read( sprite_name_buffer, 1 );
				sprite_data.read( sprite_name_buffer, name_length );

				///Each frame will have 4 vertices, each vertice has 5 floats
				//Allocate all vertices for a single sprite as soon as we know how many frames the sprite has.

				//Loop until we've created each frame of this sprite
				for ( ushort f = 0; f < frames; ++f )
				{
					ushort start_x = 0, start_y = 0, width = 0, height = 0;
					sprite_data >> start_x;
					sprite_data >> start_y;
					sprite_data >> width;
					sprite_data >> height;

					///Bottom Left
					vertices[( f * 20 ) + 0] = 0;																	/// X
					vertices[( f * 20 ) + 1] = 0;																	/// Y
					vertices[( f * 20 ) + 2] = 0.0f;																/// Z
					vertices[( f * 20 ) + 3] = start_x / png_width;										/// U - Texture mapping
					vertices[( f * 20 ) + 4] = 1.0f - ( ( start_y + height ) / png_height );		/// V - Texture mapping (Inverted Axis?)

					///Bottom Right
					vertices[( f * 20 ) + 5] = width;
					vertices[( f * 20 ) + 6] = 0;
					vertices[( f * 20 ) + 7] = 0.0f;
					vertices[( f * 20 ) + 8] = ( start_x + width ) / png_width;
					vertices[( f * 20 ) + 9] = 1.0f - ( ( start_y + height ) / png_height );

					///Top Right
					vertices[( f * 20 ) + 10] = width;
					vertices[( f * 20 ) + 11] = height;
					vertices[( f * 20 ) + 12] = 0.0f;
					vertices[( f * 20 ) + 13] = ( start_x + width ) / png_width;
					vertices[( f * 20 ) + 14] = 1.0f - ( start_y / png_height );

					///Top Left
					vertices[( f * 20 ) + 15] = 0;
					vertices[( f * 20 ) + 16] = height;
					vertices[( f * 20 ) + 17] = 0.0f;
					vertices[( f * 20 ) + 18] = start_x / png_width;
					vertices[( f * 20 ) + 19] = 1.0f - ( start_y / png_height );
				}
				render_data opengl_data( 4 * frames, vertices );
				opengl_data.texture_file = png_file;
				opengl_data.tex_id = texture_id;

				sprite* s = new sprite( shader_2d, opengl_data, ( frames > 1 ), frames );
				sprite_map[std::string( sprite_name_buffer )] = s;
				delete sprite_name_buffer;
				delete vertices;
			}

			if ( sprite_data.fail() )
			{
				throw sprite_data.rdstate();
			}
		}
		catch ( ... )
		{
			assert( 0 );
		}
	}
}

//Loads a non animated sprite
void Sprite_Manager::Load_Sprite( std::string png_file, std::string sprite_name )
{
	///Request the OpenGL texture ID from the texture manager
	//Meaning the graphic file must have been loaded into the textures already
	GLuint texture_id = get_game_engine().txtr_mgr.GetID( png_file );
	///Request all the texture data
	texture* texture = get_game_engine().txtr_mgr.GetTexPtr( texture_id );
	float width = texture->width;
	float height = texture->height;

	///A square needs four vertices, `graphic`s are, technically speaking, squares
	float* vertices = new float[20];

	vertices[0] = 0;				/// X
	vertices[1] = height;		/// Y
	vertices[2] = 0.0f;			/// Z
	vertices[3] = 0.0f;			/// nU - Texture mapping
	vertices[4] = 1.0f;			/// nV - Texture mapping

	vertices[5] = 0;
	vertices[6] = 0;
	vertices[7] = 0.0f;
	vertices[8] = 0.0f;
	vertices[9] = 0.0f;

	vertices[10] = width;
	vertices[11] = 0;
	vertices[12] = 0.0f;
	vertices[13] = 1.0f;
	vertices[14] = 0.0f;

	vertices[15] = width;
	vertices[16] = height;
	vertices[17] = 0.0f;
	vertices[18] = 1.0f;
	vertices[19] = 1.0f;

	///Lets package the data up into "info"
	render_data opengl_data( 4, vertices );
	opengl_data.texture_file = png_file;
	opengl_data.tex_id = texture_id;

	///Load sprite with the above data
	sprite* s = new sprite( shader_2d, opengl_data, false, 1 );
	sprite_map[sprite_name] = s;
	delete vertices;
}

//Return a particular graphic if it has been loaded
sprite* Sprite_Manager::Get_Sprite( std::string sprite_name )
{
	sprite_iter = sprite_map.find( sprite_name );
	if ( sprite_iter != sprite_map.end() )
	{
		return sprite_iter->second;
	}
	return nullptr;
}
