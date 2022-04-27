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

const char* simpleVertexShader =
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

const char* simpleFragmentShader =
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
	"	fragColor.g *= fPosition.x;\n"
	"	fragColor.r *= fPosition.y;\n"
	"}\n";

Shader_t CreateShader(const char* vertShaderStr, const char* fragShaderStr)
{
	Shader_t result = {};
	
	result.vertId = glCreateShader(GL_VERTEX_SHADER); AssertNoOpenGlError();
	// PrintLine_D("vertId: %d", result.vertId);
	glShaderSource(result.vertId, 1, vertShaderStr); AssertNoOpenGlError();
	glCompileShader(result.vertId); AssertNoOpenGlError();
	
	result.fragId = glCreateShader(GL_FRAGMENT_SHADER); AssertNoOpenGlError();
	// PrintLine_D("fragId: %d", result.fragId);
	glShaderSource(result.fragId, 1, fragShaderStr); AssertNoOpenGlError();
	glCompileShader(result.fragId); AssertNoOpenGlError();
	
	result.glId = glCreateProgram(); AssertNoOpenGlError();
	glAttachShader(result.glId, result.vertId); AssertNoOpenGlError();
	glAttachShader(result.glId, result.fragId); AssertNoOpenGlError();
	glLinkProgram(result.glId); AssertNoOpenGlError();
	
	result.attribLocations.position  = glGetAttribLocation(result.glId, "inPosition"); AssertNoOpenGlError();
	result.attribLocations.color     = glGetAttribLocation(result.glId, "inColor"); AssertNoOpenGlError();
	result.attribLocations.texCoord  = glGetAttribLocation(result.glId, "inTexCoord"); AssertNoOpenGlError();
	result.attribLocations.normal    = glGetAttribLocation(result.glId, "inNormal"); AssertNoOpenGlError();
	result.attribLocations.tangent   = glGetAttribLocation(result.glId, "inTangent"); AssertNoOpenGlError();
	
	result.uniforms.worldMatrix      = glGetUniformLocation(result.glId, "WorldMatrix"); AssertNoOpenGlError();
	result.uniforms.viewMatrix       = glGetUniformLocation(result.glId, "ViewMatrix"); AssertNoOpenGlError();
	result.uniforms.projectionMatrix = glGetUniformLocation(result.glId, "ProjectionMatrix"); AssertNoOpenGlError();
	result.uniforms.texture          = glGetUniformLocation(result.glId, "Texture"); AssertNoOpenGlError();
	result.uniforms.textureSize      = glGetUniformLocation(result.glId, "TextureSize"); AssertNoOpenGlError();
	result.uniforms.sourceRec        = glGetUniformLocation(result.glId, "SourceRec"); AssertNoOpenGlError();
	result.uniforms.color1           = glGetUniformLocation(result.glId, "Color1"); AssertNoOpenGlError();
	result.uniforms.color2           = glGetUniformLocation(result.glId, "Color2"); AssertNoOpenGlError();
	result.uniforms.time             = glGetUniformLocation(result.glId, "Time"); AssertNoOpenGlError();
	
	return result;
}

Texture_t CreateTexture(v2i size, const u8* pixelData, bool pixelated, bool repeating)
{
	Texture_t result = {};
	
	result.glId = glCreateTexture();
	glBindTexture(GL_TEXTURE_2D, result.glId);
	
	glTexImage2D(
		GL_TEXTURE_2D,      //bound texture type
		0,                  //image level
		GL_RGBA,            //internal format
		size.width,         //image width
		size.height,        //image height
		0,                  //border
		GL_RGBA,            //format
		GL_UNSIGNED_BYTE,   //type
		pixelData           //data
	);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (pixelated ? GL_NEAREST_MIPMAP_NEAREST : GL_LINEAR_MIPMAP_LINEAR));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (pixelated ? GL_NEAREST : GL_LINEAR));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (repeating ? GL_REPEAT : GL_CLAMP_TO_EDGE));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (repeating ? GL_REPEAT : GL_CLAMP_TO_EDGE));
	glGenerateMipmap(GL_TEXTURE_2D);
	
	result.isValid = true;
	return result;
}

