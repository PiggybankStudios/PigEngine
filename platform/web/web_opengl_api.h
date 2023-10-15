/*
File:   web_opengl_api.h
Author: Taylor Robbins
Date:   10\15\2023
*/

#ifndef _WEB_OPENGL_API_H
#define _WEB_OPENGL_API_H

WASM_IMPORTED_FUNC GLenum jsGlGetError();

WASM_IMPORTED_FUNC void jsGlClearColor(float r, float g, float b, float a);
WASM_IMPORTED_FUNC void jsGlClear(GLbitfield mask);
WASM_IMPORTED_FUNC void jsGlViewport(GLint x, GLint y, GLsizei width, GLsizei height);
WASM_IMPORTED_FUNC void jsGlEnable(GLenum cap);
WASM_IMPORTED_FUNC void jsGlDisable(GLenum cap);
WASM_IMPORTED_FUNC void jsGlBlendFunc(GLenum sfactor, GLenum dfactor);
WASM_IMPORTED_FUNC void jsGlBlendFuncSeparate(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
WASM_IMPORTED_FUNC void jsGlDepthFunc(GLenum func);
WASM_IMPORTED_FUNC void jsGlFrontFace(GLenum mode);
// WASM_IMPORTED_FUNC void jsGlAlphaFunc(GLenum func, GLfloat ref);
WASM_IMPORTED_FUNC void jsGlLineWidth(GLfloat width);

WASM_IMPORTED_FUNC GLuint jsGlCreateShader(GLenum type);
WASM_IMPORTED_FUNC GLuint jsGlCreateProgram();
WASM_IMPORTED_FUNC GLuint jsGlShaderSource(GLuint shader, GLsizei count, const char* string);
WASM_IMPORTED_FUNC GLuint jsGlCompileShader(GLuint shader);
WASM_IMPORTED_FUNC GLuint jsGlAttachShader(GLuint program, GLuint shader);
WASM_IMPORTED_FUNC GLuint jsGlLinkProgram(GLuint program);
WASM_IMPORTED_FUNC GLint jsGlGetAttribLocation(GLuint program, const GLchar* name);
WASM_IMPORTED_FUNC GLint jsGlGetUniformLocation(GLuint program, const GLchar* name);
WASM_IMPORTED_FUNC void jsGlUniform1i(GLint location, GLint v0);
WASM_IMPORTED_FUNC void jsGlUniform1f(GLint location, GLfloat v0);
WASM_IMPORTED_FUNC void jsGlUniform2f(GLint location, GLfloat v0, GLfloat v1);
WASM_IMPORTED_FUNC void jsGlUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
WASM_IMPORTED_FUNC void jsGlUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
WASM_IMPORTED_FUNC void jsGlUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
WASM_IMPORTED_FUNC void jsGlUseProgram(GLuint program);

WASM_IMPORTED_FUNC GLint jsGlCreateTexture();
WASM_IMPORTED_FUNC void jsGlBindTexture(GLenum target, GLuint texture);
WASM_IMPORTED_FUNC void jsGlTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels);
WASM_IMPORTED_FUNC void jsGlTexParameteri(GLenum target, GLenum pname, GLint param);
WASM_IMPORTED_FUNC void jsGlGenerateMipmap(GLenum target);
WASM_IMPORTED_FUNC void jsGlActiveTexture(GLenum texture);

// WASM_IMPORTED_FUNC void jsGlDeleteBuffers(GLsizei count, const GLuint* buffers);
WASM_IMPORTED_FUNC GLuint jsGlCreateBuffer();
WASM_IMPORTED_FUNC void jsGlBindBuffer(GLenum target, GLuint buffer);
WASM_IMPORTED_FUNC void jsGlBufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage);

WASM_IMPORTED_FUNC GLuint jsGlCreateVertexArray();
WASM_IMPORTED_FUNC void jsGlBindVertexArray(GLuint array);
WASM_IMPORTED_FUNC void jsGlEnableVertexAttribArray(GLuint index);
WASM_IMPORTED_FUNC void jsGlVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* offset);

WASM_IMPORTED_FUNC void jsGlDrawArrays(GLenum mode, GLint first, GLsizei count);

#endif //  _WEB_OPENGL_API_H

// +--------------------------------------------------------------+
// |                   Autocomplete Dictionary                    |
// +--------------------------------------------------------------+
/*
@Functions
GLenum jsGlGetError()
void jsGlClearColor(float r, float g, float b, float a)
void jsGlClear(GLbitfield mask)
void jsGlViewport(GLint x, GLint y, GLsizei width, GLsizei height)
void jsGlEnable(GLenum cap)
void jsGlDisable(GLenum cap)
void jsGlBlendFunc(GLenum sfactor, GLenum dfactor)
void jsGlBlendFuncSeparate(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha)
void jsGlDepthFunc(GLenum func)
void jsGlFrontFace(GLenum mode)
void jsGlLineWidth(GLfloat width)
GLuint jsGlCreateShader(GLenum type)
GLuint jsGlCreateProgram()
GLuint jsGlShaderSource(GLuint shader, GLsizei count, const char* string)
GLuint jsGlCompileShader(GLuint shader)
GLuint jsGlAttachShader(GLuint program, GLuint shader)
GLuint jsGlLinkProgram(GLuint program)
GLint jsGlGetAttribLocation(GLuint program, const GLchar* name)
GLint jsGlGetUniformLocation(GLuint program, const GLchar* name)
void jsGlUniform1i(GLint location, GLint v0)
void jsGlUniform1f(GLint location, GLfloat v0)
void jsGlUniform2f(GLint location, GLfloat v0, GLfloat v1)
void jsGlUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
void jsGlUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
void jsGlUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
void jsGlUseProgram(GLuint program)
GLint jsGlCreateTexture()
void jsGlBindTexture(GLenum target, GLuint texture)
void jsGlTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels)
void jsGlTexParameteri(GLenum target, GLenum pname, GLint param)
void jsGlGenerateMipmap(GLenum target)
void jsGlActiveTexture(GLenum texture)
GLuint jsGlCreateBuffer()
void jsGlBindBuffer(GLenum target, GLuint buffer)
void jsGlBufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage)
GLuint jsGlCreateVertexArray()
void jsGlBindVertexArray(GLuint array)
void jsGlEnableVertexAttribArray(GLuint index)
void jsGlVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* offset)
void jsGlDrawArrays(GLenum mode, GLint first, GLsizei count)
*/
