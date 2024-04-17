/*
File:   pig_vector_img.h
Author: Taylor Robbins
Date:   01\27\2022
*/

#ifndef _PIG_VECTOR_IMG_H
#define _PIG_VECTOR_IMG_H

#define VECTOR_IMG_MAX_PART_TREE_DEPTH   16 //parts within parts within parts...

enum VectorImgFillType_t
{
	VectorImgFillType_None = 0,
	VectorImgFillType_Solid,
	VectorImgFillType_NumTypes,
};
struct VectorImgFill_t
{
	VectorImgFillType_t type;
	Color_t color;
};

enum VectorImgStrokeType_t
{
	VectorImgStrokeType_None = 0,
	VectorImgStrokeType_Solid,
	VectorImgStrokeType_NumTypes,
};
struct VectorImgStroke_t
{
	VectorImgStrokeType_t type;
	Color_t color;
	r32 thickness;
};

enum VectorImgShapeType_t
{
	VectorImgShapeType_None = 0,
	VectorImgShapeType_Circle,
	VectorImgShapeType_Rectangle,
	VectorImgShapeType_Path,
	VectorImgShapeType_NumTypes,
};
const char* GetVectorImgShapeTypeStr(VectorImgShapeType_t shapeType)
{
	switch (shapeType)
	{
		case VectorImgShapeType_None:      return "None";
		case VectorImgShapeType_Circle:    return "Circle";
		case VectorImgShapeType_Rectangle: return "Rectangle";
		case VectorImgShapeType_Path:      return "Path";
		default: return "Unknown";
	}
}

struct VectorImgShape_t
{
	u64 id;
	u64 index;
	MyStr_t idStr;
	MyStr_t name;
	
	VectorImgFill_t fill;
	VectorImgStroke_t stroke;
	
	rec bounds;
	v2 origin;
	
	VectorImgShapeType_t type;
	union
	{
		// +==============================+
		// |  VectorImgShapeType_Circle   |
		// +==============================+
		struct
		{
			v2 center;
			r32 radius;
		} circle;
		// +==============================+
		// | VectorImgShapeType_Rectangle |
		// +==============================+
		struct
		{
			union
			{
				obb2 value;
				struct { v2 center, size; r32 rotation; };
			};
			v2 roundedRadius;
		} rectangle;
		// +==============================+
		// |   VectorImgShapeType_Path    |
		// +==============================+
		struct
		{
			BezierPath_t value;
			bool vertBufferUpToDate;
			bool vertBufferGenFailed;
			VertBuffer_t vertBuffer;
		} path;
	};
};

struct VectorImgPart_t
{
	u64 id;
	u64 index;
	u64 interleaveIndex;
	MyStr_t idStr;
	MyStr_t name;
	
	rec bounds;
	v2 origin;
	VarArray_t shapes; //VectorImgShape_t
	
	VarArray_t children; //VectorImgPart_t
};

struct VectorImg_t
{
	u64 id;
	bool isValid;
	MemArena_t* allocArena;
	Result_t error;
	XmlParsingError_t xmlError;
	
	u64 totalNumParts;
	u64 totalNumShapes;
	u64 nextPartId;
	u64 nextShapeId;
	
	VectorImgPart_t root;
};

#endif //  _PIG_VECTOR_IMG_H
