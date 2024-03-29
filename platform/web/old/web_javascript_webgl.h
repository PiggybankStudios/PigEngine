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
typedef float GLfloat;
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
#define GL_ZERO 0
#define GL_ONE 1
#define GL_NONE 0

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

#define GL_TEXTURE_1D 0x0DE0
#define GL_TEXTURE_2D 0x0DE1

#define GL_RED 0x1903
#define GL_GREEN 0x1904
#define GL_BLUE 0x1905
#define GL_ALPHA 0x1906
#define GL_RGB 0x1907
#define GL_RGBA 0x1908
#define GL_BGR 0x80E0
#define GL_BGRA 0x80E1

#define GL_NEAREST 0x2600
#define GL_LINEAR 0x2601
#define GL_NEAREST_MIPMAP_NEAREST 0x2700
#define GL_LINEAR_MIPMAP_NEAREST 0x2701
#define GL_NEAREST_MIPMAP_LINEAR 0x2702
#define GL_LINEAR_MIPMAP_LINEAR 0x2703
#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_TEXTURE_MIN_FILTER 0x2801
#define GL_TEXTURE_WRAP_S 0x2802
#define GL_TEXTURE_WRAP_T 0x2803
#define GL_REPEAT 0x2901
#define GL_CLAMP_TO_EDGE 0x812F

#define GL_TEXTURE0 0x84C0
#define GL_TEXTURE1 0x84C1
#define GL_TEXTURE2 0x84C2
#define GL_TEXTURE3 0x84C3
#define GL_TEXTURE4 0x84C4
#define GL_TEXTURE5 0x84C5
#define GL_TEXTURE6 0x84C6
#define GL_TEXTURE7 0x84C7
#define GL_TEXTURE8 0x84C8
#define GL_TEXTURE9 0x84C9
#define GL_TEXTURE10 0x84CA
#define GL_TEXTURE11 0x84CB
#define GL_TEXTURE12 0x84CC
#define GL_TEXTURE13 0x84CD
#define GL_TEXTURE14 0x84CE
#define GL_TEXTURE15 0x84CF
#define GL_TEXTURE16 0x84D0
#define GL_TEXTURE17 0x84D1
#define GL_TEXTURE18 0x84D2
#define GL_TEXTURE19 0x84D3
#define GL_TEXTURE20 0x84D4
#define GL_TEXTURE21 0x84D5
#define GL_TEXTURE22 0x84D6
#define GL_TEXTURE23 0x84D7
#define GL_TEXTURE24 0x84D8
#define GL_TEXTURE25 0x84D9
#define GL_TEXTURE26 0x84DA
#define GL_TEXTURE27 0x84DB
#define GL_TEXTURE28 0x84DC
#define GL_TEXTURE29 0x84DD
#define GL_TEXTURE30 0x84DE
#define GL_TEXTURE31 0x84DF

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

#define GL_NO_ERROR                      0
#define GL_INVALID_ENUM                  0x0500
#define GL_INVALID_VALUE                 0x0501
#define GL_INVALID_OPERATION             0x0502
#define GL_STACK_OVERFLOW                0x0503
#define GL_STACK_UNDERFLOW               0x0504
#define GL_OUT_OF_MEMORY                 0x0505
#define GL_INVALID_FRAMEBUFFER_OPERATION 0x0506
#define GL_CONTEXT_LOST                  0x0507
#define GL_TABLE_TOO_LARGE               0x8031

#define GL_CW 0x0900
#define GL_CCW 0x0901
#define GL_SRC_COLOR 0x0300
#define GL_ONE_MINUS_SRC_COLOR 0x0301
#define GL_SRC_ALPHA 0x0302
#define GL_ONE_MINUS_SRC_ALPHA 0x0303
#define GL_DST_ALPHA 0x0304
#define GL_ONE_MINUS_DST_ALPHA 0x0305
#define GL_DST_COLOR 0x0306
#define GL_ONE_MINUS_DST_COLOR 0x0307
#define GL_SRC_ALPHA_SATURATE 0x0308

#define GL_NEVER 0x0200
#define GL_LESS 0x0201
#define GL_EQUAL 0x0202
#define GL_LEQUAL 0x0203
#define GL_GREATER 0x0204
#define GL_NOTEQUAL 0x0205
#define GL_GEQUAL 0x0206
#define GL_ALWAYS 0x0207

#define GL_CULL_FACE 0x0B44
#define GL_BLEND 0x0BE2
#define GL_DEPTH_TEST 0x0B71
// #define GL_ALPHA_TEST 0x0BC0

extern GLenum glGetError();

extern void glClearColor(float r, float g, float b, float a);
extern void glClear(GLbitfield mask);
extern void glViewport(GLint x, GLint y, GLsizei width, GLsizei height);
extern void glEnable(GLenum cap);
extern void glDisable(GLenum cap);
extern void glBlendFunc(GLenum sfactor, GLenum dfactor);
extern void glBlendFuncSeparate(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
extern void glDepthFunc(GLenum func);
extern void glFrontFace(GLenum mode);
// extern void glAlphaFunc(GLenum func, GLfloat ref);
extern void glLineWidth(GLfloat width);

extern GLuint glCreateShader(GLenum type);
extern GLuint glCreateProgram();
extern GLuint glShaderSource(GLuint shader, GLsizei count, const char* string);
extern GLuint glCompileShader(GLuint shader);
extern GLuint glAttachShader(GLuint program, GLuint shader);
extern GLuint glLinkProgram(GLuint program);
extern GLint glGetAttribLocation(GLuint program, const GLchar* name);
extern GLint glGetUniformLocation(GLuint program, const GLchar* name);
extern void glUniform1i(GLint location, GLint v0);
extern void glUniform1f(GLint location, GLfloat v0);
extern void glUniform2f(GLint location, GLfloat v0, GLfloat v1);
extern void glUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
extern void glUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
extern void glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
extern void glUseProgram(GLuint program);

extern GLint glCreateTexture();
extern void glBindTexture(GLenum target, GLuint texture);
extern void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels);
extern void glTexParameteri(GLenum target, GLenum pname, GLint param);
extern void glGenerateMipmap(GLenum target);
extern void glActiveTexture(GLenum texture);

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
