#ifndef GLSLPROGRAM_H
#define GLSLPROGRAM_H

#include <GL/glew.h>
#include <GL/gl.h>

#include <string>
using std::string;

#include <glm/glm.hpp>
using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat4;
using glm::mat3;

#include <map>

namespace GLSLShader {
    enum GLSLShaderType {
        VERTEX, FRAGMENT, GEOMETRY,
        TESS_CONTROL, TESS_EVALUATION
    };
};

class GLSLProgram
{
private:
	GLuint shader_handle;
    int  prog_handle;
    bool linked;
    string logString;

    int  getUniformLocation(const char * name );

	///Store uniforms and attributes in a map for easy lookup
	std::map<std::string, int> UniformMap;
	std::map<std::string, int>::iterator UMapIter;

public:

	/// Authored by: Darren Reid
	// Code has been revised, redesigned, and refactored by: Josh Cooper
	/// TLDR: Many modifications by Josh Cooper
    GLSLProgram();
	~GLSLProgram();

	/// Authored by: Darren Reid
	// Code has been revised, redesigned, and refactored by: Josh Cooper
	/// TLDR: Many modifications by Josh Cooper
    bool   compileShaderFromFile( std::string fileName, GLSLShader::GLSLShaderType type );
    bool   compileShaderFromString( std::string &source_code, GLSLShader::GLSLShaderType type );
    bool   link();
    void   bind();

	/*
		Code Below by: Darren Reid
		No changes made from original Source Code
	*/

    string log();

    int    getHandle();
    bool   isLinked();

    void   bindAttribLocation( GLuint location, const char * name);
    void   bindFragDataLocation( GLuint location, const char * name );
	
	//Uploads values to the shader in OpenGL
	void   setUniform(const char *name, float x, float y);
    void   setUniform( const char *name, float x, float y, float z);
	void   setUniform( const char *name, const vec2 & v);
    void   setUniform( const char *name, const vec3 & v);
    void   setUniform( const char *name, const vec4 & v);
    void   setUniform( const char *name, const mat4 & m);
    void   setUniform( const char *name, const mat3 & m);
    void   setUniform( const char *name, float val );
    void   setUniform( const char *name, int val );
    void   setUniform( const char *name, bool val );

	//Prints to the console
    void   printActiveUniforms();
    void   printActiveAttribs();

	int GetUniform(const char* name);
	int GetAttribute(const char* name);
};

#endif