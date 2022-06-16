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
	Assert((buffer->indicesVoidPntr != nullptr) == (buffer->hasIndicesCopy));
	if (buffer->indicesVoidPntr != nullptr)
	{
		NotNull(buffer->allocArena);
		FreeMem(buffer->allocArena, buffer->indicesVoidPntr, buffer->numIndices * buffer->indexSize);
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
			if (buffer->glIndexId != 0)
			{
				glDeleteBuffers(1, &buffer->glIndexId);
				AssertNoOpenGlError();
			}
		} break;
		#endif
		default: AssertMsg(false, "Unsupported API in DestroyVertBuffer"); break;
	}
	ClearPointer(buffer);
}

//NOTE: If this function returns false, you should still call DestroyVertBuffer after checking error codes/strings
bool CreateVertBuffer_(MemArena_t* memArena, VertBuffer_t* bufferOut, bool dynamic,
	u64 numVertices, VertexType_t vertexType, u64 vertexSize, const void* verticesPntr, bool copyVertices)
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
				NotNullMsg(bufferOut->vertsVoidPntr, "Failed to allocate vertex data in CreateVertBuffer");
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
#define CreateVertBuffer2D(memArena, bufferOut, dynamic, numVertices, verticesPntr, copyVertices) CreateVertBuffer_((memArena), (bufferOut), (dynamic), (numVertices), VertexType_Default2D, VertexType_Default2D_Size, (verticesPntr), (copyVertices))
#define CreateVertBuffer3D(memArena, bufferOut, dynamic, numVertices, verticesPntr, copyVertices) CreateVertBuffer_((memArena), (bufferOut), (dynamic), (numVertices), VertexType_Default3D, VertexType_Default3D_Size, (verticesPntr), (copyVertices))

bool CreateVertBufferWithIndices_(MemArena_t* memArena, VertBuffer_t* bufferOut, bool dynamic,
	u64 numVertices, VertexType_t vertexType, u64 vertexSize, const void* verticesPntr, bool copyVertices,
	u64 numIndices, u64 indexSize, const void* indicesPntr, bool copyIndices)
{
	Assert(indexSize > 0);
	AssertIf(numIndices > 0, indicesPntr != nullptr);
	bool result = CreateVertBuffer_(memArena, bufferOut, dynamic, numVertices, vertexType, vertexSize, verticesPntr, copyVertices);
	if (!result || numIndices == 0) { return result; }
	
	switch (pig->renderApi)
	{
		// +==============================+
		// |            OpenGL            |
		// +==============================+
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
			const char* errorStr = nullptr;
			glGenBuffers(1, &bufferOut->glIndexId);
			errorStr = CheckOpenGlError();
			if (errorStr != nullptr)
			{
				PrintLine_E("CreateVertBufferWithIndices glGenBuffers failed: %s", errorStr);
				return false;
			}
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferOut->glIndexId);
			errorStr = CheckOpenGlError();
			if (errorStr != nullptr)
			{
				PrintLine_E("CreateVertBufferWithIndices glBindBuffer failed: %s", errorStr);
				return false;
			}
			
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * indexSize, indicesPntr, (dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW));
			errorStr = CheckOpenGlError();
			if (errorStr != nullptr)
			{
				PrintLine_E("CreateVertBufferWithIndices glBufferData(%u indices, %u bytes, %s) failed: %s", numIndices, numIndices * indexSize, (dynamic ? "dynamic" : "static"), errorStr);
				glDeleteBuffers(1, &bufferOut->glIndexId); AssertNoOpenGlError();
				return false;
			}
			
			bufferOut->numIndices = numIndices;
			bufferOut->indexSize = indexSize;
			if (copyIndices)
			{
				bufferOut->indicesVoidPntr = AllocMem(memArena, numIndices * indexSize);
				NotNullMsg(bufferOut->indicesVoidPntr, "Failed to allocate index data in CreateVertBufferWithIndices");
				MyMemCopy(bufferOut->indicesVoidPntr, indicesPntr, numIndices * indexSize);
				bufferOut->hasIndicesCopy = true;
			}
		} break;
		#endif
	}
	
	return result;
}

#define CreateVertBufferWithIndices2D(memArena, bufferOut, dynamic, numVertices, verticesPntr, numIndices, indicesPntrI32, copyVertices, copyIndices) CreateVertBufferWithIndices_((memArena), (bufferOut), (dynamic), (numVertices), VertexType_Default2D, VertexType_Default2D_Size, (verticesPntr), (copyVertices), (numIndices), sizeof(i32), (indicesPntr), (copyIndices))
#define CreateVertBufferWithIndices3D(memArena, bufferOut, dynamic, numVertices, verticesPntr, numIndices, indicesPntrI32, copyVertices, copyIndices) CreateVertBufferWithIndices_((memArena), (bufferOut), (dynamic), (numVertices), VertexType_Default3D, VertexType_Default3D_Size, (verticesPntr), (copyVertices), (numIndices), sizeof(i32), (indicesPntr), (copyIndices))

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
	
	bool result = false;
	switch (pig->renderApi)
	{
		// +==============================+
		// |            OpenGL            |
		// +==============================+
		#if OPENGL_SUPPORTED
		case RenderApi_OpenGL:
		{
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
			
			result = true;
		} break;
		#endif
		
		// +==============================+
		// |       Unsupported API        |
		// +==============================+
		default:
		{
			WriteLine_E("Unsupported render API in ChangeVertBufferVertices!");
			result = false;
		} break;
	}
	
	return result;
}
#define ChangeVertBufferVertices2D(buffer, startIndex, numVertices, verticesPntr) ChangeVertBufferVertices_((buffer), (startIndex), (numVertices), (verticesPntr), VertexType_Default2D, VertexType_Default2D_Size)
#define ChangeVertBufferVertices3D(buffer, startIndex, numVertices, verticesPntr) ChangeVertBufferVertices_((buffer), (startIndex), (numVertices), (verticesPntr), VertexType_Default3D, VertexType_Default3D_Size)
