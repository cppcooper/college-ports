#ifndef GRAPHIC_H
#define GRAPHIC_H

#include <word/graphics/fundamentals/GLSLProgram.h>
#include <word/graphics/fundamentals/vbo_data.h>
#include <word/systems/OGLheaders.h>


using ushort = unsigned short;

//This class is intended for rendering all sorts of sprites/images
class sprite
{
	friend class SpriteBatch;
private:
	GLSLProgram* shader_2d = nullptr;

	render_data		opengl_data;
	///Aliases for the render_data members
	std::string		&texture_file = opengl_data.texture_file;
	GLuint			&tex_id = opengl_data.tex_id;

	GLuint			&VAO_id = opengl_data.VAO_id;

	GLuint			&Vertex_VBO_id = opengl_data.Vertex_VBO_id;
	GLuint			&total_vertices = opengl_data.total_vertices;
	float*				&Vertices = opengl_data.Vertices;
	///######

	glm::mat4 modelMatrix; /// Store the model matrix 

	ushort frame_index = 0;
	bool animated;
	ushort frames;

public:
	sprite( GLSLProgram* shader, render_data opengl_data, bool animated, ushort frames );
	~sprite();

	GLfloat scale = 1.0;
	GLfloat width;
	GLfloat height;
	GLfloat dest_x;
	GLfloat dest_y;
	GLfloat angle;
	GLfloat alpha;

	sprite& operator[]( ushort frame_number );
	void Scale( float radius );
	//scaled width
	GLfloat get_width() const { return width * scale; }
	//scaled height
	GLfloat get_height() const { return height * scale; }
	void Draw();
	void Draw( float dest_x, float dest_y );
};

#endif
