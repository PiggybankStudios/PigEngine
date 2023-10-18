/*
File:   web_render_types.h
Author: Taylor Robbins
Date:   10\16\2023
*/

#ifndef _WEB_RENDER_TYPES_H
#define _WEB_RENDER_TYPES_H

struct Web_Shader_t
{
	GLuint glId;
	GLuint vertId;
	GLuint fragId;
	
	union
	{
		GLint values[5];
		struct
		{
			GLint position;
			GLint color;
			GLint texCoord;
			GLint normal;
			GLint tangent;
		};
	} attribLocations;
	
	struct
	{
		GLint worldMatrix;
		GLint viewMatrix;
		GLint projectionMatrix;
		GLint texture;
		GLint textureSize;
		GLint sourceRec;
		GLint color1;
		GLint color2;
		GLint time;
	} uniforms;
};

struct Web_Texture_t
{
	bool isValid;
	GLint glId;
	
	union
	{
		v2 size;
		struct { r32 width, height; };
	};
	union
	{
		v2i sizei;
		struct { i32 widthi, heighti; };
	};
};

enum Web_VertexType_t
{
	Web_VertexType_None = 0x00,
	Web_VertexType_All  = 0x1F,
	
	Web_VertexType_PositionBit  = 0x01,
	Web_VertexType_ColorBit     = 0x02,
	Web_VertexType_TexCoordBit  = 0x04,
	Web_VertexType_NormalBit    = 0x08,
	Web_VertexType_TangentBit   = 0x10,
	
	Web_VertexType_Default2D = (Web_VertexType_PositionBit|Web_VertexType_ColorBit|Web_VertexType_TexCoordBit),
	Web_VertexType_Default3D = (Web_VertexType_PositionBit|Web_VertexType_ColorBit|Web_VertexType_TexCoordBit|Web_VertexType_NormalBit),
};

// VertexType_Default2D
START_PACK()
struct ATTR_PACKED Web_Vertex2D_t
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
#define Web_VertexType_Default2D_Size sizeof(Web_Vertex2D_t)

// VertexType_Default3D
START_PACK()
struct ATTR_PACKED Web_Vertex3D_t
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
#define Web_VertexType_Default3D_Size sizeof(Vertex3D_t)

struct Web_VertBuffer_t
{
	MemArena_t* allocArena;
	bool isValid;
	
	bool isDynamic;
	u64 numVertices;
	Web_VertexType_t vertexType;
	u64 vertexSize;
	bool hasVerticesCopy;
	union
	{
		void* vertsVoidPntr;
		Web_Vertex2D_t* verts2D;
		Web_Vertex3D_t* verts3D;
	};
	
	GLuint glId;
};

#define WEB_MAX_NUM_VERTEX_ATTRIBUTES 5
struct Web_VertexAttribLocations_t
{
	union
	{
		GLint values[WEB_MAX_NUM_VERTEX_ATTRIBUTES];
		struct
		{
			GLint position;
			GLint color;
			GLint texCoord;
			GLint normal;
			GLint tangent;
		};
	} gl;
};

struct Web_VertexArrayObject_t
{
	Web_VertexType_t vertexType;
	GLuint glId;
	bool boundOnce;
	Web_VertexAttribLocations_t attribLocations;
};

#endif //  _WEB_RENDER_TYPES_H
