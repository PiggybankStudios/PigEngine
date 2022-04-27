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
const char* simpleFragmentShader = "#version 300 es\nprecision highp float;\nin vec3 fPosition;\nin vec4 fColor;\nout vec4 fragColor;\nvoid main()\n{\n\tfragColor = fColor;\n\tfragColor.gb = fPosition.xy;\n}";

Shader_t CreateShader(const char* vertShaderStr, const char* fragShaderStr)
{
	Shader_t result = {};
	
	result.vertId = glCreateShader(GL_VERTEX_SHADER);
	// PrintLine_D("vertId: %d", result.vertId);
	glShaderSource(result.vertId, 1, vertShaderStr);
	glCompileShader(result.vertId);
	
	result.fragId = glCreateShader(GL_FRAGMENT_SHADER);
	// PrintLine_D("fragId: %d", result.fragId);
	glShaderSource(result.fragId, 1, fragShaderStr);
	glCompileShader(result.fragId);
	
	result.glId = glCreateProgram();
	glAttachShader(result.glId, result.vertId);
	glAttachShader(result.glId, result.fragId);
	glLinkProgram(result.glId);
	
	result.attribLocations.position  = glGetAttribLocation(result.glId, "inPosition");
	result.attribLocations.color1    = glGetAttribLocation(result.glId, "inColor1");
	result.attribLocations.color2    = glGetAttribLocation(result.glId, "inColor2");
	result.attribLocations.texCoord1 = glGetAttribLocation(result.glId, "inTexCoord1");
	result.attribLocations.texCoord2 = glGetAttribLocation(result.glId, "inTexCoord2");
	result.attribLocations.normal1   = glGetAttribLocation(result.glId, "inNormal1");
	result.attribLocations.normal2   = glGetAttribLocation(result.glId, "inNormal2");
	result.attribLocations.tangent   = glGetAttribLocation(result.glId, "inTangent");
	
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
	
	GLuint bufferId = glCreateBuffer();
	glBindBuffer(GL_ARRAY_BUFFER, bufferId);
	glBufferData(GL_ARRAY_BUFFER, numVertices * vertexSize, verticesPntr, (dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW));
	
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
	
	result.glId = glCreateVertexArray();
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
				glEnableVertexAttribArray(boundShader->attribLocations.values[attIndex]);
			}
		}
		vao->boundOnce = true;
	}
	
	u8* attribOffset = nullptr;
	if (IsFlagSet(boundBuffer->vertexType, VertexType_PositionBit))
	{
		glVertexAttribPointer(boundShader->attribLocations.position, 3, GL_FLOAT, GL_FALSE, (GLsizei)boundBuffer->vertexSize, attribOffset);
		attribOffset += sizeof(v3);
	}
	if (IsFlagSet(boundBuffer->vertexType, VertexType_Color1Bit))
	{
		glVertexAttribPointer(boundShader->attribLocations.color1, 4, GL_FLOAT, GL_FALSE, (GLsizei)boundBuffer->vertexSize, attribOffset);
		attribOffset += sizeof(v4);
	}
	if (IsFlagSet(boundBuffer->vertexType, VertexType_Color2Bit))
	{
		glVertexAttribPointer(boundShader->attribLocations.color2, 4, GL_FLOAT, GL_FALSE, (GLsizei)boundBuffer->vertexSize, attribOffset);
		attribOffset += sizeof(v4);
	}
	if (IsFlagSet(boundBuffer->vertexType, VertexType_TexCoord1Bit))
	{
		glVertexAttribPointer(boundShader->attribLocations.texCoord1, 2, GL_FLOAT, GL_FALSE, (GLsizei)boundBuffer->vertexSize, attribOffset);
		attribOffset += sizeof(v2);
	}
	if (IsFlagSet(boundBuffer->vertexType, VertexType_TexCoord2Bit))
	{
		glVertexAttribPointer(boundShader->attribLocations.texCoord2, 2, GL_FLOAT, GL_FALSE, (GLsizei)boundBuffer->vertexSize, attribOffset);
		attribOffset += sizeof(v2);
	}
	if (IsFlagSet(boundBuffer->vertexType, VertexType_Normal1Bit))
	{
		glVertexAttribPointer(boundShader->attribLocations.normal1, 3, GL_FLOAT, GL_TRUE, (GLsizei)boundBuffer->vertexSize, attribOffset);
		attribOffset += sizeof(v3);
	}
	if (IsFlagSet(boundBuffer->vertexType, VertexType_Normal2Bit))
	{
		glVertexAttribPointer(boundShader->attribLocations.normal2, 3, GL_FLOAT, GL_TRUE, (GLsizei)boundBuffer->vertexSize, attribOffset);
		attribOffset += sizeof(v3);
	}
	if (IsFlagSet(boundBuffer->vertexType, VertexType_TangentBit))
	{
		glVertexAttribPointer(boundShader->attribLocations.tangent, 3, GL_FLOAT, GL_TRUE, (GLsizei)boundBuffer->vertexSize, attribOffset);
		attribOffset += sizeof(v3);
	}
}
