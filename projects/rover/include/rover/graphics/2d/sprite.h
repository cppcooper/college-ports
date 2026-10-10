#ifndef GRAPHIC_H
#define GRAPHIC_H

#include <rover/graphics/fundamentals/GLSLProgram.h>
#include <rover/graphics/fundamentals/vbo_data.h>
#include <rover/systems/OGLheaders.h>


using ushort = unsigned short;

//This class is intended for rendering all sorts of sprites/images
class sprite
{
private:
	GLSLProgram* shader_2d = nullptr;

	render_data		opengl_data;
	///Aliases for the render_data members
	std::string		&texture_file = opengl_data.texture_file;
	GLuint				&tex_id = opengl_data.tex_id;

	GLuint				&VAO_id = opengl_data.VAO_id;

	GLuint				&Vertex_VBO_id = opengl_data.Vertex_VBO_id;
	GLuint				&total_vertices = opengl_data.total_vertices;
	float*				&Vertices = opengl_data.Vertices;
	///######

	glm::mat4 modelMatrix; /// Store the model matrix 

	ushort frame_index = 0;
	bool animated;
	ushort frames;

public:
	sprite( GLSLProgram* shader, render_data opengl_data, bool animated, ushort frames );
	~sprite();

	GLfloat dest_x;
	GLfloat dest_y;
	GLfloat angle;
	GLfloat alpha;

	sprite& operator[]( ushort frame_number );
	void Draw();
	void Draw( float dest_x, float dest_y );
};

#endif
