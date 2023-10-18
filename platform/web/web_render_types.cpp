/*
File:   web_render_types.cpp
Author: Taylor Robbins
Date:   10\16\2023
Description: 
	** Holds functions that help the platform layer of Pig Engine create Web_Shader_t,
	** Web_VertBuffer_t, Web_VertexArrayObject_t, Web_Texture_t, etc.
*/

const char* Web_GetOpenGlErrorStr(GLenum glError, bool printUnkown = false)
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
		default: return "Unknown"; // TODO: Uncomment me! //const char* result = (printUnkown ? TempPrint("[0x%08X]", glError) : "Unknown"); NotNull(result); return result;
	}
}

const char* Web_CheckOpenGlError(bool printUnkown = false)
{
	GLenum glError = jsGlGetError();
	if (glError != GL_NO_ERROR) { return Web_GetOpenGlErrorStr(glError, printUnkown); }
	else { return nullptr; }
}
#define Web_AssertNoOpenGlError() do                                               \
{                                                                                  \
	GLenum glError = jsGlGetError();                                               \
	if (glError != GL_NO_ERROR)                                                    \
	{                                                                              \
		PrintLine_E("Got OpenGL Error: %s", Web_GetOpenGlErrorStr(glError, true)); \
	}                                                                              \
	Assert(glError == GL_NO_ERROR);                                                \
} while(0)

#if DEBUG_BUILD
#define Web_DebugAssertNoOpenGlError() Web_AssertNoOpenGlError()
#else
#define Web_DebugAssertNoOpenGlError() //nothing
#endif

Web_Shader_t Web_CreateShader(const char* vertShaderStr, const char* fragShaderStr)
{
	Web_Shader_t result = {};
	
	result.vertId = jsGlCreateShader(GL_VERTEX_SHADER); Web_AssertNoOpenGlError();
	// PrintLine_D("vertId: %d", result.vertId);
	jsGlShaderSource(result.vertId, 1, vertShaderStr); Web_AssertNoOpenGlError();
	jsGlCompileShader(result.vertId); Web_AssertNoOpenGlError();
	
	result.fragId = jsGlCreateShader(GL_FRAGMENT_SHADER); Web_AssertNoOpenGlError();
	// PrintLine_D("fragId: %d", result.fragId);
	jsGlShaderSource(result.fragId, 1, fragShaderStr); Web_AssertNoOpenGlError();
	jsGlCompileShader(result.fragId); Web_AssertNoOpenGlError();
	
	result.glId = jsGlCreateProgram(); Web_AssertNoOpenGlError();
	jsGlAttachShader(result.glId, result.vertId); Web_AssertNoOpenGlError();
	jsGlAttachShader(result.glId, result.fragId); Web_AssertNoOpenGlError();
	jsGlLinkProgram(result.glId); Web_AssertNoOpenGlError();
	
	result.attribLocations.position  = jsGlGetAttribLocation(result.glId, "inPosition"); Web_AssertNoOpenGlError();
	result.attribLocations.color     = jsGlGetAttribLocation(result.glId, "inColor"); Web_AssertNoOpenGlError();
	result.attribLocations.texCoord  = jsGlGetAttribLocation(result.glId, "inTexCoord"); Web_AssertNoOpenGlError();
	result.attribLocations.normal    = jsGlGetAttribLocation(result.glId, "inNormal"); Web_AssertNoOpenGlError();
	result.attribLocations.tangent   = jsGlGetAttribLocation(result.glId, "inTangent"); Web_AssertNoOpenGlError();
	
	result.uniforms.worldMatrix      = jsGlGetUniformLocation(result.glId, "WorldMatrix"); Web_AssertNoOpenGlError();
	result.uniforms.viewMatrix       = jsGlGetUniformLocation(result.glId, "ViewMatrix"); Web_AssertNoOpenGlError();
	result.uniforms.projectionMatrix = jsGlGetUniformLocation(result.glId, "ProjectionMatrix"); Web_AssertNoOpenGlError();
	result.uniforms.texture          = jsGlGetUniformLocation(result.glId, "Texture"); Web_AssertNoOpenGlError();
	result.uniforms.textureSize      = jsGlGetUniformLocation(result.glId, "TextureSize"); Web_AssertNoOpenGlError();
	result.uniforms.sourceRec        = jsGlGetUniformLocation(result.glId, "SourceRec"); Web_AssertNoOpenGlError();
	result.uniforms.color1           = jsGlGetUniformLocation(result.glId, "Color1"); Web_AssertNoOpenGlError();
	result.uniforms.color2           = jsGlGetUniformLocation(result.glId, "Color2"); Web_AssertNoOpenGlError();
	result.uniforms.time             = jsGlGetUniformLocation(result.glId, "Time"); Web_AssertNoOpenGlError();
	
	return result;
}

Web_Texture_t Web_CreateTexture(v2i size, const u8* pixelData, bool pixelated, bool repeating)
{
	Web_Texture_t result = {};
	
	result.glId = jsGlCreateTexture();
	jsGlBindTexture(GL_TEXTURE_2D, result.glId);
	
	jsGlTexImage2D(
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
	
	jsGlTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (pixelated ? GL_NEAREST_MIPMAP_NEAREST : GL_LINEAR_MIPMAP_LINEAR));
	jsGlTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (pixelated ? GL_NEAREST : GL_LINEAR));
	jsGlTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (repeating ? GL_REPEAT : GL_CLAMP_TO_EDGE));
	jsGlTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (repeating ? GL_REPEAT : GL_CLAMP_TO_EDGE));
	jsGlGenerateMipmap(GL_TEXTURE_2D);
	
	result.isValid = true;
	return result;
}

