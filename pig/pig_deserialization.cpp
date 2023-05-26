/*
File:   pig_deserialization.cpp
Author: Taylor Robbins
Date:   01\10\2022
Description: 
	** Holds a bunch of helper functions for use when deserializing files of various formats
*/

// +--------------------------------------------------------------+
// |                Binary Deserialization Helpers                |
// +--------------------------------------------------------------+
const void* BinDeser_ReadStruct_(const void* dataPntr, u64 dataSize, u64* byteIndexPntr, u64 structureSize)
{
	NotNull2(dataPntr, byteIndexPntr);
	if ((*byteIndexPntr) + structureSize > dataSize) { return nullptr; }
	const void* result = ((u8*)dataPntr) + (*byteIndexPntr);
	*byteIndexPntr = (*byteIndexPntr) + structureSize;
	return result;
}
#define BinDeser_ReadStruct(dataPntr, dataSize, byteIndexPntr, type) (type*)BinDeser_ReadStruct_((dataPntr), (dataSize), (byteIndexPntr), sizeof(type))

const void* BinDeser_ReadStructDynamicSize_(const void* dataPntr, u64 dataSize, u64* byteIndexPntr, u64* structSizeOut, u64 sizeMemberOffset, u64 minStructSize, u64 maxStructSize)
{
	NotNull2(dataPntr, byteIndexPntr);
	Assert (sizeMemberOffset + sizeof(u64) <= minStructSize);
	if ((*byteIndexPntr) + minStructSize > dataSize) { return nullptr; }
	const u64* sizePntr = (const u64*)(((u8*)dataPntr) + (*byteIndexPntr) + sizeMemberOffset);
	if (*sizePntr < minStructSize) { return nullptr; }
	if (*sizePntr > maxStructSize) { return nullptr; }
	if (structSizeOut != nullptr) { *structSizeOut = *sizePntr; }
	const void* result = ((u8*)dataPntr) + (*byteIndexPntr);
	*byteIndexPntr = (*byteIndexPntr) + (*sizePntr);
	return result;
}
#define BinDeser_ReadStructDynamicSize(dataPntr, dataSize, byteIndexPntr, structSizeOut, type, sizeMemberName, firstOptionalMemberName) (type*)BinDeser_ReadStructDynamicSize_((dataPntr), (dataSize), (byteIndexPntr), (structSizeOut), STRUCT_VAR_OFFSET(type, sizeMemberName), STRUCT_VAR_OFFSET(type, firstOptionalMemberName), sizeof(type))

#define BinDeser_IsMemberPresent(structSize, type, memberName) (structSize >= (STRUCT_VAR_OFFSET(type, memberName) + sizeof(((type*)0)->memberName)))

bool BinDeser_ReadVariable_(const void* dataPntr, u64 dataSize, u64* byteIndexPntr, u64 valueSize, void* valueOutPntr)
{
	NotNull3(dataPntr, byteIndexPntr, valueOutPntr);
	if ((*byteIndexPntr) + valueSize > dataSize) { return false; }
	const void* valueReadPntr = ((u8*)dataPntr) + (*byteIndexPntr);
	MyMemCopy(valueOutPntr, valueReadPntr, valueSize);
	*byteIndexPntr = (*byteIndexPntr) + valueSize;
	return true;
}
#define BinDeser_ReadU8(dataPntr, dataSize, byteIndexPntr, variableOut)  BinDeser_ReadVariable_((dataPntr), (dataSize), (byteIndexPntr), sizeof(u8),  (void*)(variableOut))
#define BinDeser_ReadU16(dataPntr, dataSize, byteIndexPntr, variableOut) BinDeser_ReadVariable_((dataPntr), (dataSize), (byteIndexPntr), sizeof(u16), (void*)(variableOut))
#define BinDeser_ReadU32(dataPntr, dataSize, byteIndexPntr, variableOut) BinDeser_ReadVariable_((dataPntr), (dataSize), (byteIndexPntr), sizeof(u32), (void*)(variableOut))
#define BinDeser_ReadU64(dataPntr, dataSize, byteIndexPntr, variableOut) BinDeser_ReadVariable_((dataPntr), (dataSize), (byteIndexPntr), sizeof(u64), (void*)(variableOut))
#define BinDeser_ReadI8(dataPntr, dataSize, byteIndexPntr, variableOut)  BinDeser_ReadVariable_((dataPntr), (dataSize), (byteIndexPntr), sizeof(i8),  (void*)(variableOut))
#define BinDeser_ReadI16(dataPntr, dataSize, byteIndexPntr, variableOut) BinDeser_ReadVariable_((dataPntr), (dataSize), (byteIndexPntr), sizeof(i16), (void*)(variableOut))
#define BinDeser_ReadI32(dataPntr, dataSize, byteIndexPntr, variableOut) BinDeser_ReadVariable_((dataPntr), (dataSize), (byteIndexPntr), sizeof(i32), (void*)(variableOut))
#define BinDeser_ReadI64(dataPntr, dataSize, byteIndexPntr, variableOut) BinDeser_ReadVariable_((dataPntr), (dataSize), (byteIndexPntr), sizeof(i64), (void*)(variableOut))
