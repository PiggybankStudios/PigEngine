/*
File:   pig_vox_frame_set.cpp
Author: Taylor Robbins
Date:   10\21\2024
Description: 
	** FrameSets are the common format that is produced by any voxel file format
	** deserialization process. They contain the voxel data in "frames", which are each
	** their own 3D grid of a particular size with voxels stored in the form of Color_t array
*/

void FreeVoxFrame(VoxFrame_t* frame, MemArena_t* allocArena)
{
	NotNull(frame);
	if (frame->colors != nullptr)
	{
		NotNull(allocArena);
		FreeString(allocArena, &frame->name);
		FreeMem(allocArena, frame->colors, sizeof(Color_t) * frame->size.width * frame->size.height * frame->size.depth);
	}
	ClearPointer(frame);
}
void FreeVoxFrameSet(VoxFrameSet_t* frameSet)
{
	NotNull(frameSet);
	VarArrayLoop(&frameSet->frames, fIndex)
	{
		VarArrayLoopGet(VoxFrame_t, frame, &frameSet->frames, fIndex);
		FreeVoxFrame(frame, frameSet->allocArena);
	}
	FreeVarArray(&frameSet->frames);
	ClearPointer(frameSet);
}

void InitVoxFrameSet(VoxFrameSet_t* frameSet, MemArena_t* memArena, u64 numFramesExpected = 0)
{
	NotNull2(frameSet, memArena);
	ClearPointer(frameSet);
	frameSet->allocArena = memArena;
	frameSet->triangulated = false;
	CreateVarArray(&frameSet->frames, memArena, sizeof(VoxFrame_t), numFramesExpected);
}

VoxFrame_t* AddVoxFrame(VoxFrameSet_t* frameSet, v3i size, Color_t fillColor)
{
	NotNull2(frameSet, frameSet->allocArena);
	VoxFrame_t* result = VarArrayAdd(&frameSet->frames, VoxFrame_t);
	ClearPointer(result);
	result->frameIndex = frameSet->frames.length-1;
	result->size = size;
	result->numVoxels = (u64)(size.width * size.height * size.depth);
	result->colors = AllocArray(frameSet->allocArena, Color_t, result->numVoxels);
	//TODO: Should we bubble up the OOM scenario so the deserialization code can handle when a corrupt size is read and we try and allocate millions of bytes?
	NotNull(result->colors);
	if (fillColor.r == fillColor.g && fillColor.r == fillColor.b && fillColor.r == fillColor.a)
	{
		MyMemSet(result->colors, fillColor.r, sizeof(Color_t) * result->numVoxels);
	}
	else
	{
		for (u64 cIndex = 0; cIndex < result->numVoxels; cIndex++) { result->colors[cIndex] = fillColor; }
	}
	return result;
}

//NOTE: This will not copy the triangulated vertBuffer because the data has already been passed to the GPU
void CopyVoxFrame(VoxFrame_t* destFrame, const VoxFrame_t* sourceFrame, MemArena_t* memArena)
{
	NotNull3(destFrame, sourceFrame, memArena);
	ClearPointer(destFrame);
	destFrame->name = AllocString(memArena, &sourceFrame->name);
	destFrame->position = sourceFrame->position;
	destFrame->numVoxels = sourceFrame->numVoxels;
	destFrame->size = sourceFrame->size;
	destFrame->triangulated = false;
	if (sourceFrame->colors != nullptr)
	{
		u64 numVoxels = (u64)(sourceFrame->size.width * sourceFrame->size.height * sourceFrame->size.depth);
		destFrame->colors = AllocArray(memArena, Color_t, numVoxels);
		NotNull(destFrame->colors);
		MyMemCopy(destFrame->colors, sourceFrame->colors, sizeof(Color_t) * numVoxels);
	}
}
void CopyVoxFrameSet(VoxFrameSet_t* destFrameSet, const VoxFrameSet_t* sourceFrameSet, MemArena_t* memArena)
{
	NotNull3(destFrameSet, sourceFrameSet, memArena);
	ClearPointer(destFrameSet);
	InitVoxFrameSet(destFrameSet, memArena, sourceFrameSet->frames.length);
	destFrameSet->isValid = sourceFrameSet->isValid;
	destFrameSet->error = sourceFrameSet->error;
	destFrameSet->triangulated = false;
	VarArrayLoop(&sourceFrameSet->frames, fIndex)
	{
		const VarArrayLoopGet(VoxFrame_t, sourceFrame, &sourceFrameSet->frames, fIndex);
		VoxFrame_t* destFrame = VarArrayAdd(&destFrameSet->frames, VoxFrame_t);
		NotNull(destFrame);
		CopyVoxFrame(destFrame, sourceFrame, memArena);
	}
}

