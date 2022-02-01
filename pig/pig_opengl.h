/*
File:   pig_opengl.h
Author: Taylor Robbins
Date:   09\26\2021
Description:
	** Contains a bunch of helper functions for dealing with various OpenGL stuff
*/

#ifndef _PIG_OPENGL_H
#define _PIG_OPENGL_H

#if OPENGL_SUPPORTED

const char* GetOpenGlErrorStr(GLenum glError, bool printUnkown = false)
{
	switch (glError)
	{
		case GL_NO_ERROR:                      return "NO_ERROR";
		case GL_INVALID_ENUM:                  return "INVALID_ENUM";
		case GL_INVALID_VALUE:                 return "INVALID_VALUE";
		case GL_INVALID_OPERATION:             return "INVALID_OPERATION";
		case GL_STACK_UNDERFLOW:               return "STACK_UNDERFLOW";
		case GL_STACK_OVERFLOW:                return "STACK_OVERFLOW";
		case GL_OUT_OF_MEMORY:                 return "OUT_OF_MEMORY";
		case GL_INVALID_FRAMEBUFFER_OPERATION: return "INVALID_FRAMEBUFFER_OPERATION";
		case GL_CONTEXT_LOST:                  return "GL_CONTEXT_LOST";
		case GL_TABLE_TOO_LARGE:               return "GL_TABLE_TOO_LARGE";
		default: const char* result = (printUnkown ? TempPrint("[0x%08X]", glError) : "Unknown"); NotNull(result); return result;
	}
}

const char* CheckOpenGlError(bool printUnkown = false)
{
	GLenum glError = glGetError();
	if (glError != GL_NO_ERROR) { return GetOpenGlErrorStr(glError, printUnkown); }
	else { return nullptr; }
}
#define AssertNoOpenGlError() do                                               \
{                                                                              \
	GLenum glError = glGetError();                                             \
	if (glError != GL_NO_ERROR)                                                \
	{                                                                          \
		PrintLine_E("Got OpenGL Error: %s", GetOpenGlErrorStr(glError, true)); \
	}                                                                          \
	Assert(glError == GL_NO_ERROR);                                            \
} while(0)

#if DEBUG_BUILD
#define DebugAssertNoOpenGlError() AssertNoOpenGlError()
#else
#define DebugAssertNoOpenGlError() //nothing
#endif

#endif //OPENGL_SUPPORTED

#endif //  _PIG_OPENGL_H
