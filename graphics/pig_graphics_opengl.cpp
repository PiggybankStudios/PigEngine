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
	
	Assert(gfx->optionsSet);
	Assert(gfx->options.opengl_loadProcFunction != nullptr);
	if (!gladLoadGLLoader((GLADloadproc)gfx->options.opengl_loadProcFunction))
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
	//There's no initialization we need to do before options have been set (and before a window has been created)
	return true;
}

#if PIG_GFX_GLFW_SUPPORTED
void PigGfx_SetGlfwWindowHints_OpenGL()
{
	// PrintLine_D("Requesting OpenGL %d.%d PROFILE=%s FORWARD_COMPAT=%s DEBUG=%s",
	// 	gfx->options.opengl_RequestVersionMajor,
	// 	gfx->options.opengl_RequestVersionMinor,
	// 	GetOpenGlProfileStr(gfx->options.opengl_RequestProfile),
	// 	gfx->options.opengl_RequestForwardCompat ? "true" : "false",
	// 	gfx->options.opengl_requestDebugContext ? "true" : "false"
	// ); //TODO: Re-enable me once we have debug output in pig_graphics
	int requestProfileGlfwValue = GLFW_OPENGL_ANY_PROFILE;
	switch (gfx->options.opengl_RequestProfile)
	{
		case OpenGlProfile_Any: requestProfileGlfwValue = GLFW_OPENGL_ANY_PROFILE; break;
		case OpenGlProfile_Core: requestProfileGlfwValue = GLFW_OPENGL_CORE_PROFILE; break;
		case OpenGlProfile_Compat: requestProfileGlfwValue = GLFW_OPENGL_COMPAT_PROFILE; break;
		default: AssertMsg(false, "Unsupported value passed for opengl_RequestProfile in PigGfx_SetOptions!"); break;
	}
	glfwWindowHint(GLFW_CLIENT_API,            GLFW_OPENGL_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, gfx->options.opengl_RequestVersionMajor);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, gfx->options.opengl_RequestVersionMinor);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, gfx->options.opengl_RequestForwardCompat ? GLFW_TRUE : GLFW_FALSE); //Makes MacOSX happy?
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT,  gfx->options.opengl_requestDebugContext ? GLFW_TRUE : GLFW_FALSE);
	glfwWindowHint(GLFW_OPENGL_PROFILE,        requestProfileGlfwValue);
}
#endif

GraphicsContext_t* PigGfx_CreateContext_OpenGL()
{
	Assert(!gfx->contextCreated);
	if (!_PigGfx_GladInit()) { return nullptr; }
	
	gfx->contextCreated = true;
	
	ClearStruct(gfx->context);
	gfx->context.renderApi = RenderApi_OpenGL;
	gfx->context.openglVersionMajor = GLVersion.major;
	gfx->context.openglVersionMinor = GLVersion.minor;
	return &gfx->context;
}

// +--------------------------------------------------------------+
// |                       Render Functions                       |
// +--------------------------------------------------------------+
void PigGfx_BeginRendering_OpenGL(bool doClearColor, Color_t clearColor, bool doClearDepth, r32 clearDepth, bool doClearStencil, int clearStencilValue)
{
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //Win32_AssertNoOpenGlError(glBlendFunc);
	glEnable(GL_BLEND); //Win32_AssertNoOpenGlError(glEnable);
	glEnable(GL_DEPTH_TEST); //Win32_AssertNoOpenGlError(glEnable);
	glDepthFunc(GL_LEQUAL); //Win32_AssertNoOpenGlError(glDepthFunc);
	//NOTE: GL_APHA_TEST and glAlphaFunc have been deprecated. We should use discard; in shaders instead
	// glEnable(GL_ALPHA_TEST); //Win32_AssertNoOpenGlError(glEnable);
	// glAlphaFunc(GL_GEQUAL, 0.01f); //Win32_AssertNoOpenGlError(glAlphaFunc);
	glDisable(GL_CULL_FACE); //Win32_AssertNoOpenGlError(glAlphaFunc);
	glFrontFace(GL_CW); //Win32_AssertNoOpenGlError(glAlphaFunc);
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	if (doClearColor)
	{
		v4 colorVec = ToVec4(clearColor);
		glClearColor(colorVec.r, colorVec.g, colorVec.b, colorVec.a);
	}
	if (doClearDepth)
	{
		glClearDepth(clearDepth);
	}
	if (doClearStencil)
	{
		glClearStencil(clearStencilValue);
	}
	glClear(
		(doClearColor   ? GL_COLOR_BUFFER_BIT   : 0) |
		(doClearDepth   ? GL_DEPTH_BUFFER_BIT   : 0) |
		(doClearStencil ? GL_STENCIL_BUFFER_BIT : 0)
	);
}
