/*
File:   pig_vertex_buffer.cpp
Author: Taylor Robbins
Date:   10\10\2021
Description: 
	** Holds functions that help us create and manipulate vertex buffers
*/

void DestroyVertBuffer(VertBuffer_t* buffer)
{
	NotNull(buffer);
	Assert((buffer->vertsVoidPntr != nullptr) == (buffer->hasVerticesCopy));
	if (buffer->vertsVoidPntr != nullptr)
	{
		NotNull(buffer->allocArena);
		FreeMem(buffer->allocArena, buffer->vertsVoidPntr, buffer->numVertices * buffer->vertexSize);
	}
	switch (pig->renderApi)
	{
		// +==============================+
		// |            OpenGL            |
		// +==============================+
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
			if (buffer->glId != 0)
			{
				glDeleteBuffers(1, &buffer->glId);
				AssertNoOpenGlError();
			}
		} break;
		#endif
		default: AssertMsg(false, "Unsupported API in DestroyVertBuffer"); break;
	}
	ClearPointer(buffer);
}

bool CreateVertBuffer_(MemArena_t* memArena, VertBuffer_t* bufferOut, bool dynamic, u64 numVertices, void* verticesPntr, bool copyVertices, VertexType_t vertexType, u64 vertexSize)
{
	NotNull(memArena);
	NotNull(bufferOut);
	Assert(verticesPntr != nullptr || numVertices == 0);
	Assert(vertexType != VertexType_None);
	Assert(vertexSize > 0);
	ClearPointer(bufferOut);
	
	bool result = false;
	switch (pig->renderApi)
	{
		// +==============================+
		// |            OpenGL            |
		// +==============================+
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
			const char* errorStr = nullptr;
			GLuint bufferId = 0;
			glGenBuffers(1, &bufferId);
			errorStr = CheckOpenGlError();
			if (errorStr != nullptr)
			{
				PrintLine_E("CreateVertBuffer glGenBuffers failed: %s", errorStr);
				return false;
			}
			glBindBuffer(GL_ARRAY_BUFFER, bufferId);
			errorStr = CheckOpenGlError();
			if (errorStr != nullptr)
			{
				PrintLine_E("CreateVertBuffer glBindBuffer failed: %s", errorStr);
				return false;
			}
			
			glBufferData(GL_ARRAY_BUFFER, numVertices * vertexSize, verticesPntr, (dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW));
			errorStr = CheckOpenGlError();
			if (errorStr != nullptr)
			{
				PrintLine_E("CreateVertBuffer glBufferData(%u vertices, %u bytes, %s) failed: %s", numVertices, numVertices * vertexSize, (dynamic ? "dynamic" : "static"), errorStr);
				glDeleteBuffers(1, &bufferId); AssertNoOpenGlError();
				return false;
			}
			
			if (copyVertices)
			{
				bufferOut->vertsVoidPntr = AllocMem(memArena, numVertices * vertexSize);
				NotNullMsg(bufferOut->vertsVoidPntr, "Failed to allocate vertex data on local heap in CreateVertBuffer");
				MyMemCopy(bufferOut->vertsVoidPntr, verticesPntr, numVertices * vertexSize);
				bufferOut->hasVerticesCopy = true;
			}
			
			bufferOut->glId = bufferId;
			bufferOut->id = pig->nextVertBufferId;
			pig->nextVertBufferId++;
			bufferOut->allocArena = memArena;
			bufferOut->isDynamic = dynamic;
			bufferOut->numVertices = numVertices;
			bufferOut->vertexType = vertexType;
			bufferOut->vertexSize = vertexSize;
			bufferOut->isValid = true;
			result = true;
		} break;
		#endif
		
		// +==============================+
		// |       Unsupported API        |
		// +==============================+
		default:
		{
			WriteLine_E("Unsupported render API in CreateVertBuffer!");
			result = false;
		} break;
	}
	
	return result;
}
#define CreateVertBuffer2D(memArena, bufferOut, dynamic, numVertices, verticesPntr, copyVertices) CreateVertBuffer_((memArena), (bufferOut), (dynamic), (numVertices), (verticesPntr), (copyVertices), VertexType_Default2D, VertexType_Default2D_Size)
#define CreateVertBuffer3D(memArena, bufferOut, dynamic, numVertices, verticesPntr, copyVertices) CreateVertBuffer_((memArena), (bufferOut), (dynamic), (numVertices), (verticesPntr), (copyVertices), VertexType_Default3D, VertexType_Default3D_Size)

