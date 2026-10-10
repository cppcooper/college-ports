#include <rover/graphics/managers/Shader_Manager.h>
#include <cassert>

/*
Darren's Shader Manager
with slight modifications (such as comments)

Based on Version 1.1
*/

//Deconstructor cleans up the block allocations for the Shaders
ShaderManager::~ShaderManager()
{
	for (auto item : ShaderMap)
	{
		delete item.second;
	}
}

//Loads and compiles vertice and fragment shaders from Files into one GLSLProgram*
GLSLProgram* ShaderManager::Load(std::string vertName, std::string fragName) 
{
	GLSLProgram* prog = new GLSLProgram();
	
	///Compile the vertex portion of the shader
	if (!prog->compileShaderFromFile(vertName, GLSLShader::VERTEX))
	{
		printf("Vertex shader failed to compile!\n%s", prog->log().c_str());
		assert(false && "Vertex shader failed to compile");
		return NULL;
	}

	///Compile the fragment portion of the shader
	if (!prog->compileShaderFromFile(fragName, GLSLShader::FRAGMENT))
	{
		printf("Fragment shader failed to compile!\n%s", prog->log().c_str());
		assert(false && "Fragment shader failed to compile");
		return NULL;
	}

	///Links the shader to OpenGL using the handle obtained during the compile
	if ( !prog->link() )
	{
		printf("Shader program failed to link!\n%s", prog->log().c_str());
		assert(false && "Shader program failed to link.");
		return NULL;
	}

	assert(prog != NULL);
	return prog;
}

//Loads and compiles vertice and fragment shaders from strings into one GLSLProgram*
GLSLProgram* ShaderManager::LoadFromStrings(std::string vertName, std::string fragName, std::string vertString, std::string fragString)
{
	GLSLProgram* prog = new GLSLProgram();

	///Compile the vertex portion of the shader
	if(!prog->compileShaderFromString(vertString, GLSLShader::VERTEX))
	{
		printf("Vertex shader failed to compile from string!\n%s", prog->log().c_str());
		assert(false && "Vertex shader failed to compile from string");
		return NULL;
	}

	///Compile the fragment portion of the shader
	if(!prog->compileShaderFromString(fragString, GLSLShader::FRAGMENT))
	{
		printf("Fragment shader failed to compile from string!\n%s", prog->log().c_str());
		assert(false && "Fragment shader failed to compile from string");
		return NULL;
	}

	///Links the shader to OpenGL using the handle obtained during the compile
	if ( !prog->link() )
	{
		printf("Shader program failed to link!\n%s", prog->log().c_str());
		assert(false && "Shader program failed to link.");
		return NULL;
	}

	assert(prog != NULL);
	return prog;
}

//Searches for the corresponding Vertex/Fragment shader pair
GLSLProgram* ShaderManager::GetShader(std::string vertName, std::string fragName)
{
	std::string key = vertName;
	key += fragName;


	shaderIter = ShaderMap.find(key);
	if (shaderIter != ShaderMap.end())
	{
		///The shader exists to we return it
		return (shaderIter->second);
	}
	else
	{
		///The shader doesn't exist so we load it
		GLSLProgram* prog = Load(vertName, fragName);
		if (prog != NULL)
		{
			/// successful loaded and linked shader program, added to map and return the result
			ShaderMap[key] = prog;

			///It loaded successfully now we return it
			return prog;
		}
		assert(prog != NULL);
	}

	/// ERROR
	return NULL;
}