// +--------------------------------------------------------------+
// |                        Get Functions                         |
// +--------------------------------------------------------------+
VoxFrame_t* FindVoxFrameByName(VoxFrameSet_t* frameSet, MyStr_t name, bool ignoreCase = true, bool prefixMatch = false)
{
	NotNull(frameSet);
	VarArrayLoop(&frameSet->frames, fIndex)
	{
		VarArrayLoopGet(VoxFrame_t, frame, &frameSet->frames, fIndex);
		if ((prefixMatch && !ignoreCase && StrStartsWith(frame->name, name)) ||
			(!prefixMatch && !ignoreCase && StrEquals(frame->name, name)) ||
			(prefixMatch && ignoreCase && frame->name.length >= name.length && StrEqualsIgnoreCase(StrSubstringLength(&frame->name, 0, name.length), name)) ||
			(!prefixMatch && ignoreCase && StrEqualsIgnoreCase(frame->name, name)))
		{
			return frame;
		}
	}
	return nullptr;
}

// +--------------------------------------------------------------+
// |                        Triangulation                         |
// +--------------------------------------------------------------+
void TriangulateVoxFrame(VoxFrameSet_t* frameSet, VoxFrame_t* frame)
{
	NotNull4(frameSet, frameSet->allocArena, frame, frame->colors);
	if (frame->triangulated)
	{
		DestroyVertBuffer(&frame->vertBuffer);
		frame->triangulated = false;
	}
	
	MemArena_t* scratch = GetScratchArena(frameSet->allocArena);
	
	//TODO: Verify this upper bounds calculation!
	// Worst case scenario, we have a checkerboard pattern in 3D space where every
	// other voxel is filled. So half of the voxels have all 6 faces exposed.
	// So we take numVoxels, multiply by number of sides on each voxel, and then
	// divide by two to get the number of faces needed for a checkerboard pattern
	// (add 6 to account for odd sized grids having one voxel more than half)
	u64 maxNumFacesNeeded = ((frame->numVoxels * Dir3_Count) / 2) + Dir3_Count;
	
	VarArray_t vertices;
	CreateVarArray(&vertices, scratch, sizeof(Vertex3D_t), maxNumFacesNeeded * BOX_PRIMITIVE_VERTICES_PER_FACE);
	VarArray_t indices;
	CreateVarArray(&indices, scratch, sizeof(i32), maxNumFacesNeeded * BOX_PRIMITIVE_INDICES_PER_FACE);
	
	PrimitiveIndexedVerts_t boxVerts = GenerateVertsForBox(Box_Unit, scratch, true);
	Assert(boxVerts.numVertices == Dir3_Count * BOX_PRIMITIVE_VERTICES_PER_FACE);
	Assert(boxVerts.numIndices == Dir3_Count * BOX_PRIMITIVE_INDICES_PER_FACE);
	
	for (i32 yOffset = 0; yOffset < frame->size.height; yOffset++)
	{
		for (i32 zOffset = 0; zOffset < frame->size.depth; zOffset++)
		{
			for (i32 xOffset = 0; xOffset < frame->size.width; xOffset++)
			{
				v3i offset = NewVec3i(xOffset, yOffset, zOffset);
				u64 voxelIndex = INDEX_FROM_COORD3D(offset.x, offset.y, offset.z, frame->size.width, frame->size.height, frame->size.depth);
				Color_t voxelColor = frame->colors[voxelIndex];
				if (voxelColor.a != 0)
				{
					v4 voxelColorVec = ToVec4(voxelColor);
					v3 voxelPos = ToVec3(offset);
					for (u8 dIndex = 0; dIndex < Dir3_Count; dIndex++)
					{
						Dir3_t sideDir = Dir3FromIndex(dIndex);
						v3i adjacentOffset = offset + ToVec3i(sideDir);
						bool needToMakeFace = true;
						if (adjacentOffset.x >= 0 && adjacentOffset.x < frame->size.width &&
							adjacentOffset.y >= 0 && adjacentOffset.y < frame->size.height &&
							adjacentOffset.z >= 0 && adjacentOffset.z < frame->size.depth)
						{
							u64 sideVoxelIndex = INDEX_FROM_COORD3D(adjacentOffset.x, adjacentOffset.y, adjacentOffset.z, frame->size.width, frame->size.height, frame->size.depth);
							needToMakeFace = (frame->colors[sideVoxelIndex].a == 0);
						}
						
						if (needToMakeFace)
						{
							u64 faceIndex = 0;
							switch (sideDir)
							{
								case Dir3_Right:    faceIndex = BOX_PRIMITIVE_RIGHT_FACE_INDEX;  break;
								case Dir3_Left:     faceIndex = BOX_PRIMITIVE_LEFT_FACE_INDEX;   break;
								case Dir3_Up:       faceIndex = BOX_PRIMITIVE_TOP_FACE_INDEX;    break;
								case Dir3_Down:     faceIndex = BOX_PRIMITIVE_BOTTOM_FACE_INDEX; break;
								case Dir3_Forward:  faceIndex = BOX_PRIMITIVE_FRONT_FACE_INDEX;  break;
								case Dir3_Backward: faceIndex = BOX_PRIMITIVE_BACK_FACE_INDEX;   break;
							}
							
							u64 firstVertIndex = (faceIndex * BOX_PRIMITIVE_VERTICES_PER_FACE);
							i32 startingIndex = (i32)vertices.length - (i32)firstVertIndex;
							Vertex3D_t* newVerts = VarArrayAddRange(&vertices, vertices.length, BOX_PRIMITIVE_VERTICES_PER_FACE, Vertex3D_t);
							NotNull(newVerts);
							for (u64 vIndex = 0; vIndex < BOX_PRIMITIVE_VERTICES_PER_FACE; vIndex++)
							{
								const PrimitiveVert3D_t* primVert = &boxVerts.vertices[firstVertIndex + vIndex];
								newVerts[vIndex].position = voxelPos + primVert->position;
								newVerts[vIndex].color = voxelColorVec;
								newVerts[vIndex].texCoord = Vec2_Zero; //TODO: Do we need texture coordinates?
								newVerts[vIndex].normal = ToVec3(sideDir);
							}
							
							i32* newIndices = VarArrayAddRange(&indices, indices.length, BOX_PRIMITIVE_INDICES_PER_FACE, i32);
							NotNull(newIndices);
							for (u64 iIndex = 0; iIndex < BOX_PRIMITIVE_INDICES_PER_FACE; iIndex++)
							{
								const PrimitiveIndex3D_t* primIndex = &boxVerts.indices[(faceIndex * BOX_PRIMITIVE_INDICES_PER_FACE) + iIndex];
								newIndices[iIndex] = startingIndex + (i32)primIndex->index;
								DebugAssert(newIndices[iIndex] >= 0);
							}
						}
					}
				}
			}
		}
	}
	DebugAssert(vertices.length <= maxNumFacesNeeded * BOX_PRIMITIVE_VERTICES_PER_FACE);
	DebugAssert(indices.length <= maxNumFacesNeeded * BOX_PRIMITIVE_INDICES_PER_FACE);
	
	bool createSuccess = CreateVertBuffer3DWithIndicesI32(
		frameSet->allocArena,
		&frame->vertBuffer,
		false, //dynamic
		vertices.length, VarArrayGet(&vertices, 0, Vertex3D_t),
		indices.length, VarArrayGet(&indices, 0, i32),
		false, false
	);
	DebugAssertAndUnused(createSuccess == true, createSuccess);
	
	FreeScratchArena(scratch);
}

void TriangulateVoxFrameSet(VoxFrameSet_t* frameSet)
{
	NotNull2(frameSet, frameSet->allocArena);
	VarArrayLoop(&frameSet->frames, fIndex)
	{
		VarArrayLoopGet(VoxFrame_t, frame, &frameSet->frames, fIndex);
		TriangulateVoxFrame(frameSet, frame);
	}
	frameSet->triangulated = true;
}