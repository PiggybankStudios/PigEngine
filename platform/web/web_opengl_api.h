/*
File:   web_opengl_api.h
Author: Taylor Robbins
Date:   10\15\2023
*/

#ifndef _WEB_OPENGL_API_H
#define _WEB_OPENGL_API_H

WASM_IMPORTED_FUNC GLenum glGetError();

WASM_IMPORTED_FUNC void glClearColor(float r, float g, float b, float a);
WASM_IMPORTED_FUNC void glClear(GLbitfield mask);
WASM_IMPORTED_FUNC void glViewport(GLint x, GLint y, GLsizei width, GLsizei height);
WASM_IMPORTED_FUNC void glEnable(GLenum cap);
WASM_IMPORTED_FUNC void glDisable(GLenum cap);
WASM_IMPORTED_FUNC void glBlendFunc(GLenum sfactor, GLenum dfactor);
WASM_IMPORTED_FUNC void glBlendFuncSeparate(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
WASM_IMPORTED_FUNC void glDepthFunc(GLenum func);
WASM_IMPORTED_FUNC void glFrontFace(GLenum mode);
// WASM_IMPORTED_FUNC void glAlphaFunc(GLenum func, GLfloat ref);
WASM_IMPORTED_FUNC void glLineWidth(GLfloat width);

WASM_IMPORTED_FUNC GLuint glCreateShader(GLenum type);
WASM_IMPORTED_FUNC GLuint glCreateProgram();
WASM_IMPORTED_FUNC GLuint glShaderSource(GLuint shader, GLsizei count, const char* string);
WASM_IMPORTED_FUNC GLuint glCompileShader(GLuint shader);
WASM_IMPORTED_FUNC GLuint glAttachShader(GLuint program, GLuint shader);
WASM_IMPORTED_FUNC GLuint glLinkProgram(GLuint program);
WASM_IMPORTED_FUNC GLint glGetAttribLocation(GLuint program, const GLchar* name);
WASM_IMPORTED_FUNC GLint glGetUniformLocation(GLuint program, const GLchar* name);
WASM_IMPORTED_FUNC void glUniform1i(GLint location, GLint v0);
WASM_IMPORTED_FUNC void glUniform1f(GLint location, GLfloat v0);
WASM_IMPORTED_FUNC void glUniform2f(GLint location, GLfloat v0, GLfloat v1);
WASM_IMPORTED_FUNC void glUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
WASM_IMPORTED_FUNC void glUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
WASM_IMPORTED_FUNC void glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
WASM_IMPORTED_FUNC void glUseProgram(GLuint program);

WASM_IMPORTED_FUNC GLint glCreateTexture();
WASM_IMPORTED_FUNC void glBindTexture(GLenum target, GLuint texture);
WASM_IMPORTED_FUNC void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels);
WASM_IMPORTED_FUNC void glTexParameteri(GLenum target, GLenum pname, GLint param);
WASM_IMPORTED_FUNC void glGenerateMipmap(GLenum target);
WASM_IMPORTED_FUNC void glActiveTexture(GLenum texture);

// WASM_IMPORTED_FUNC void glDeleteBuffers(GLsizei count, const GLuint* buffers);
WASM_IMPORTED_FUNC GLuint glCreateBuffer();
WASM_IMPORTED_FUNC void glBindBuffer(GLenum target, GLuint buffer);
WASM_IMPORTED_FUNC void glBufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage);

WASM_IMPORTED_FUNC GLuint glCreateVertexArray();
WASM_IMPORTED_FUNC void glBindVertexArray(GLuint array);
WASM_IMPORTED_FUNC void glEnableVertexAttribArray(GLuint index);
WASM_IMPORTED_FUNC void glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* offset);

WASM_IMPORTED_FUNC void glDrawArrays(GLenum mode, GLint first, GLsizei count);

#endif //  _WEB_OPENGL_API_H
