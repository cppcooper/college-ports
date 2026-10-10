/*
v1.0.1
The MIT License (MIT)

Copyright (c) <2015> <Josh S, Cooper>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.*/


#ifndef ENGINE3D_H
#define ENGINE3D_H

#define _USE_MATH_DEFINES

#ifdef _WIN32
	#define WIN32_EXTRA_LEAN
	#include <Windows.h>
#endif

///C++ Libraries
#include <string>
#include <functional>
#include <unordered_set>
#include <iostream>
#include <sstream> 
#include <fstream>
//Threading Libraries
#include <thread>
#include <mutex>
#include <atomic>


///C Libraries
#include <stdio.h>
#include <conio.h>
#include <stdarg.h>
#include <Commdlg.h>
#include <cassert>


/// OpenGL Structures
#include <GL/glew.h> 
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp> 

#include <rover/graphics/fundamentals/GLSLProgram.h>

enum class engine_RenderMode { R2D = 0, R3D };
enum class engine_WindowModel { DECORATEDWINDOW = 1, FULLSCREEN, BORDERLESSFULLSCREEN};

///Function basically processes input
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);


///engine_3d is a class which is used to provide a basical game framework
// it provides 3 threads: Input, Update, Draw
///Input is within the main thread of execution for the application
//Update and Draw are both within their own new threads
class engine_3d
{
private:
	///Multi-Threading members
	bool Run_Threads = false;
	std::function<void()> Update_f;
	std::function<void()> Draw_f;
	std::thread Game_Update_thread;
	std::thread Game_Draw_thread;
	HDC currentDC;
	HGLRC currentContext;

	///Rendering mode member
	engine_RenderMode mode;
	std::atomic<int> screenWidth;
	std::atomic<int> screenHeight;
	engine_3d(engine_WindowModel windowMode = engine_WindowModel::DECORATEDWINDOW, int width = 1920, int height = 1080);

public:
	engine_WindowModel winMode;

	GLFWwindow* window;
	float nearplane, farplane;
	
	glm::mat4 projectionMatrix;
	glm::mat4 viewMatrix;

public:	

	~engine_3d();
	int get_ScreenWidth() const
	{
		return screenWidth;
	}
	int get_ScreenHeight() const
	{
		return screenHeight;
	}
	//Only one engine should exist..
	static engine_3d& Get_Instance()
	{
		static engine_3d engine( engine_WindowModel::DECORATEDWINDOW, 1000, 600 );
		return engine;
	}

	bool Init();
	void Quit();
	void ShowCursor(bool show);

	void set_Update_f( std::function<void()> Update_f );
	void set_Draw_f( std::function<void()> Draw_f );
	void Update();
	void Draw();
	void Run();

	void SetMode( engine_RenderMode newMode );
	void SetMode( engine_RenderMode newMode, GLSLProgram* shader );
};


#endif
