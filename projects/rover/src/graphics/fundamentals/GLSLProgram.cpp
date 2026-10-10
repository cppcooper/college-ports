#include <rover/core/global.h>
#include <rover/graphics/fundamentals/GLSLProgram.h>

///#include "glutils.h"

#include <fstream>
using std::ifstream;
using std::ios;

#include <sstream>
using std::ostringstream;

#include <sys/stat.h>
/// Authored by: Darren Reid
// Code has been revised, redesigned, and refactored by: Josh Cooper
/// TLDR: Many modifications by Josh Cooper
GLSLProgram::GLSLProgram()
{
	prog_handle = -1;
	linked = false;
	shader_handle = 0;
}

/// Authored by: Darren Reid
// Code has been revised, redesigned, and refactored by: Josh Cooper
/// TLDR: Many modifications by Josh Cooper
GLSLProgram::~GLSLProgram()
{
	if (prog_handle)
	{
		glDeleteProgram(prog_handle);
	}
}

/// Authored by: Darren Reid
// Code has been revised, redesigned, and refactored by: Josh Cooper
/// TLDR: Many modifications by Josh Cooper
bool GLSLProgram::compileShaderFromFile( std::string fileName, GLSLShader::GLSLShaderType type )
{
	if ( prog_handle <= 0 )
	{
		prog_handle = glCreateProgram();
		if ( prog_handle == 0 )
		{
			logString = "Unable to create shader program.";
			return false;
		}
	}

	ifstream inFile( shaders_dir + fileName, ios::in );

	if ( inFile.is_open() )
	{
		inFile.seekg( 0, inFile.end );
		unsigned int length = inFile.tellg();
		char* buffer = new char[length+1];
		memset( buffer, 0, length + 1 );

		inFile.seekg( 0, inFile.beg );
		inFile.read( buffer, length );
		if ( inFile.bad() )
		{
			delete buffer;
			logString = "Unable to read file.";
			return false;
		}
		std::string code( buffer );
		delete buffer;
		return compileShaderFromString( code, type );
	}

	logString = "File not found.";
	return false;
}

/// Authored by: Darren Reid
// Code has been revised, redesigned, and refactored by: Josh Cooper
/// TLDR: Many modifications by Josh Cooper
bool GLSLProgram::compileShaderFromString( std::string &source_code, GLSLShader::GLSLShaderType type )
{
	if ( prog_handle <= 0 )
	{
		prog_handle = glCreateProgram();
		if ( prog_handle == 0 )
		{
			logString = "Unable to create shader program.";
			return false;
		}
	}

	///Creating the shader handle
	switch ( type )
	{
		case GLSLShader::VERTEX:
			shader_handle = glCreateShader( GL_VERTEX_SHADER );
			break;
		case GLSLShader::FRAGMENT:
			shader_handle = glCreateShader( GL_FRAGMENT_SHADER );
			break;
		case GLSLShader::GEOMETRY:
			shader_handle = glCreateShader( GL_GEOMETRY_SHADER );
			break;
		case GLSLShader::TESS_CONTROL:
			shader_handle = glCreateShader( GL_TESS_CONTROL_SHADER );
			break;
		case GLSLShader::TESS_EVALUATION:
			shader_handle = glCreateShader( GL_TESS_EVALUATION_SHADER );
			break;
		default:
			logString = "Unknown shader type.";
			return false;
	}

	///Compile the shader
	const char * cstr_code = source_code.c_str();
	glShaderSource( shader_handle, 1, &cstr_code, NULL );
	glCompileShader( shader_handle );

	/// Check for errors
	int result;
	glGetShaderiv( shader_handle, GL_COMPILE_STATUS, &result );
	if ( GL_FALSE == result )
	{
		/// Compile failed, store log and return false
		int length = 0;
		logString = "";
		glGetShaderiv( shader_handle, GL_INFO_LOG_LENGTH, &length );

		if ( length > 0 )
		{
			char * c_log = new char[length];
			int written = 0;
			glGetShaderInfoLog( shader_handle, length, &written, c_log );
			logString = c_log;
			delete[] c_log;
		}
		return false;
	}
	else
	{
		/// Compile succeeded, attach shader and return true
		glAttachShader( prog_handle, shader_handle );
		return true;
	}
}

/// Authored by: Darren Reid
// Code has been revised, redesigned, and refactored by: Josh Cooper
/// TLDR: Many modifications by Josh Cooper
bool GLSLProgram::link()
{
	if ( linked )
	{
		return true;
	}
	else if ( prog_handle <= 0 )
	{
		return false;
	}
	else
	{
		///Program is not already linked, and is linkable
		glLinkProgram( prog_handle );

		///Gotta verify the link went Okay
		int status = 0;
		glGetProgramiv( prog_handle, GL_LINK_STATUS, &status );
		if ( GL_FALSE == status )
		{
			///It failed, we need logs

			///Get the length of the log
			int length = 0;
			glGetProgramiv( prog_handle, GL_INFO_LOG_LENGTH, &length );

			if ( length > 0 )
			{
				///The log has a non zero size
				// So allocate space for the log temporarily
				char * c_log = new char[length];
				int written = 0;
				glGetProgramInfoLog( prog_handle, length, &written, c_log );
				logString = c_log;
				delete[] c_log;
			}
			return false;
		}
		else
		{
			/// It didn't fail
			linked = true;
			return true;
		}
	}
}

/// Authored by: Darren Reid
// Code has been revised, redesigned, and refactored by: Josh Cooper
/// TLDR: Many modifications by Josh Cooper
void GLSLProgram::bind()
{
	assert( link() );
    glUseProgram( prog_handle );
}



