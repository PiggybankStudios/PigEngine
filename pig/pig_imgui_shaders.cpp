/*
File:   pig_imgui_shaders.cpp
Author: Taylor Robbins
Date:   03\01\2024
*/

const char* ImguiShaderCode_Vert_GLSL_120 =
	"uniform mat4 ProjectionMatrix;\n"
	"attribute vec2 inPosition;\n"
	"attribute vec4 inColor1;\n"
	"attribute vec2 inTexCoord1;\n"
	"varying vec2 Frag_UV;\n"
	"varying vec4 Frag_Color;\n"
	"void main()\n"
	"{\n"
	"    Frag_UV = inTexCoord1;\n"
	"    Frag_Color = inColor1;\n"
	"    gl_Position = ProjectionMatrix * vec4(inPosition.xy,0,1);\n"
	"}\n";

const char* ImguiShaderCode_Vert_GLSL_130 =
	"uniform mat4 ProjectionMatrix;\n"
	"in vec2 inPosition;\n"
	"in vec4 inColor1;\n"
	"in vec2 inTexCoord1;\n"
	"out vec2 Frag_UV;\n"
	"out vec4 Frag_Color;\n"
	"void main()\n"
	"{\n"
	"    Frag_UV = inTexCoord1;\n"
	"    Frag_Color = inColor1;\n"
	"    gl_Position = ProjectionMatrix * vec4(inPosition.xy,0,1);\n"
	"}\n";

const char* ImguiShaderCode_Vert_GLSL_300_es =
	"precision highp float;\n"
	"layout (location = 0) in vec2 inPosition;\n"
	"layout (location = 1) in vec4 inColor1;\n"
	"layout (location = 2) in vec2 inTexCoord1;\n"
	"uniform mat4 ProjectionMatrix;\n"
	"out vec2 Frag_UV;\n"
	"out vec4 Frag_Color;\n"
	"void main()\n"
	"{\n"
	"    Frag_UV = inTexCoord1;\n"
	"    Frag_Color = inColor1;\n"
	"    gl_Position = ProjectionMatrix * vec4(inPosition.xy,0,1);\n"
	"}\n";

const char* ImguiShaderCode_Vert_GLSL_410_core =
	"layout (location = 0) in vec2 inPosition;\n"
	"layout (location = 1) in vec4 inColor1;\n"
	"layout (location = 2) in vec2 inTexCoord1;\n"
	"uniform mat4 ProjectionMatrix;\n"
	"out vec2 Frag_UV;\n"
	"out vec4 Frag_Color;\n"
	"void main()\n"
	"{\n"
	"    Frag_UV = inTexCoord1;\n"
	"    Frag_Color = inColor1;\n"
	"    gl_Position = ProjectionMatrix * vec4(inPosition.xy,0,1);\n"
	"}\n";

const char* ImguiShaderCode_Frag_GLSL_120 =
	"#ifdef GL_ES\n"
	"    precision mediump float;\n"
	"#endif\n"
	"uniform sampler2D Texture1;\n"
	"varying vec2 Frag_UV;\n"
	"varying vec4 Frag_Color;\n"
	"void main()\n"
	"{\n"
	"    gl_FragColor = Frag_Color * texture2D(Texture1, Frag_UV.st);\n"
	"}\n";

const char* ImguiShaderCode_Frag_GLSL_130 =
	"uniform sampler2D Texture1;\n"
	"in vec2 Frag_UV;\n"
	"in vec4 Frag_Color;\n"
	"out vec4 Out_Color;\n"
	"void main()\n"
	"{\n"
	"    Out_Color = Frag_Color * texture(Texture1, Frag_UV.st);\n"
	"}\n";

const char* ImguiShaderCode_Frag_GLSL_300_es =
	"precision mediump float;\n"
	"uniform sampler2D Texture1;\n"
	"in vec2 Frag_UV;\n"
	"in vec4 Frag_Color;\n"
	"layout (location = 0) out vec4 Out_Color;\n"
	"void main()\n"
	"{\n"
	"    Out_Color = Frag_Color * texture(Texture1, Frag_UV.st);\n"
	"}\n";

const char* ImguiShaderCode_Frag_GLSL_410_core =
	"in vec2 Frag_UV;\n"
	"in vec4 Frag_Color;\n"
	"uniform sampler2D Texture1;\n"
	"layout (location = 0) out vec4 Out_Color;\n"
	"void main()\n"
	"{\n"
	"    Out_Color = Frag_Color * texture(Texture1, Frag_UV.st);\n"
	"}\n";
