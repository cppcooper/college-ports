#include <rover/core/global.h>
#include <rover/graphics/fundamentals/vbo_data.h>

//Prepares OpenGL for rendering this data
void render_data::init()
{
	if ( !initialized )
	{
		initialized = true;
		tex_id = get_game_engine().txtr_mgr.GetID( texture_file );

		/// Generate Vertex Buffer Object ID
		glGenBuffers( 1, &Vertex_VBO_id );
		/// Bind first VBO id to its purpose (Vertex Data)
		glBindBuffer( GL_ARRAY_BUFFER, Vertex_VBO_id );
		/// We need to upload the vertex data for the buffer
		glBufferData( GL_ARRAY_BUFFER, this->total_vertices * sizeof( float ) * 5, this->Vertices, GL_STATIC_DRAW );

		//###########################################
		glGenVertexArrays( 1, &VAO_id );
		glBindVertexArray( VAO_id );
		/// We don't know how to read our Vertex Data out of the VBO
		///This section will act as an instruction manual
		//=================================
		glEnableVertexAttribArray( 0 ); ///enable position attribute
		glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( float ) * 5, BUFFER_OFFSET(0) );///position
		//+++++++++++++++++++++++++++++++++
		glEnableVertexAttribArray( 1 ); ///enable normal attribute
		glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, sizeof( float ) * 5, BUFFER_OFFSET( sizeof( float ) * 3 ) );///uv
		//###########################################

	}
}

//Clears data out of the OpenGL buffers
void render_data::deinit()
{
	if ( initialized )
	{
		///free the vbo's
		glDeleteBuffers( 1, &Vertex_VBO_id );
		///free the vao
		glDeleteVertexArrays( 1, &VAO_id );
	}
}

//Prepares OpenGL for rendering this data
void mesh_data::init()
{
	if ( !initialized )
	{
		initialized = true;
		tex_id = get_game_engine().txtr_mgr.GetID( texture_file );

		/// Generate Vertex Buffer Object ID
		glGenBuffers( 1, &Vertex_VBO_id );
		/// Bind first VBO id to its purpose (Vertex Data)
		glBindBuffer( GL_ARRAY_BUFFER, Vertex_VBO_id );
		/// We need to upload the vertex data for the buffer
		glBufferData( GL_ARRAY_BUFFER, this->total_vertices * sizeof( float ) * 8, this->Vertices, GL_STATIC_DRAW );

		//###########################################
		glGenVertexArrays( 1, &VAO_id );
		glBindVertexArray( VAO_id );
		/// We don't know how to read our Vertex Data out of the VBO
		///This section will act as an instruction manual
		//=================================
		glEnableVertexAttribArray( 0 ); ///enable position attribute
		glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( float ) * 8, NULL );///position
		//+++++++++++++++++++++++++++++++++
		glEnableVertexAttribArray( 1 ); ///enable normal attribute
		glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, sizeof( float ) * 8, BUFFER_OFFSET( sizeof( float ) * 3 ) );///normal
		//+++++++++++++++++++++++++++++++++
		glEnableVertexAttribArray( 2 ); ///enable texture uv attribute
		glVertexAttribPointer( 2, 2, GL_FLOAT, GL_FALSE, sizeof( float ) * 8, BUFFER_OFFSET( sizeof( float ) * 6 ) );///uv
		//###########################################

		/// Similar process as above, but for Index data - ( ie. the order in which to draw vertices of triangles )
		/// We cannot setup the Indices until after the VAO
		glGenBuffers( 1, &Index_VBO_id );
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, Index_VBO_id );
		glBufferData( GL_ELEMENT_ARRAY_BUFFER, this->total_indices * sizeof( int ), this->Indices, GL_STATIC_DRAW );
	}
}

//Clears data out of the OpenGL buffers
void mesh_data::deinit()
{
	if ( initialized )
	{
		///free the vbo's
		glDeleteBuffers( 1, &Vertex_VBO_id );
		glDeleteBuffers( 1, &Index_VBO_id );
		///free the vao
		glDeleteVertexArrays( 1, &VAO_id );
	}
}
