/*
File:   pig_vox_frame_set.h
Author: Taylor Robbins
Date:   10\19\2024
*/

#ifndef _PIG_VOX_FRAME_SET_H
#define _PIG_VOX_FRAME_SET_H

struct VoxFrame_t
{
	u64 frameIndex;
	bool triangulated;
	MyStr_t name;
	v3 position;
	v3i size;
	u64 numVoxels;
	Color_t* colors;
	VertBuffer_t vertBuffer;
};
struct VoxFrameSet_t
{
	MemArena_t* allocArena;
	bool isValid;
	Result_t error;
	bool triangulated;
	VarArray_t frames; //VoxFrame_t
};

#endif //  _PIG_VOX_FRAME_SET_H
