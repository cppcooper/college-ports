#ifndef VBO_DATA_H
#define VBO_DATA_H

#include <rover/systems/OGLheaders.h>
#include <string>

//This data structure has the bare minimum that is required to render something
struct render_data
{
	std::string texture_file;
	GLuint tex_id;

	GLuint VAO_id;				///Vertex Array Object id
	GLuint Vertex_VBO_id;	///Vertex Buffer Object id
	GLuint total_vertices = 0;
	float* Vertices = nullptr;

	bool initialized = false;

	render_data(){}
	render_data( int nv, float* verts )
	{
		total_vertices = nv;
		this->Vertices = verts;
		
	}
	
	//Prepares OpenGL for rendering this data
	virtual void init();

	//Clears data out of the OpenGL buffers
	virtual void deinit();
};

//This data structure has the bare minimum that is required to render a complex mesh
struct mesh_data : render_data
{

	GLuint Index_VBO_id;		///Vertex Buffer Object id for the indexing of the vertices
	GLuint total_indices = 0;
	unsigned int* Indices = nullptr;

	mesh_data(){}
	mesh_data( int nv, int ni, float* verts, unsigned int* indices, std::string tx ) : render_data( nv, verts )
	{
		total_indices = ni;
		this->Indices = indices;
		texture_file = tx;
	}

	//Prepares OpenGL for rendering this data
	virtual void init();

	//Clears data out of the OpenGL buffers
	virtual void deinit();
};

#endif
