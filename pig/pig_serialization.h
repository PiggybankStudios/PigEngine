/*
File:   pig_serialization.h
Author: Taylor Robbins
Date:   05\25\2023
*/

#ifndef _PIG_SERIALIZATION_H
#define _PIG_SERIALIZATION_H

//NOTE: "Slz" stands for Serialization

// +--------------------------------------------------------------+
// |                        Packed Structs                        |
// +--------------------------------------------------------------+
START_PACK();
struct ATTR_PACKED SlzRec_t
{
	r32 x;
	r32 y;
	r32 width;
	r32 height;
};
struct ATTR_PACKED SlzVec2_t
{
	r32 x;
	r32 y;
};
struct ATTR_PACKED SlzVec3_t
{
	r32 x;
	r32 y;
	r32 z;
};
struct ATTR_PACKED SlzVec4_t
{
	r32 x;
	r32 y;
	r32 z;
	r32 w;
};
struct ATTR_PACKED SlzMyStr_t
{
	u64 length;
};
END_PACK();

// +--------------------------------------------------------------+
// |                         Member Types                         |
// +--------------------------------------------------------------+
enum SlzMemberType_t
{
	SlzMemberType_None = 0,
	SlzMemberType_r32,
	SlzMemberType_r64,
	SlzMemberType_u8,
	SlzMemberType_u16,
	SlzMemberType_u32,
	SlzMemberType_u64,
	SlzMemberType_i8,
	SlzMemberType_i16,
	SlzMemberType_i32,
	SlzMemberType_i64,
	SlzMemberType_rec,
	SlzMemberType_v2,
	SlzMemberType_v3,
	SlzMemberType_v4,
	SlzMemberType_MyStr_t,
	SlzMemberType_NumTypes,
};
const char* GetSlzMemberTypeStr(SlzMemberType_t enumValue)
{
	switch (enumValue)
	{
		case SlzMemberType_None:    return "None";
		case SlzMemberType_r32:     return "r32";
		case SlzMemberType_r64:     return "r64";
		case SlzMemberType_u8:      return "u8";
		case SlzMemberType_u16:     return "u16";
		case SlzMemberType_u32:     return "u32";
		case SlzMemberType_u64:     return "u64";
		case SlzMemberType_i8:      return "i8";
		case SlzMemberType_i16:     return "i16";
		case SlzMemberType_i32:     return "i32";
		case SlzMemberType_i64:     return "i64";
		case SlzMemberType_rec:     return "rec";
		case SlzMemberType_v2:      return "v2";
		case SlzMemberType_v3:      return "v3";
		case SlzMemberType_v4:      return "v4";
		case SlzMemberType_MyStr_t: return "MyStr_t";
		default: return "Unknown";
	}
}

#define SlzMemberTypeSize_r32     sizeof(r32)
#define SlzMemberTypeSize_r64     sizeof(r64)
#define SlzMemberTypeSize_u8      sizeof(u8)
#define SlzMemberTypeSize_u16     sizeof(u16)
#define SlzMemberTypeSize_u32     sizeof(u32)
#define SlzMemberTypeSize_u64     sizeof(u64)
#define SlzMemberTypeSize_i8      sizeof(i8)
#define SlzMemberTypeSize_i16     sizeof(i16)
#define SlzMemberTypeSize_i32     sizeof(i32)
#define SlzMemberTypeSize_i64     sizeof(i64)
#define SlzMemberTypeSize_rec     sizeof(SlzRec_t)
#define SlzMemberTypeSize_v2      sizeof(SlzVec2_t)
#define SlzMemberTypeSize_v3      sizeof(SlzVec3_t)
#define SlzMemberTypeSize_v4      sizeof(SlzVec4_t)
#define SlzMemberTypeSize_MyStr_t sizeof(SlzMyStr_t)

u64 GetSlzMemberTypeSize(SlzMemberType_t memberType)
{
	switch (memberType)
	{
		case SlzMemberType_None:    return 0;
		case SlzMemberType_r32:     return SlzMemberTypeSize_r32;
		case SlzMemberType_r64:     return SlzMemberTypeSize_r64;
		case SlzMemberType_u8:      return SlzMemberTypeSize_u8;
		case SlzMemberType_u16:     return SlzMemberTypeSize_u16;
		case SlzMemberType_u32:     return SlzMemberTypeSize_u32;
		case SlzMemberType_u64:     return SlzMemberTypeSize_u64;
		case SlzMemberType_i8:      return SlzMemberTypeSize_i8;
		case SlzMemberType_i16:     return SlzMemberTypeSize_i16;
		case SlzMemberType_i32:     return SlzMemberTypeSize_i32;
		case SlzMemberType_i64:     return SlzMemberTypeSize_i64;
		case SlzMemberType_rec:     return SlzMemberTypeSize_rec;
		case SlzMemberType_v2:      return SlzMemberTypeSize_v2;
		case SlzMemberType_v3:      return SlzMemberTypeSize_v3;
		case SlzMemberType_v4:      return SlzMemberTypeSize_v4;
		case SlzMemberType_MyStr_t: return SlzMemberTypeSize_MyStr_t;
		default: return 0;
	}
}

#endif //  _PIG_SERIALIZATION_H
