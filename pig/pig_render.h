/*
File:   pig_render.h
Author: Taylor Robbins
Date:   09\26\2021
*/

#ifndef _PIG_RENDER_H
#define _PIG_RENDER_H

enum VertBufferPrimitive_t
{
	VertBufferPrimitive_None = 0,
	VertBufferPrimitive_Lines,
	VertBufferPrimitive_Triangles,
	VertBufferPrimitive_Quads,
	VertBufferPrimitive_NumTypes,
};
const char* GetVertBufferPrimitiveStr(VertBufferPrimitive_t primitive)
{
	switch (primitive)
	{
		case VertBufferPrimitive_None:      return "Triangles";
		case VertBufferPrimitive_Lines:     return "Lines";
		case VertBufferPrimitive_Triangles: return "Triangles";
		case VertBufferPrimitive_Quads:     return "Triangles";
		default: return "Unknown";
	}
}

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

#define MAX_NUM_VERTEX_ATTRIBUTES 8
struct VertexAttribLocations_t
{
	#if OPENGL_SUPPORTED
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
	#endif
};

//NOTE: Vertex Array Objects are not shared between contexts in OpenGL so we have to create unique VAOs for each window
struct VertexArrayObject_t
{
	u64 windowId;
	VertexType_t vertexType;
	#if OPENGL_SUPPORTED
	GLuint glId;
	#endif
	bool boundOnce;
	VertexAttribLocations_t attribLocations;
};

enum ShaderUniform_t
{
	ShaderUniform_None              = 0x00000000,
	
	ShaderUniform_WorldMatrix       = 0x00000001,
	ShaderUniform_ViewMatrix        = 0x00000002,
	ShaderUniform_ProjectionMatrix  = 0x00000004,
	
	ShaderUniform_CameraPosition    = 0x00000008,
	
	ShaderUniform_Texture1          = 0x00000010,
	ShaderUniform_Texture1Size      = 0x00000020,
	ShaderUniform_Texture2          = 0x00000040,
	ShaderUniform_Texture2Size      = 0x00000080,
	
	ShaderUniform_SourceRec1        = 0x00000100,
	ShaderUniform_SourceRec2        = 0x00000200,
	ShaderUniform_MaskRectangle     = 0x00000400,
	ShaderUniform_ShiftVec          = 0x00000800,
	
	ShaderUniform_Color1            = 0x00001000,
	ShaderUniform_Color2            = 0x00002000,
	ShaderUniform_ReplaceColors     = 0x00004000,
	
	ShaderUniform_Time              = 0x00008000,
	ShaderUniform_Brightness        = 0x00010000,
	ShaderUniform_Saturation        = 0x00020000,
	ShaderUniform_CircleRadius      = 0x00040000,
	ShaderUniform_CircleInnerRadius = 0x00080000,
	
	ShaderUniform_Value0            = 0x00100000,
	ShaderUniform_Value1            = 0x00200000,
	ShaderUniform_Value2            = 0x00400000,
	ShaderUniform_Value3            = 0x00800000,
	ShaderUniform_Value4            = 0x01000000,
	ShaderUniform_Value5            = 0x02000000,
	ShaderUniform_Value6            = 0x04000000,
	ShaderUniform_Value7            = 0x08000000,
	
	ShaderUniform_All               = 0x0FFFFFFF,
	
	ShaderUniform_RequireNone = ShaderUniform_None,
	ShaderUniform_RequireMatrices = (ShaderUniform_WorldMatrix|ShaderUniform_ViewMatrix|ShaderUniform_ProjectionMatrix),
	ShaderUniform_RequireTexture  = (ShaderUniform_Texture1|ShaderUniform_Texture1Size),
	ShaderUniform_RequireCircle   = (ShaderUniform_CircleRadius|ShaderUniform_CircleInnerRadius),
	ShaderUniform_NumGenericValues = 8,
};

