/*
File:   pig_serialization.h
Author: Taylor Robbins
Date:   05\25\2023
*/

#ifndef _PIG_SERIALIZATION_H
#define _PIG_SERIALIZATION_H

enum SerializableStructMemberType_t
{
	SerializableStructMemberType_None = 0,
	SerializableStructMemberType_r32,
	SerializableStructMemberType_r64,
	SerializableStructMemberType_u8,
	SerializableStructMemberType_u16,
	SerializableStructMemberType_u32,
	SerializableStructMemberType_u64,
	SerializableStructMemberType_i8,
	SerializableStructMemberType_i16,
	SerializableStructMemberType_i32,
	SerializableStructMemberType_i64,
	SerializableStructMemberType_rec,
	SerializableStructMemberType_v3,
	SerializableStructMemberType_MyStr,
	SerializableStructMemberType_NumTypes,
};
const char* GetSerializableStructMemberTypeStr(SerializableStructMemberType_t enumValue)
{
	switch (enumValue)
	{
		case SerializableStructMemberType_None:  return "None";
		case SerializableStructMemberType_r32:   return "r32";
		case SerializableStructMemberType_r64:   return "r64";
		case SerializableStructMemberType_u8:    return "u8";
		case SerializableStructMemberType_u16:   return "u16";
		case SerializableStructMemberType_u32:   return "u32";
		case SerializableStructMemberType_u64:   return "u64";
		case SerializableStructMemberType_i8:    return "i8";
		case SerializableStructMemberType_i16:   return "i16";
		case SerializableStructMemberType_i32:   return "i32";
		case SerializableStructMemberType_i64:   return "i64";
		case SerializableStructMemberType_rec:   return "rec";
		case SerializableStructMemberType_v3:    return "v3";
		case SerializableStructMemberType_MyStr: return "MyStr";
		default: return "Unknown";
	}
}

#endif //  _PIG_SERIALIZATION_H