bool Web_CreateVertBuffer_(MemArena_t* memArena, Web_VertBuffer_t* bufferOut, bool dynamic, u64 numVertices, void* verticesPntr, bool copyVertices, Web_VertexType_t vertexType, u64 vertexSize)
{
	NotNull(memArena);
	NotNull(bufferOut);
	Assert(verticesPntr != nullptr || numVertices == 0);
	Assert(vertexType != Web_VertexType_None);
	Assert(vertexSize > 0);
	ClearPointer(bufferOut);
	
	GLuint bufferId = jsGlCreateBuffer(); Web_AssertNoOpenGlError();
	jsGlBindBuffer(GL_ARRAY_BUFFER, bufferId); Web_AssertNoOpenGlError();
	jsGlBufferData(GL_ARRAY_BUFFER, numVertices * vertexSize, verticesPntr, (dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW)); Web_AssertNoOpenGlError();
	
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
#define Web_CreateVertBuffer2D(memArena, bufferOut, dynamic, numVertices, verticesPntr, copyVertices) Web_CreateVertBuffer_((memArena), (bufferOut), (dynamic), (numVertices), (verticesPntr), (copyVertices), Web_VertexType_Default2D, Web_VertexType_Default2D_Size)
#define Web_CreateVertBuffer3D(memArena, bufferOut, dynamic, numVertices, verticesPntr, copyVertices) Web_CreateVertBuffer_((memArena), (bufferOut), (dynamic), (numVertices), (verticesPntr), (copyVertices), Web_VertexType_Default3D, Web_VertexType_Default3D_Size)

Web_VertexArrayObject_t Web_CreateVertexArrayObject(Web_VertexType_t vertexType)
{
	Web_VertexArrayObject_t result = {};
	
	result.glId = jsGlCreateVertexArray(); Web_AssertNoOpenGlError();
	result.vertexType = vertexType;
	result.boundOnce = false;
	
	return result;
}

void Web_BindVertexArrayObject(Web_VertexArrayObject_t* vao, const Web_Shader_t* boundShader, const Web_VertBuffer_t* boundBuffer)
{
	jsGlBindVertexArray(vao->glId);
	if (!vao->boundOnce || true) //TODO: Remove me when we figure out why the assertion in the else statement is firing on laptop
	{
		for (u64 attIndex = 0; attIndex < WEB_MAX_NUM_VERTEX_ATTRIBUTES; attIndex++)
		{
			if (boundShader->attribLocations.values[attIndex] >= 0)
			{
				jsGlEnableVertexAttribArray(boundShader->attribLocations.values[attIndex]); Web_AssertNoOpenGlError();
			}
		}
		vao->boundOnce = true;
	}
	
	u8* attribOffset = nullptr;
	if (IsFlagSet(boundBuffer->vertexType, Web_VertexType_PositionBit))
	{
		if (boundShader->attribLocations.position >= 0)
		{
			jsGlVertexAttribPointer(boundShader->attribLocations.position, 3, GL_FLOAT, GL_FALSE, (GLsizei)boundBuffer->vertexSize, attribOffset); Web_AssertNoOpenGlError();
		}
		attribOffset += sizeof(v3);
	}
	if (IsFlagSet(boundBuffer->vertexType, Web_VertexType_ColorBit))
	{
		if (boundShader->attribLocations.color >= 0)
		{
			jsGlVertexAttribPointer(boundShader->attribLocations.color, 4, GL_FLOAT, GL_FALSE, (GLsizei)boundBuffer->vertexSize, attribOffset); Web_AssertNoOpenGlError();
		}
		attribOffset += sizeof(v4);
	}
	if (IsFlagSet(boundBuffer->vertexType, Web_VertexType_TexCoordBit))
	{
		if (boundShader->attribLocations.texCoord >= 0)
		{
			jsGlVertexAttribPointer(boundShader->attribLocations.texCoord, 2, GL_FLOAT, GL_FALSE, (GLsizei)boundBuffer->vertexSize, attribOffset); Web_AssertNoOpenGlError();
		}
		attribOffset += sizeof(v2);
	}
	if (IsFlagSet(boundBuffer->vertexType, Web_VertexType_NormalBit))
	{
		if (boundShader->attribLocations.normal >= 0)
		{
			jsGlVertexAttribPointer(boundShader->attribLocations.normal, 3, GL_FLOAT, GL_TRUE, (GLsizei)boundBuffer->vertexSize, attribOffset); Web_AssertNoOpenGlError();
		}
		attribOffset += sizeof(v3);
	}
	if (IsFlagSet(boundBuffer->vertexType, Web_VertexType_TangentBit))
	{
		if (boundShader->attribLocations.tangent >= 0)
		{
			jsGlVertexAttribPointer(boundShader->attribLocations.tangent, 3, GL_FLOAT, GL_TRUE, (GLsizei)boundBuffer->vertexSize, attribOffset); Web_AssertNoOpenGlError();
		}
		attribOffset += sizeof(v3);
	}
}