bool CreateVertBuffer_(MemArena_t* memArena, VertBuffer_t* bufferOut, bool dynamic, u64 numVertices, void* verticesPntr, bool copyVertices, VertexType_t vertexType, u64 vertexSize)
{
	NotNull(memArena);
	NotNull(bufferOut);
	Assert(verticesPntr != nullptr || numVertices == 0);
	Assert(vertexType != VertexType_None);
	Assert(vertexSize > 0);
	ClearPointer(bufferOut);
	
	GLuint bufferId = glCreateBuffer(); AssertNoOpenGlError();
	glBindBuffer(GL_ARRAY_BUFFER, bufferId); AssertNoOpenGlError();
	glBufferData(GL_ARRAY_BUFFER, numVertices * vertexSize, verticesPntr, (dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW)); AssertNoOpenGlError();
	
	if (copyVertices)
	{
		bufferOut->vertsVoidPntr = AllocMem(memArena, numVertices * vertexSize);
		NotNullMsg(bufferOut->vertsVoidPntr, "Failed to allocate vertex data on local heap in CreateVertBuffer");
		MyMemCopy(bufferOut->vertsVoidPntr, verticesPntr, numVertices * vertexSize);
		bufferOut->hasVerticesCopy = true;
	}
	
	bufferOut->glId = bufferId;
	bufferOut->allocArena = memArena;
	bufferOut->isDynamic = dynamic;
	bufferOut->numVertices = numVertices;
	bufferOut->vertexType = vertexType;
	bufferOut->vertexSize = vertexSize;
	bufferOut->isValid = true;
	
	return true;
}
#define CreateVertBuffer2D(memArena, bufferOut, dynamic, numVertices, verticesPntr, copyVertices) CreateVertBuffer_((memArena), (bufferOut), (dynamic), (numVertices), (verticesPntr), (copyVertices), VertexType_Default2D, VertexType_Default2D_Size)
#define CreateVertBuffer3D(memArena, bufferOut, dynamic, numVertices, verticesPntr, copyVertices) CreateVertBuffer_((memArena), (bufferOut), (dynamic), (numVertices), (verticesPntr), (copyVertices), VertexType_Default3D, VertexType_Default3D_Size)

VertexArrayObject_t CreateVertexArrayObject(VertexType_t vertexType)
{
	VertexArrayObject_t result = {};
	
	result.glId = glCreateVertexArray(); AssertNoOpenGlError();
	result.vertexType = vertexType;
	result.boundOnce = false;
	
	return result;
}

void BindVertexArrayObject(VertexArrayObject_t* vao, const Shader_t* boundShader, const VertBuffer_t* boundBuffer)
{
	glBindVertexArray(vao->glId);
	if (!vao->boundOnce || true) //TODO: Remove me when we figure out why the assertion in the else statement is firing on laptop
	{
		for (u64 attIndex = 0; attIndex < MAX_NUM_VERTEX_ATTRIBUTES; attIndex++)
		{
			if (boundShader->attribLocations.values[attIndex] >= 0)
			{
				glEnableVertexAttribArray(boundShader->attribLocations.values[attIndex]); AssertNoOpenGlError();
			}
		}
		vao->boundOnce = true;
	}
	
	u8* attribOffset = nullptr;
	if (IsFlagSet(boundBuffer->vertexType, VertexType_PositionBit))
	{
		if (boundShader->attribLocations.position >= 0)
		{
			glVertexAttribPointer(boundShader->attribLocations.position, 3, GL_FLOAT, GL_FALSE, (GLsizei)boundBuffer->vertexSize, attribOffset); AssertNoOpenGlError();
		}
		attribOffset += sizeof(v3);
	}
	if (IsFlagSet(boundBuffer->vertexType, VertexType_ColorBit))
	{
		if (boundShader->attribLocations.color >= 0)
		{
			glVertexAttribPointer(boundShader->attribLocations.color, 4, GL_FLOAT, GL_FALSE, (GLsizei)boundBuffer->vertexSize, attribOffset); AssertNoOpenGlError();
		}
		attribOffset += sizeof(v4);
	}
	if (IsFlagSet(boundBuffer->vertexType, VertexType_TexCoordBit))
	{
		if (boundShader->attribLocations.texCoord >= 0)
		{
			glVertexAttribPointer(boundShader->attribLocations.texCoord, 2, GL_FLOAT, GL_FALSE, (GLsizei)boundBuffer->vertexSize, attribOffset); AssertNoOpenGlError();
		}
		attribOffset += sizeof(v2);
	}
	if (IsFlagSet(boundBuffer->vertexType, VertexType_NormalBit))
	{
		if (boundShader->attribLocations.normal >= 0)
		{
			glVertexAttribPointer(boundShader->attribLocations.normal, 3, GL_FLOAT, GL_TRUE, (GLsizei)boundBuffer->vertexSize, attribOffset); AssertNoOpenGlError();
		}
		attribOffset += sizeof(v3);
	}
	if (IsFlagSet(boundBuffer->vertexType, VertexType_TangentBit))
	{
		if (boundShader->attribLocations.tangent >= 0)
		{
			glVertexAttribPointer(boundShader->attribLocations.tangent, 3, GL_FLOAT, GL_TRUE, (GLsizei)boundBuffer->vertexSize, attribOffset); AssertNoOpenGlError();
		}
		attribOffset += sizeof(v3);
	}
}
