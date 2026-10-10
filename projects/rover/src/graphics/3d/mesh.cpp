#include <sstream>
#include <atomic>

#include <rover/core/global.h>
#include <rover/graphics/3d/mesh.h>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

using std::stringstream;

//Creates the mesh
mesh::mesh( GLSLProgram* shader, mesh_data info, bool isStripped )
{
	shader->bind(); ///Cannot remove this line (nothing will draw)
	this->info = info;
	this->info.init();

	shader_3d = shader;
	bStripped = isStripped;
}

//Destroy the mesh data and clear it from OpenGL
mesh::~mesh()
{
	info.deinit();
	delete Vertices;
	delete Indices;
}

//Renders the mesh
void mesh::draw( glm::mat4& modelMatrix )
{
	shader_3d->bind();
	shader_3d->setUniform( "modelMatrix", modelMatrix );

	glBindVertexArray( VAO_id );
	get_game_engine().txtr_mgr.BindTexture( tex_id );

	if ( bStripped )
	{
		glDrawElements( GL_TRIANGLE_STRIP, this->total_indices, GL_UNSIGNED_INT, BUFFER_OFFSET( 0 ) );
	}
	else
	{
		glDrawElements( GL_TRIANGLES, this->total_indices, GL_UNSIGNED_INT, BUFFER_OFFSET( 0 ) );
	}
	glBindVertexArray( 0 );
}
