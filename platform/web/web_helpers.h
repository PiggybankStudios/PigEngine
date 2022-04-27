/*
File:   web_helpers.h
Author: Taylor Robbins
Date:   04\26\2022
*/

#ifndef _WEB_HELPERS_H
#define _WEB_HELPERS_H

//TODO: Remove me when we are done testing gl functionality without the application

struct Shader_t
{
	GLuint glId;
	GLuint vertId;
	GLuint fragId;
	
	union
	{
		GLint values[8];
		struct
		{
			GLint position;
			GLint color1;
			GLint color2;
			GLint texCoord1;
			GLint texCoord2;
			GLint normal1;
			GLint normal2;
			GLint tangent;
		};
	} attribLocations;
};

enum VertexType_t
{
	VertexType_None = 0x00,
	VertexType_All  = 0xFF,
	
	VertexType_PositionBit  = 0x01,
	VertexType_Color1Bit    = 0x02,
	VertexType_Color2Bit    = 0x04,
	VertexType_TexCoord1Bit = 0x08,
	VertexType_TexCoord2Bit = 0x10,
	VertexType_Normal1Bit   = 0x20,
	VertexType_Normal2Bit   = 0x40,
	VertexType_TangentBit   = 0x80,
	
	VertexType_Default2D = (VertexType_PositionBit|VertexType_Color1Bit|VertexType_TexCoord1Bit),
	VertexType_Default3D = (VertexType_PositionBit|VertexType_Color1Bit|VertexType_TexCoord1Bit|VertexType_Normal1Bit),
};

// VertexType_Default2D
START_PACK()
struct ATTR_PACKED Vertex2D_t
{
	union
	{
		v3 position;
		struct { r32 x, y, z; };
	};
	union
	{
		v4 color;
		struct { r32 r, g, b, a; };
	};
	union
	{
		v2 texCoord;
		struct { r32 u, v; };
	};
};
END_PACK()
#define VertexType_Default2D_Size sizeof(Vertex2D_t)

// VertexType_Default3D
START_PACK()
struct ATTR_PACKED Vertex3D_t
{
	union
	{
		v3 position;
		struct { r32 x, y, z; };
	};
	union
	{
		v4 color;
		struct { r32 r, g, b, a; };
	};
	union
	{
		v2 texCoord;
		struct { r32 u, v; };
	};
	union
	{
		v3 normal;
		struct { r32 nX, nY, nZ; };
	};
};
END_PACK()
#define VertexType_Default3D_Size sizeof(Vertex3D_t)

struct VertBuffer_t
{
	MemArena_t* allocArena;
	bool isValid;
	
	bool isDynamic;
	u64 numVertices;
	VertexType_t vertexType;
	u64 vertexSize;
	bool hasVerticesCopy;
	union
	{
		void* vertsVoidPntr;
		Vertex2D_t* verts2D;
		Vertex3D_t* verts3D;
	};
	
	GLuint glId;
};

#define MAX_NUM_VERTEX_ATTRIBUTES 8
struct VertexAttribLocations_t
{
	union
	{
		GLint values[MAX_NUM_VERTEX_ATTRIBUTES];
		struct
		{
			GLint position;
			GLint color1;
			GLint color2;
			GLint texCoord1;
			GLint texCoord2;
			GLint normal1;
			GLint normal2;
			GLint tangent;
		};
	} gl;
};

struct VertexArrayObject_t
{
	VertexType_t vertexType;
	GLuint glId;
	bool boundOnce;
	VertexAttribLocations_t attribLocations;
};

#endif //  _WEB_HELPERS_H
