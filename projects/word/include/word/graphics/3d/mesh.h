#ifndef S3DMESH_H_
#define S3DMESH_H_
#include <vector>
#include <fstream>
#include <mutex>
#include <word/core/log.h>
#include <word/graphics/fundamentals/GLSLProgram.h>
#include <word/graphics/fundamentals/vbo_data.h>

using std::string;
using std::ifstream;

//Bare minimum class for rendering a mesh
class mesh
{
private:
	GLSLProgram*	shader_3d = nullptr;
	bool bStripped = true;

	mesh_data		info;
	///Aliases for `mesh_data` members
	std::string		&texture_file		= info.texture_file;
	GLuint				&tex_id					= info.tex_id;

	GLuint				&VAO_id				= info.VAO_id;

	GLuint				&Vertex_VBO_id	= info.Vertex_VBO_id;
	GLuint				&total_vertices	= info.total_vertices;
	float*				&Vertices				= info.Vertices;

	GLuint				&Index_VBO_id	= info.Index_VBO_id;
	GLuint				&total_indices		= info.total_indices;
	unsigned int*	&Indices				= info.Indices;

protected:
	void Load( string file_path );

	///bool deleteme = false;

public:
	mesh( GLSLProgram* shader, mesh_data info, bool isStripped = true );
	~mesh();

	void draw( glm::mat4& modelMatrix );
};

#endif
