/*
File:   web_helpers.cpp
Author: Taylor Robbins
Date:   04\06\2022
Description: 
	** Holds a bunch of helper functions (this file may get deleted as things get organized into proper files)
*/

r32 OscillateBy(u64 timeSource, r32 min, r32 max, u64 periodMs, u64 offset = 0)
{
	r32 lerpValue = (SinR32((((timeSource + offset) % periodMs) / (r32)periodMs) * 2*Pi32) + 1.0f) / 2.0f;
	return min + (max - min) * lerpValue;
}

const char* simpleVertexShader = "#version 300 es\nin vec3 inPosition;\nin vec4 inColor;\nout vec3 fPosition;\nout vec4 fColor;\nvoid main()\n{\n\tfPosition = inPosition;\n\tfColor = inColor;\n\tgl_Position = vec4(inPosition, 1.0);\n}\n";
const char* simpleFragmentShader = "#version 300 es\nprecision highp float;\nin vec3 fPosition;\nin vec4 fColor;\nout vec4 fragColor;\nvoid main()\n{\n\tfragColor = fColor;\n}";

struct Shader_t
{
	GLuint id;
	GLuint vertId;
	GLuint fragId;
};

Shader_t CreateShader(const char* vertShaderStr, const char* fragShaderStr)
{
	Shader_t result = {};
	
	result.vertId = glCreateShader(GL_VERTEX_SHADER);
	PrintLine_D("vertId: %d", result.vertId);
	glShaderSource(result.vertId, 1, vertShaderStr);
	glCompileShader(result.vertId);
	
	result.fragId = glCreateShader(GL_FRAGMENT_SHADER);
	PrintLine_D("fragId: %d", result.fragId);
	glShaderSource(result.fragId, 1, fragShaderStr);
	glCompileShader(result.fragId);
	
	result.id = glCreateProgram();
	glAttachShader(result.id, result.vertId);
	glAttachShader(result.id, result.fragId);
	glLinkProgram(result.id);
	
	return result;
}
