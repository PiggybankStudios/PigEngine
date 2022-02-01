/*
File:   pig_svg.h
Author: Taylor Robbins
Date:   01\23\2022
*/

#ifndef _PIG_SVG_H
#define _PIG_SVG_H

#define SVG_TRIANGULATION_CURVE_VERT_COUNT    9 //verts/curve

enum TryDeserSvgFileError_t
{
	TryDeserSvgFileError_None = 0,
	TryDeserSvgFileError_EmptyFile,
	TryDeserSvgFileError_TryParseError,
	TryDeserSvgFileError_XmlParsingError,
	TryDeserSvgFileError_GroupTransformIsInvalid,
	TryDeserSvgFileError_GroupOutsideSvg,
	TryDeserSvgFileError_ShapeOutsideGroup,
	TryDeserSvgFileError_MissingProperty,
	TryDeserSvgFileError_StyleFormatError,
	TryDeserSvgFileError_PathFormatError,
	TryDeserSvgFileError_NumErrors,
};
const char* GetTryDeserSvgFileErrorStr(TryDeserSvgFileError_t error)
{
	switch (error)
	{
		case TryDeserSvgFileError_EmptyFile:               return "EmptyFile";
		case TryDeserSvgFileError_TryParseError:           return "TryParseError";
		case TryDeserSvgFileError_XmlParsingError:         return "XmlParsingError";
		case TryDeserSvgFileError_GroupTransformIsInvalid: return "GroupTransformIsInvalid";
		case TryDeserSvgFileError_GroupOutsideSvg:         return "GroupOutsideSvg";
		case TryDeserSvgFileError_ShapeOutsideGroup:       return "ShapeOutsideGroup";
		case TryDeserSvgFileError_MissingProperty:         return "MissingProperty";
		case TryDeserSvgFileError_StyleFormatError:        return "StyleFormatError";
		case TryDeserSvgFileError_PathFormatError:         return "PathFormatError";
		default: return "Unknown";
	}
}

enum SvgTransformType_t
{
	SvgTransformType_None,
	SvgTransformType_Scale,
	SvgTransformType_Rotate,
	SvgTransformType_Translate,
	SvgTransformType_Matrix,
	SvgTransformType_NumTypes,
};
struct SvgTransform_t
{
	SvgTransformType_t type;
	union
	{
		r32 rotation;
		r32 scale;
		v2 vector;
		mat4 matrix;
	};
};

enum SvgFillStyle_t
{
	SvgFillStyle_None = 0,
	SvgFillStyle_Solid,
	SvgFillStyle_NumStyles,
};
const char* GetSvgFillStyleStr(SvgFillStyle_t fillStyle)
{
	switch (fillStyle)
	{
		case SvgFillStyle_None:  return "None";
		case SvgFillStyle_Solid: return "Solid";
		default: return "Unknown";
	}
}

struct SvgFill_t
{
	SvgFillStyle_t style;
	Color_t color;
};

enum SvgStrokeStyle_t
{
	SvgStrokeStyle_None = 0,
	SvgStrokeStyle_Solid,
	SvgStrokeStyle_NumStyles,
};
const char* GetSvgStrokeStyleStr(SvgStrokeStyle_t strokeStyle)
{
	switch (strokeStyle)
	{
		case SvgStrokeStyle_None:  return "None";
		case SvgStrokeStyle_Solid: return "Solid";
		default: return "Unknown";
	}
}

struct SvgStroke_t
{
	SvgStrokeStyle_t style;
	Color_t color;
	r32 thickness;
};

enum SvgShapeType_t
{
	SvgShapeType_None,
	SvgShapeType_Circle,
	SvgShapeType_Rectangle,
	SvgShapeType_Path,
	SvgShapeType_NumTypes,
};
const char* GetSvgShapeTypeStr(SvgShapeType_t shapeType)
{
	switch (shapeType)
	{
		case SvgShapeType_None:      return "None";
		case SvgShapeType_Circle:    return "Circle";
		case SvgShapeType_Rectangle: return "Rectangle";
		case SvgShapeType_Path:      return "Path";
		default: return "Unknown";
	}
}

struct SvgShape_t
{
	SvgShapeType_t type;
	MyStr_t idStr;
	MyStr_t label;
	SvgTransform_t transform;
	SvgFill_t fill;
	SvgStroke_t stroke;
	union
	{
		struct
		{
			v2 center;
			r32 radius;
		} circle;
		struct
		{
			union
			{
				obb2 value;
				struct { v2 center, size; r32 rotation; };
			};
			v2 roundedRadius;
		} rectangle;
		struct
		{
			BezierPath_t value;
			bool vertBufferUpToDate;
			bool vertBufferGenFailed;
			VertBuffer_t vertBuffer;
		} path;
	};
};

struct SvgGroup_t
{
	u64 parentIndex; //UINT64_MAX = no parents
	u64 tokenParseIndex;
	u64 interleaveIndex;
	MyStr_t idStr;
	MyStr_t label;
	SvgTransform_t transform;
	v2 origin;
	VarArray_t shapes; //SvgShape_t
};

struct SvgData_t
{
	MemArena_t* allocArena;
	VarArray_t groups; //SvgGroup_t
};

#endif //  _PIG_SVG_H