// Code Below by: Darren Reid
/// No changes made from original Source Code

string GLSLProgram::log()
{
    return logString;
}

int GLSLProgram::getHandle()
{
    return prog_handle;
}

bool GLSLProgram::isLinked()
{
    return linked;
}

void GLSLProgram::bindAttribLocation( GLuint location, const char * name)
{
    glBindAttribLocation(prog_handle, location, name);
}

void GLSLProgram::bindFragDataLocation( GLuint location, const char * name )
{
    glBindFragDataLocation(prog_handle, location, name);
}

void GLSLProgram::setUniform(const char *name, float x, float y)
{
	int loc = getUniformLocation(name);
	assert(loc >= 0 && "setUniform failed");
	if (loc >= 0)
	{
		glUniform2f(loc, x, y);
	}
}

//ALL `setUniform`s upload values to the shader in OpenGL
void GLSLProgram::setUniform( const char *name, float x, float y, float z)
{
    int loc = getUniformLocation(name);
	assert(loc >= 0 && "setUniform failed");
    if( loc >= 0 ) 
	{
        glUniform3f(loc,x,y,z);
    }
}

void GLSLProgram::setUniform(const char *name, const vec2 & v)
{
	this->setUniform(name, v.x, v.y);
}

void GLSLProgram::setUniform( const char *name, const vec3 & v)
{
    this->setUniform(name,v.x,v.y,v.z);
}

void GLSLProgram::setUniform( const char *name, const vec4 & v)
{
    int loc = getUniformLocation(name);
	assert(loc >= 0 && "setUniform failed");
    if( loc >= 0 ) 
	{
        glUniform4f(loc,v.x,v.y,v.z,v.w);
    }
}

void GLSLProgram::setUniform( const char *name, const mat4 & m)
{
    int loc = getUniformLocation(name);
	assert(loc >= 0 && "setUniform failed");
    if( loc >= 0 )
    {
        glUniformMatrix4fv(loc, 1, GL_FALSE, &m[0][0]);
    }
}

void GLSLProgram::setUniform( const char *name, const mat3 & m)
{
    int loc = getUniformLocation(name);
	assert(loc >= 0 && "setUniform failed");
    if( loc >= 0 )
    {
        glUniformMatrix3fv(loc, 1, GL_FALSE, &m[0][0]);
    }
}

void GLSLProgram::setUniform( const char *name, float val )
{
    int loc = getUniformLocation(name);
	assert(loc >= 0 && "setUniform failed");
    if( loc >= 0 )
    {
        glUniform1f(loc, val);
    }
}

void GLSLProgram::setUniform( const char *name, int val )
{
    int loc = getUniformLocation(name);
	assert(loc >= 0 && "setUniform failed");
    if( loc >= 0 )
    {
        glUniform1i(loc, val);
    }
}

void GLSLProgram::setUniform( const char *name, bool val )
{
    int loc = getUniformLocation(name);
	assert(loc >= 0 && "setUniform failed");
    if( loc >= 0 )
    {
        glUniform1i(loc, val);
    }
}

void GLSLProgram::printActiveUniforms() {

    GLint nUniforms, size, location, maxLen;
    GLchar * name;
    GLsizei written;
    GLenum type;

    glGetProgramiv( prog_handle, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxLen);
    glGetProgramiv( prog_handle, GL_ACTIVE_UNIFORMS, &nUniforms);

    name = (GLchar *) malloc( maxLen );

    printf(" Location | Name\n");
    printf("------------------------------------------------\n");
    for( int i = 0; i < nUniforms; ++i ) {
        glGetActiveUniform( prog_handle, i, maxLen, &written, &size, &type, name );
        location = glGetUniformLocation(prog_handle, name);
        printf(" %-8d | %s\n",location, name);
    }

    free(name);
}

void GLSLProgram::printActiveAttribs() {

    GLint written, size, location, maxLength, nAttribs;
    GLenum type;
    GLchar * name;

    glGetProgramiv(prog_handle, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxLength);
    glGetProgramiv(prog_handle, GL_ACTIVE_ATTRIBUTES, &nAttribs);

    name = (GLchar *) malloc( maxLength );

    printf(" Index | Name\n");
    printf("------------------------------------------------\n");
    for( int i = 0; i < nAttribs; i++ ) {
        glGetActiveAttrib( prog_handle, i, maxLength, &written, &size, &type, name );
        location = glGetAttribLocation(prog_handle, name);
        printf(" %-5d | %s\n",location, name);
    }

    free(name);
}

int GLSLProgram::getUniformLocation(const char * name )
{
	///return glGetUniformLocation(handle, name);
	bind();
	return GetUniform(name);
}

int GLSLProgram::GetUniform(const char* name)
{
	int result = -1;

	if (linked)
	{
		UMapIter = UniformMap.find(name);
		if (UMapIter == UniformMap.end())
		{
			result = glGetUniformLocation(prog_handle, name);
			
			if (result != -1) UniformMap[name] = result;
		}
		else
		{		
			result = UMapIter->second;
		}
	}

	return result;
}

int GLSLProgram::GetAttribute(const char* name)
{
	int result = -1;

	if (linked)
	{
		UMapIter = UniformMap.find(name);
		if (UMapIter == UniformMap.end())
		{
			result = glGetAttribLocation(prog_handle, name);

			if (result != -1) UniformMap[name] = result;
		}
		else
		{
			result = UMapIter->second;
		}
	}

	return result;
}
