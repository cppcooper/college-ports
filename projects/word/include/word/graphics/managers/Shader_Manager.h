#ifndef _SHADER_MGR_H
#define _SHADER_MGR_H

/*
Darren's Shader Manager
with slight modifications
(eg. Removal of three redundant methods)

Based on Version 1.1
*/

#include <word/graphics/fundamentals/GLSLProgram.h>

class ShaderManager
{
private:
	std::map<std::string, GLSLProgram*> ShaderMap;

	GLSLProgram* Load(std::string vertName, std::string fragName);
	GLSLProgram*LoadFromStrings(std::string vertName, std::string fragName, std::string vertString, std::string fragString);

	std::map<std::string, GLSLProgram*>::iterator shaderIter;

public:
	///Try to retrive a shader: if none exists for this combination or vert and frag shaders, load and 
	///compile and link it, then store on map
	GLSLProgram* GetShader(std::string vertName, std::string fragName);

	~ShaderManager();
};

#endif