enum ShaderError_t
{
	ShaderError_None = 0,
	ShaderError_CouldntOpenFile,
	ShaderError_Empty,
	ShaderError_UnsupportedApi,
	ShaderError_ApiError,
	ShaderError_CompilationNotSupported,
	ShaderError_MissingDivider,
	ShaderError_VertexCreationFailed,
	ShaderError_VertexCompileFailed,
	ShaderError_FragmentCreationFailed,
	ShaderError_FragmentCompileFailed,
	ShaderError_ShaderCreationFailed,
	ShaderError_LinkingFailed,
	ShaderError_VertexArrayCreationFailed,
	ShaderError_MissingAttribute,
	ShaderError_MissingUniform,
	ShaderError_NumErrors,
};
const char* GetShaderErrorStr(ShaderError_t error)
{
	switch (error)
	{
		case ShaderError_None:                      return "None";
		case ShaderError_CouldntOpenFile:           return "CouldntOpenFile";
		case ShaderError_Empty:                     return "Empty";
		case ShaderError_UnsupportedApi:            return "UnsupportedApi";
		case ShaderError_ApiError:                  return "ApiError";
		case ShaderError_CompilationNotSupported:   return "CompilationNotSupported";
		case ShaderError_MissingDivider:            return "MissingDivider";
		case ShaderError_VertexCreationFailed:      return "VertexCreationFailed";
		case ShaderError_VertexCompileFailed:       return "VertexCompileFailed";
		case ShaderError_FragmentCreationFailed:    return "FragmentCreationFailed";
		case ShaderError_FragmentCompileFailed:     return "FragmentCompileFailed";
		case ShaderError_ShaderCreationFailed:      return "ShaderCreationFailed";
		case ShaderError_LinkingFailed:             return "LinkingFailed";
		case ShaderError_VertexArrayCreationFailed: return "VertexArrayCreationFailed";
		case ShaderError_MissingAttribute:          return "MissingAttribute";
		case ShaderError_MissingUniform:            return "MissingUniform";
		default: return "Unknown";
	}
}

struct ShaderDynamicUniform_t
{
	u64 uniformIndex;
	MyStr_t name;
	#if OPENGL_SUPPORTED
	GLint glLocation;
	#endif
};

struct Shader_t
{
	bool isValid;
	u64 id;
	ShaderError_t error;
	MyStr_t apiErrorStr;
	MemArena_t* allocArena;
	VertexType_t vertexType;
	u32 requiredUniforms;
	u32 uniformFlags;
	
	VarArray_t dynamicUniforms; //ShaderDynamicUniform_t
	
	#if OPENGL_SUPPORTED
	GLuint glId;
	GLuint glVertId;
	GLuint glFragId;
	#endif
	
	MyStr_t vertexLog;
	MyStr_t fragmentLog;
	MyStr_t linkLog;
	
	VertexAttribLocations_t attribLocations;
	
	#if OPENGL_SUPPORTED
	struct
	{
		//Matrices
		GLint worldMatrix;
		GLint viewMatrix;
		GLint projectionMatrix;
		
		//3D Stuff
		GLint cameraPosition;
		
		//Textures
		GLint texture1;
		GLint texture1Size;
		GLint texture2;
		GLint texture2Size;
		
		//Tex Coord Stuff
		GLint sourceRec1;
		GLint sourceRec2;
		GLint maskRectangle;
		GLint shiftVec;
		
		//Colors
		GLint color1;
		GLint color2;
		GLint replaceColors;
		
		//Other Effects
		GLint time;
		GLint brightness;
		GLint saturation;
		GLint circleRadius;
		GLint circleInnerRadius;
		
		//Generic "Value"s
		GLint value[ShaderUniform_NumGenericValues];
	} glLocations;
	#endif
};

struct VertBuffer_t
{
	MemArena_t* allocArena;
	bool isValid;
	u64 id;
	
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
	
	#if OPENGL_SUPPORTED
	GLuint glId;
	#endif
};

