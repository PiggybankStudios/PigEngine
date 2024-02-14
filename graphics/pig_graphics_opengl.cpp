/*
File:   pig_graphics_opengl.cpp
Author: Taylor Robbins
Date:   02\06\2024
Description: 
	** None 
*/

#include "glad/glad.c"

const char* GetOpenGlProfileStr(OpenGlProfile_t enumValue)
{
	switch (enumValue)
	{
		case OpenGlProfile_None:   return "None";
		case OpenGlProfile_Any:    return "Any";
		case OpenGlProfile_Core:   return "Core";
		case OpenGlProfile_Compat: return "Compat";
		default: return "Unknown";
	}
}

static bool _PigGfx_GladInit()
{
	// WriteLine_R("Initializing GLAD..."); //TODO: Re-Enable me!
	
	Assert(gfx->optionsSet);
	Assert(gfx->options.gl.loadProcFunction != nullptr);
	if (!gladLoadGLLoader((GLADloadproc)gfx->options.gl.loadProcFunction))
	{
		PigGfx_InitFailure("Could not initialize GLAD. gladLoadGLLoader failed");
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
	// 	gfx->options.gl.requestVersionMajor,
	// 	gfx->options.gl.requestVersionMinor,
	// 	GetOpenGlProfileStr(gfx->options.gl.requestProfile),
	// 	gfx->options.gl.requestForwardCompat ? "true" : "false",
	// 	gfx->options.gl.requestDebugContext ? "true" : "false"
	// ); //TODO: Re-enable me once we have debug output in pig_graphics
	int requestProfileGlfwValue = GLFW_OPENGL_ANY_PROFILE;
	switch (gfx->options.gl.requestProfile)
	{
		case OpenGlProfile_Any: requestProfileGlfwValue = GLFW_OPENGL_ANY_PROFILE; break;
		case OpenGlProfile_Core: requestProfileGlfwValue = GLFW_OPENGL_CORE_PROFILE; break;
		case OpenGlProfile_Compat: requestProfileGlfwValue = GLFW_OPENGL_COMPAT_PROFILE; break;
		default: AssertMsg(false, "Unsupported value passed for opengl_RequestProfile in PigGfx_SetOptions!"); break;
	}
	glfwWindowHint(GLFW_CLIENT_API,            GLFW_OPENGL_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, gfx->options.gl.requestVersionMajor);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, gfx->options.gl.requestVersionMinor);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, gfx->options.gl.requestForwardCompat ? GLFW_TRUE : GLFW_FALSE); //Makes MacOSX happy?
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT,  gfx->options.gl.requestDebugContext ? GLFW_TRUE : GLFW_FALSE);
	glfwWindowHint(GLFW_OPENGL_PROFILE,        requestProfileGlfwValue);
}
#endif

void PigGfx_DestroyContext_OpenGL(GraphicsContext_t* context)
{
	//TODO: Implement me!
	ClearPointer(context);
}

GraphicsContext_t* PigGfx_CreateContext_OpenGL(MemArena_t* memArena)
{
	Assert(!gfx->contextCreated);
	if (!_PigGfx_GladInit()) { return nullptr; }
	
	gfx->contextCreated = true;
	
	ClearStruct(gfx->context);
	gfx->context.renderApi = RenderApi_OpenGL;
	gfx->context.gl.versionMajor = GLVersion.major;
	gfx->context.gl.versionMinor = GLVersion.minor;
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
