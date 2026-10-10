#include <rover/core/global.h>
#include <rover/graphics/2d/sprite.h>

//Associates the sprite to an array containing each frame of animation
sprite::sprite( GLSLProgram* shader, render_data opengl_data, bool animated, ushort frames )
{
	shader->bind();
	shader_2d = shader;
	this->animated = animated;
	this->frames = frames;
	this->opengl_data = opengl_data;
	this->opengl_data.init();

	frame_index = 0;
	dest_x = 0.f;
	dest_y = 0.f;
	angle = 0.f;
	alpha = 1.f;
}

//Deletes the allocated `graphic`s
sprite::~sprite()
{
	opengl_data.deinit();
}

//Sets the sprite frame and returns the object for further use
sprite& sprite::operator[]( ushort frame_number )
{
	frame_index = animated ? frame_number % frames : 0;
	return *this;
}

//Render a sprite frame at a particular location
void sprite::Draw( float dest_x, float dest_y )
{
	this->dest_x = dest_x;
	this->dest_y = dest_y;
	Draw();
}

//Renders a sprite frame
void sprite::Draw()
{
	glBindVertexArray( VAO_id );
	get_game_engine().txtr_mgr.BindTexture( tex_id );

	modelMatrix = glm::translate( glm::mat4( 1.f ), glm::vec3( dest_x, dest_y, 0.f ) );
	modelMatrix = glm::rotate( modelMatrix, angle, glm::vec3( 0.f, 0.f, 1.f ) );

	shader_2d->bind();
	shader_2d->setUniform( "modelMatrix", modelMatrix );
	shader_2d->setUniform( "in_Alpha", alpha );

	/// draw a quad: 1 quad x 4points per quad = 4 verts, the third argument
	glDrawArrays( GL_QUADS, frame_index * 4, 4 );

	/// bind with 0, so, switch back to normal pointer operation
	glBindVertexArray( 0 );
}