enum TextureError_t
{
	TextureError_None = 0,
	TextureError_CouldntOpenFile,
	TextureError_EmptyFile,
	TextureError_ParseFailure,
	TextureError_UnsupportedApi,
	TextureError_ApiError,
	TextureError_NumErrors,
};
const char* GetTextureErrorStr(TextureError_t error)
{
	switch (error)
	{
		case TextureError_None:            return "None";
		case TextureError_CouldntOpenFile: return "CouldntOpenFile";
		case TextureError_EmptyFile:       return "EmptyFile";
		case TextureError_ParseFailure:    return "ParseFailure";
		case TextureError_UnsupportedApi:  return "UnsupportedApi";
		case TextureError_ApiError:        return "ApiFailure";
		default: return "Unknown";
	}
}

struct Texture_t
{
	bool isValid;
	u64 id;
	TextureError_t error;
	MyStr_t apiErrorStr;
	MemArena_t* allocArena;
	
	bool isPixelated;
	bool isRepeating;
	bool hasAlpha;
	bool singleChannel;
	
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
	
	#if OPENGL_SUPPORTED
	GLuint glId;
	#endif
};

enum SpriteSheetError_t
{
	SpriteSheetError_None = 0,
	SpriteSheetError_CouldntOpenFile,
	SpriteSheetError_EmptyFile,
	SpriteSheetError_ParseFailure,
	SpriteSheetError_AllocFailure,
	SpriteSheetError_TextureError,
	SpriteSheetError_NumErrors,
};
const char* GetSpriteSheetErrorStr(SpriteSheetError_t error)
{
	switch (error)
	{
		case SpriteSheetError_None:            return "None";
		case SpriteSheetError_CouldntOpenFile: return "CouldntOpenFile";
		case SpriteSheetError_EmptyFile:       return "EmptyFile";
		case SpriteSheetError_ParseFailure:    return "ParseFailure";
		case SpriteSheetError_AllocFailure:    return "AllocFailure";
		case SpriteSheetError_TextureError:    return "TextureError";
		default: return "Unknown";
	}
}

struct SpriteSheetFramePoint_t
{
	v2 point;
	MyStr_t name;
};

struct SpriteSheetFrame_t
{
	u64 index;
	v2i gridPos;
	bool filled;
	MyStr_t name;
	u32 codepoint;
	r32 advanceX;
	reci charBounds;
	reci logicalBounds;
	v2i charOffset;
	VarArray_t points; //SpriteSheetFramePoint_t
};

struct SpriteSheet_t
{
	bool isValid;
	u64 id;
	MemArena_t* allocArena;
	SpriteSheetError_t error;
	
	Texture_t texture;
	union
	{
		v2i numFrames;
		struct { i32 numFramesX, numFramesY; };
	};
	union
	{
		v2i frameSize;
		struct { i32 frameWidth, frameHeight; };
	};
	union
	{
		v2i padding;
		struct { i32 paddingX, paddingY; };
	};
	
	u64 numFilledFrames;
	VarArray_t frames; //SpriteSheetFrame_t
};

struct ModelMaterial_t
{
	u64 materialIndex;
	MyStr_t name;
	r32 specularExponent;
	// r32 opticalDensity; //TODO: ?? always 0.0
	// r32 dissolve; //TODO: ?? 1.0 for most things and 0.0 for wooden fence and metal cage
	// i8 illuminationType; //TODO: What are the valid types? always 2
	Color_t ambientColor;
	Color_t diffuseColor;
	Color_t specularColor;
	// Texture_t ambientTexture;
	Texture_t diffuseTexture;
	// Texture_t specularTexture;
};
struct ModelPart_t
{
	u64 partIndex;
	u64 materialIndex;
	VertBuffer_t buffer;
};
struct Model_t
{
	MemArena_t* allocArena;
	VarArray_t materials; //ModelMaterial_t
	VarArray_t parts; //ModelPart_t
};

#endif //  _PIG_RENDER_H
