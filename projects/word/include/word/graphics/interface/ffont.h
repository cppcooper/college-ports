#ifndef _FONT_HANDLER_H_
#define _FONT_HANDLER_H_

#include <iof_chunk.h>
#include <map>
#include <word/graphics/fundamentals/GLSLProgram.h>
#include <word/graphics/fundamentals/vbo_data.h>
#include <word/systems/OGLheaders.h>

//This class is used for rendering a font to the screen ( Just one font, there aren't multiple fonts to load so the functionality wouldn't be needed.. )
class FFont
{
	GLSLProgram* font_shader = nullptr;

	render_data		info;
	///Aliases for render_data members
	std::string		&texture_file = info.texture_file;
	GLuint				&tex_id = info.tex_id;

	GLuint				&VAO_id = info.VAO_id;

	GLuint				&Vertex_VBO_id = info.Vertex_VBO_id;
	GLuint				&total_vertices = info.total_vertices;
	float*				&Vertices = info.Vertices;
	///########

	glm::mat4 modelMatrix; /// Store the model matrix
	int widths[256];

protected:
	float FFont::WidthText( std::string text );
	glm::vec3 cursor_position;

public:
	bool Fancy_Font = false;
	glm::vec3 text_position;
	float font_size;
	float text_angle;
	float text_alpha;

	FFont( GLSLProgram* shader );

	///Sets up the drawing information
	FFont( float font_size = 16.0f, bool Fancy_Font = false, float text_alpha = 1.0f, float text_angle = 0.0f )
	{
		this->text_angle = text_angle;
		this->text_alpha = text_alpha;
		this->font_size = font_size;
		this->Fancy_Font = Fancy_Font;
	}
	void set_properties( float font_size = 16.0f, bool Fancy_Font = false, float text_alpha = 1.0f, float text_angle = 0.0f )
	{
		this->text_angle = text_angle;
		this->text_alpha = text_alpha;
		this->font_size = font_size;
		this->Fancy_Font = Fancy_Font;
	}

	///Copies particular draw/cursor properties from one FFont to another
	FFont& operator=( FFont& other )
	{
		this->cursor_position = glm::vec3( 0.0f );
		this->text_position = other.text_position;
		this->text_angle = other.text_angle;
		this->text_alpha = other.text_alpha;
		this->font_size = other.font_size;
		this->Fancy_Font = other.Fancy_Font;
		return *this;
	}
					//Prepare the font to be rendered
	void Initialize()														{ info.init(); }
					//Print a string to the screen using the draw properties and our Font Texture
	void print( std::string text );
	void print( char letter );
	void print( float x, float y, std::string text )
	{
		text_position.x = x + ( 0.08f * font_size );
		text_position.y = y - ( 0.3f * font_size );
		print( text );
	}
	void print( float x, float y, char letter )
	{
		text_position.x = x + ( 0.08f * font_size );
		text_position.y = y - ( 0.3f * font_size );
		std::string text;
		text += letter;
		print( text );
	}
					//Clear the rendering data from the OpenGL buffer
	~FFont()																{ info.deinit(); delete Vertices; }
};

extern FFont* Fcout;

#endif
