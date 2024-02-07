/*
File:   pig_graphics_opengl.cpp
Author: Taylor Robbins
Date:   02\06\2024
Description: 
	** None 
*/

#include "glad/glad.c"

static bool _PigGfx_GladInit()
{
	// WriteLine_R("Initializing GLAD..."); //TODO: Re-Enable me!
	
	#if PIG_GFX_GLFW_SUPPORTED
	NotNull(gfx->glfwWindowPntr);
	glfwMakeContextCurrent(gfx->glfwWindowPntr);
	#else
	#error OPENGL only supports GLFW as the windowing API right now. Please set PIG_GFX_GLFW_SUPPORTED to 1 or update this function!
	#endif
	
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		// Win32_InitError("Could not initialize GLAD. gladLoadGLLoader failed"); //TODO: Put this error code somewhere?
		return false;
	}
	
	//TODO: Use these globals that are now populated
	// GLVersion.major
	// GLVersion.minor
	// if (GLAD_GL_VERSION_3_0) { /* at least OpenGL 3.0 is supported */ }
	// if (GLAD_GL_EXT_framebuffer_multisample) { /* it's supported */ }
	
	// WriteLine_R("GLAD Init Succeeded!"); //TODO: Re-Enable me!
	return true;
}

bool PigGfx_Init_OpenGL()
{
	return true;
}

bool PigGfx_CreateContextInWindow_OpenGL()
{
	if (!_PigGfx_GladInit()) { return false; }
	return true;
}
