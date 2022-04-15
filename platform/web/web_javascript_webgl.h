/*
File:   web_javascript_webgl.h
Author: Taylor Robbins
Date:   04\05\2022
*/

#ifndef _WEB_JAVASCRIPT_WEBGL_H
#define _WEB_JAVASCRIPT_WEBGL_H

typedef unsigned int GLenum;
typedef unsigned char GLboolean;
typedef unsigned int GLbitfield;
typedef void GLvoid;
// typedef khronos_int8_t GLbyte;
// typedef khronos_uint8_t GLubyte;
// typedef khronos_int16_t GLshort;
// typedef khronos_uint16_t GLushort;
typedef int GLint;
typedef unsigned int GLuint;
// typedef khronos_int32_t GLclampx;
typedef int GLsizei;
// typedef khronos_float_t GLfloat;
// typedef khronos_float_t GLclampf;
typedef double GLdouble;
typedef double GLclampd;
typedef void *GLeglClientBufferEXT;
typedef void *GLeglImageOES;
typedef char GLchar;
typedef char GLcharARB;
// #ifdef __APPLE__
// typedef void *GLhandleARB;
// #else
// typedef unsigned int GLhandleARB;
// #endif
// typedef khronos_uint16_t GLhalf;
// typedef khronos_uint16_t GLhalfARB;
// typedef khronos_int32_t GLfixed;
// typedef khronos_intptr_t GLintptr;
// typedef khronos_intptr_t GLintptrARB;
// typedef khronos_ssize_t GLsizeiptr;
// typedef khronos_ssize_t GLsizeiptrARB;
// typedef khronos_int64_t GLint64;
// typedef khronos_int64_t GLint64EXT;
// typedef khronos_uint64_t GLuint64;
// typedef khronos_uint64_t GLuint64EXT;

#define GL_POINTS 0x0000
#define GL_LINES 0x0001
#define GL_LINE_LOOP 0x0002
#define GL_LINE_STRIP 0x0003
#define GL_TRIANGLES 0x0004
#define GL_TRIANGLE_STRIP 0x0005
#define GL_TRIANGLE_FAN 0x0006
#define GL_QUADS 0x0007

#define GL_DEPTH_BUFFER_BIT 0x00000100
#define GL_STENCIL_BUFFER_BIT 0x00000400
#define GL_COLOR_BUFFER_BIT 0x00004000

#define GL_FRAGMENT_SHADER 0x8B30
#define GL_VERTEX_SHADER 0x8B31

extern void glClearColor(float r, float g, float b, float a);
extern void glClear(GLbitfield mask);
extern void glViewport(GLint x, GLint y, GLsizei width, GLsizei height);

extern GLuint glCreateShader(GLenum type);
extern GLuint glCreateProgram();
extern GLuint glShaderSource(GLuint shader, GLsizei count, const char* string);
extern GLuint glCompileShader(GLuint shader);
extern GLuint glAttachShader(GLuint program, GLuint shader);
extern GLuint glLinkProgram(GLuint program);

#endif //  _WEB_JAVASCRIPT_WEBGL_H
