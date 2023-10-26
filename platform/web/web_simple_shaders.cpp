/*
File:   web_simple_shaders.cpp
Author: Taylor Robbins
Date:   10\17\2023
Description: 
	** Holds some GLSL code in some constant strings that help us create a simple shader for 2D rendering from the platform layer 
*/

const char* WebSimpleVertexShader =
	"#version 300 es\n"
	"uniform mat4 WorldMatrix;\n"
	"uniform mat4 ViewMatrix;\n"
	"uniform mat4 ProjectionMatrix;\n"
	"in vec3 inPosition;\n"
	"in vec4 inColor;\n"
	"in vec2 inTexCoord;\n"
	"out vec3 fPosition;\n"
	"out vec4 fColor;\n"
	"out vec2 fSampleCoord;\n"
	"void main()\n"
	"{\n"
	"	fPosition = inPosition;\n"
	"	fColor = inColor;\n"
	"	fSampleCoord = inTexCoord;\n"
	"	mat4 transformMatrix = ProjectionMatrix * (ViewMatrix * WorldMatrix);\n"
	"	gl_Position = transformMatrix * vec4(inPosition, 1.0);\n"
	"}\n";

const char* WebSimpleFragmentShader =
	"#version 300 es\n"
	"precision highp float;\n"
	"uniform vec4 Color1;\n"
	"uniform sampler2D Texture;\n"
	"uniform vec2 TextureSize;\n"
	"in vec3 fPosition;\n"
	"in vec4 fColor;\n"
	"in vec2 fSampleCoord;\n"
	"out vec4 fragColor;\n"
	"void main()\n"
	"{\n"
	"	vec4 sampleColor = texture(Texture, fSampleCoord);\n"
	"	fragColor = Color1 * fColor * sampleColor;\n"
	// "	fragColor.g *= fPosition.x;\n"
	// "	fragColor.r *= fPosition.y;\n"
	"}\n";
