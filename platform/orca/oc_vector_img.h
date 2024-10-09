/*
File:   oc_vector_img.h
Author: Taylor Robbins
Date:   09\29\2024
*/

#ifndef _OC_VECTOR_IMG_H
#define _OC_VECTOR_IMG_H

struct VectorEdge_t
{
	v2 start;
	v2 control1;
	v2 control2;
	v2 end;
};

struct VectorPath_t
{
	VarArray_t edges; //VectorEdge_t
};

enum VectorFillType_t
{
	VectorFillType_None,
	VectorFillType_Solid,
	VectorFillType_NumTypes,
};
const char* GetVectorFillTypeStr(VectorFillType_t enumValue)
{
	switch (enumValue)
	{
		case VectorFillType_None:  return "None";
		case VectorFillType_Solid: return "Solid";
		default: return "Unknown";
	}
}
enum VectorStrokeType_t
{
	VectorStrokeType_None,
	VectorStrokeType_Solid,
	VectorStrokeType_NumTypes,
};
const char* GetVectorStrokeTypeStr(VectorStrokeType_t enumValue)
{
	switch (enumValue)
	{
		case VectorStrokeType_None:  return "None";
		case VectorStrokeType_Solid: return "Solid";
		default: return "Unknown";
	}
}

struct VectorShape_t
{
	MyStr_t name;
	bool visible;
	r32 opacity;
	rec bounds;
	
	VectorFillType_t fillType;
	bool fillOddSections;
	Color_t fillColor;
	VectorStrokeType_t strokeType;
	r32 strokeWidth;
	Color_t strokeColor;
	r32 miterLimit;
	
	VarArray_t paths; //VectorPath_t
};

struct VectorImg_t
{
	MemArena_t* allocArena;
	VarArray_t shapes; //VectorShape_t
};

#endif //  _OC_VECTOR_IMG_H
