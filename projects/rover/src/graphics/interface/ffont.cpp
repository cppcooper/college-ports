#include <rover/core/global.h>
#include <rover/core/log.h>
#include <rover/graphics/interface/ffont.h>
#include <string>


//Loads the font png, reads the width data, prepares the object for rendering letters
FFont::FFont( GLSLProgram* shader )
{
	shader->bind();
	font_shader = shader;

	font_size = 16.0f;
	text_angle = 0.0f;
	text_alpha = 1.0f;

	info.total_vertices = 4 * 256;
	float* verts = new float[5 * info.total_vertices];
	info.Vertices = verts;
	info.texture_file = "font.png";
	get_game_engine().txtr_mgr.LoadTexture( texture_file , fonts_dir );


	///This essentially sets up the texture mappings for each letter
	for ( int loop = 0; loop<256; loop++ )
	{
		float cx = ( (float)( loop % 16 ) ) / 16.0f;				/// X Position Of Current Character
		float cy = ( (float)( loop / 16 ) ) / 16.0f;				/// Y Position Of Current Character

		verts[( loop * 20 )] = 0;
		verts[1 + ( loop * 20 )] = 0;		/// Vertex Coord (Bottom Left)
		verts[2 + ( loop * 20 )] = 0;
		verts[3 + ( loop * 20 )] = cx;
		verts[4 + ( loop * 20 )] = 1 - ( cy + 1.f / 16 );	/// Texture Coord (Bottom Left)

		verts[5 + ( loop * 20 )] = 1;
		verts[6 + ( loop * 20 )] = 0;	/// Vertex Coord (Bottom Right)
		verts[7 + ( loop * 20 )] = 0;
		verts[8 + ( loop * 20 )] = cx + ( 1.f / 16 );
		verts[9 + ( loop * 20 )] = 1 - ( cy + 1.f / 16 );	/// Texture Coord (Bottom Right)

		verts[10 + ( loop * 20 )] = 1;
		verts[11 + ( loop * 20 )] = 1;	/// Vertex Coord (Top Right)
		verts[12 + ( loop * 20 )] = 0;
		verts[13 + ( loop * 20 )] = cx + ( 1.f / 16 );
		verts[14 + ( loop * 20 )] = 1 - cy;	/// Texture Coord (Top Right)

		verts[15 + ( loop * 20 )] = 0;
		verts[16 + ( loop * 20 )] = 1;		/// Vertex Coord (Top Left)
		verts[17 + ( loop * 20 )] = 0;
		verts[18 + ( loop * 20 )] = cx;
		verts[19 + ( loop * 20 )] = 1 - cy;	/// Texture Coord (Top Left)
	}

	info.init();

	///We need to know the width of each letter
	std::ifstream data_file;
	data_file.open( fonts_dir + "font.dat", std::ios::in | std::ios::binary );
	if ( !data_file.is_open() )
	{
		LOGFILE1( dbg_log::logERROR ) << "Error while loading widths data file: 'fontMetrics.dat' for FFont";
		assert( data_file.is_open() );
	}

	short buffer[256]; ///memory to read into
	char *buff = (char *)buffer;

	data_file.seekg( 0, std::ios::beg );///seek to start

	///load entire file in one go...much faster than reading 2 chars at a time!
	data_file.read( buff, 512 );
	data_file.close();

	for ( int i = 0; i < 256; ++i )
	{
		widths[i] = (_int32)buffer[i];
	}

	//#############
}

//Returns the width of the string if rendered in the font loaded in FFont
float FFont::WidthText( std::string text )
{
	int letter;
	float width_text = 0;
	float scale = font_size / 128;

	for ( unsigned int i = 0; i < text.size(); ++i )
	{
		letter = Fancy_Font ? text[i] - 32 + 128 : text[i] - 32;

		width_text += widths[letter] * scale;
	}

	return width_text;
}

//Render strings in OpenGL using the font texture
void FFont::print( std::string text )
{
	//get_cooper3d().SetMode( engine_RenderMode::R2D, font_shader );

	///Prepare our OpenGL objects in the buffer for rendering
	glBindVertexArray( VAO_id );
	get_game_engine().txtr_mgr.BindTexture( tex_id );

	///Setting up our initial position
	modelMatrix = glm::translate( glm::mat4( 1.f ), text_position );
	modelMatrix = glm::rotate( modelMatrix, text_angle, glm::vec3( 0.f, 0.f, 1.f ) );

	font_shader->bind();
	font_shader->setUniform( "in_Scale", font_size );
	font_shader->setUniform( "in_Alpha", text_alpha );

	float scale = font_size / 128;
	///We must render one letter at a time
	for ( int i = 0, letter = 0; i < text.size(); ++i )
	{
		font_shader->setUniform( "modelMatrix", modelMatrix );
		letter = Fancy_Font ? text[i] - 32 + 128 : text[i] - 32;

		if ( text[i] == '\n' )
		{
			text_position.y -= ( font_size / 2 );
			modelMatrix = glm::translate( glm::mat4( 1.f ), text_position );
			modelMatrix = glm::rotate( modelMatrix, text_angle, glm::vec3( 0.f, 0.f, 1.f ) );
		}
		else
		{
			///Render quad
			glDrawArrays( GL_QUADS, letter * 4, 4 );
			///Move to the next position
			modelMatrix = glm::translate( modelMatrix, glm::vec3( (float)widths[letter] * scale, 0.f, 0.f ) );
		}
	}
	font_shader->setUniform( "in_Scale", 1.f );
	glBindVertexArray( 0 );
}
