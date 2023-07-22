/*
File:   pig_serialization.cpp
Author: Taylor Robbins
Date:   08\13\2022
Description: 
	** Holds functions that help us serialize data in various ways
	** Often these are helper functions to facilitate a 2-pass approach to serialization (one pass to size, allocate, then another pass to fill)
*/

// +--------------------------------------------------------------+
// |                 Binary Serialization Headers                 |
// +--------------------------------------------------------------+
void* BinSer_WriteStructure_(void* result, u64 resultSize, u64* byteIndexPntr, u64 structureSize)
{
	NotNull(byteIndexPntr);
	AssertIf(result != nullptr, (*byteIndexPntr) + structureSize <= resultSize);
	void* structPntr = (result != nullptr) ? (((u8*)result) + (*byteIndexPntr)) : nullptr;
	*byteIndexPntr = (*byteIndexPntr) + structureSize;
	return structPntr;
}
#define BinSer_WriteStructure(result, resultSize, byteIndexPntr, type) (type*)BinSer_WriteStructure_((result), (resultSize), (byteIndexPntr), sizeof(type))

void BinSer_WriteValue_(void* result, u64 resultSize, u64* byteIndexPntr, u64 valueSize, const void* valuePntr)
{
	NotNull2(byteIndexPntr, valuePntr);
	if (result != nullptr)
	{
		Assert((*byteIndexPntr) + valueSize <= resultSize);
		void* valueWritePntr = ((u8*)result) + (*byteIndexPntr);
		MyMemCopy(valueWritePntr, valuePntr, valueSize);
	}
	*byteIndexPntr = (*byteIndexPntr) + valueSize;
}
#define BinSer_WriteValue(result, resultSize, byteIndexPntr, type, value) do                           \
{                                                                                                      \
	type localVariableForValue = (value);                                                              \
	BinSer_WriteValue_((result), (resultSize), (byteIndexPntr), sizeof(type), &localVariableForValue); \
} while(0)

#define BinSer_WriteU8(result, resultSize, byteIndexPntr, value)  BinSer_WriteValue((result), (resultSize), (byteIndexPntr), u8,  (value))
#define BinSer_WriteU16(result, resultSize, byteIndexPntr, value) BinSer_WriteValue((result), (resultSize), (byteIndexPntr), u16, (value))
#define BinSer_WriteU32(result, resultSize, byteIndexPntr, value) BinSer_WriteValue((result), (resultSize), (byteIndexPntr), u32, (value))
#define BinSer_WriteU64(result, resultSize, byteIndexPntr, value) BinSer_WriteValue((result), (resultSize), (byteIndexPntr), u64, (value))
#define BinSer_WriteI8(result, resultSize, byteIndexPntr, value)  BinSer_WriteValue((result), (resultSize), (byteIndexPntr), i8,  (value))
#define BinSer_WriteI16(result, resultSize, byteIndexPntr, value) BinSer_WriteValue((result), (resultSize), (byteIndexPntr), i16, (value))
#define BinSer_WriteI32(result, resultSize, byteIndexPntr, value) BinSer_WriteValue((result), (resultSize), (byteIndexPntr), i32, (value))
#define BinSer_WriteI64(result, resultSize, byteIndexPntr, value) BinSer_WriteValue((result), (resultSize), (byteIndexPntr), i64, (value))
