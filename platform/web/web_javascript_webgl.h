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
typedef size_t GLsizeiptr;
// typedef khronos_ssize_t GLsizeiptrARB;
// typedef khronos_int64_t GLint64;
// typedef khronos_int64_t GLint64EXT;
// typedef khronos_uint64_t GLuint64;
// typedef khronos_uint64_t GLuint64EXT;

#define GL_FALSE 0
#define GL_TRUE 1

#define GL_BYTE 0x1400
#define GL_UNSIGNED_BYTE 0x1401
#define GL_SHORT 0x1402
#define GL_UNSIGNED_SHORT 0x1403
#define GL_INT 0x1404
#define GL_UNSIGNED_INT 0x1405
#define GL_FLOAT 0x1406

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

#define GL_STREAM_DRAW 0x88E0
#define GL_STREAM_READ 0x88E1
#define GL_STREAM_COPY 0x88E2
#define GL_STATIC_DRAW 0x88E4
#define GL_STATIC_READ 0x88E5
#define GL_STATIC_COPY 0x88E6
#define GL_DYNAMIC_DRAW 0x88E8
#define GL_DYNAMIC_READ 0x88E9
#define GL_DYNAMIC_COPY 0x88EA

#define GL_ARRAY_BUFFER 0x8892

extern void glClearColor(float r, float g, float b, float a);
extern void glClear(GLbitfield mask);
extern void glViewport(GLint x, GLint y, GLsizei width, GLsizei height);

extern GLuint glCreateShader(GLenum type);
extern GLuint glCreateProgram();
extern GLuint glShaderSource(GLuint shader, GLsizei count, const char* string);
extern GLuint glCompileShader(GLuint shader);
extern GLuint glAttachShader(GLuint program, GLuint shader);
extern GLuint glLinkProgram(GLuint program);
extern GLint glGetAttribLocation(GLuint program, const GLchar* name);
extern void glUseProgram(GLuint program);

// extern void glDeleteBuffers(GLsizei count, const GLuint* buffers);
extern GLuint glCreateBuffer();
extern void glBindBuffer(GLenum target, GLuint buffer);
extern void glBufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage);

extern GLuint glCreateVertexArray();
extern void glBindVertexArray(GLuint array);
extern void glEnableVertexAttribArray(GLuint index);
extern void glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* offset);

extern void glDrawArrays(GLenum mode, GLint first, GLsizei count);

#endif //  _WEB_JAVASCRIPT_WEBGL_H