bool CreateVertBufferFromIndexedPrimitiveVerts3D(MemArena_t* memArena, VertBuffer_t* bufferOut, bool dynamic, const PrimitiveIndexedVerts_t* primVerts, Color_t color, bool copyVertices)
{
	Assert(memArena != TempArena);
	NotNull(primVerts);
	Assert(primVerts->numIndices > 0);
	TempPushMark();
	
	v4 colorVec = ToVec4(color);
	Vertex3D_t* vertices = AllocArray(TempArena, Vertex3D_t, primVerts->numIndices);
	NotNull(vertices);
	for (u64 iIndex = 0; iIndex < primVerts->numIndices; iIndex++)
	{
		u64 vertIndex = primVerts->indices[iIndex].index;
		Assert(vertIndex < primVerts->numVertices);
		vertices[iIndex].position = primVerts->vertices[vertIndex].position;
		vertices[iIndex].color = colorVec;
		vertices[iIndex].texCoord = primVerts->indices[iIndex].texCoord;
		vertices[iIndex].normal = primVerts->indices[iIndex].normal;
		// vertices[iIndex].color = ToVec4(((iIndex % 6) < 3) ? color : ColorComplementary(color)); //DEBUG
	}
	bool result = CreateVertBuffer3D(memArena, bufferOut, dynamic, primVerts->numIndices, vertices, copyVertices);
	
	TempPopMark();
	return result;
}

bool ChangeVertBufferVertices_(VertBuffer_t* buffer, u64 startIndex, u64 numVertices, void* verticesPntr, VertexType_t vertexType, u64 vertexSize)
{
	NotNull(buffer);
	Assert(buffer->isValid);
	Assert(buffer->isDynamic);
	Assert(verticesPntr != nullptr || numVertices == 0);
	Assert(vertexSize == buffer->vertexSize);
	Assert(vertexType == buffer->vertexType);
	Assert(startIndex <= buffer->numVertices);
	Assert(startIndex + numVertices <= buffer->numVertices);
	if (numVertices == 0) { return true; }
	
	glBindBuffer(GL_ARRAY_BUFFER, buffer->glId);
	const char* errorStr = CheckOpenGlError();
	if (errorStr != nullptr)
	{
		PrintLine_E("ChangeVertBufferVertices glBindBuffer failed: %s", errorStr);
		return false;
	}
	
	glBufferSubData(GL_ARRAY_BUFFER, startIndex * vertexSize, numVertices * vertexSize, verticesPntr);
	errorStr = CheckOpenGlError();
	if (errorStr != nullptr)
	{
		PrintLine_E("ChangeVertBufferVertices glBufferSubData(%u startIndex, %u vertices, %u bytes) failed: %s", startIndex, numVertices, numVertices * vertexSize, errorStr);
		return false;
	}
	
	if (buffer->hasVerticesCopy)
	{
		NotNull(buffer->vertsVoidPntr);
		MyMemCopy(((u8*)buffer->vertsVoidPntr) + (startIndex * buffer->vertexSize), verticesPntr, numVertices * buffer->vertexSize);
	}
	
	return true;
}
#define ChangeVertBufferVertices2D(buffer, startIndex, numVertices, verticesPntr) ChangeVertBufferVertices_((buffer), (startIndex), (numVertices), (verticesPntr), VertexType_Default2D, VertexType_Default2D_Size)
#define ChangeVertBufferVertices3D(buffer, startIndex, numVertices, verticesPntr) ChangeVertBufferVertices_((buffer), (startIndex), (numVertices), (verticesPntr), VertexType_Default3D, VertexType_Default3D_Size)
