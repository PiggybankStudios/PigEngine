/*
File:   pig_vox_file.cpp
Author: Taylor Robbins
Date:   10\19\2024
Description: 
	** Holds the functions that deserialize the .vox file format the Magica Voxel uses for saving
	** This produces a VoxFrameSet_t that can then be converted to VoxModel_t
*/

// https://paulbourke.net/dataformats/vox/
//TODO: Parse more of the proprietary chunks documented in https://github.com/ephtracy/voxel-model/blob/master/MagicaVoxel-file-format-vox-extension.txt

#define VOX_FILE_MAGIC_STR            "VOX " //0x56 0x4F 0x58 0x20
#define VOX_FILE_MAGIC_LENGTH         4 //bytes
#define VOX_FILE_CHUNK_NAME_LENGTH    4 //bytes
#define VOX_FILE_PALETTE_SIZE         256 //colors
#define VOX_FILE_MAX_HIERARCHY_DEPTH  10 //parents

enum VoxFileChunkName_t
{
	VoxFileChunkName_None = 0,
	VoxFileChunkName_MAIN, //first chunk, no data, all other chunks are children
	VoxFileChunkName_PACK, //data is 1 integer, number of models in the file
	VoxFileChunkName_SIZE, //dimensions of following XYZI chunk, data is 3 integers
	VoxFileChunkName_XYZI, //actual voxel data, first voxel count, then 4*N bytes of voxel data
	VoxFileChunkName_RGBA, //color palette, 256 entries or rgba 32-bit colors, entry 0 is unused
	VoxFileChunkName_nTRN, //Transform
	VoxFileChunkName_nGRP, //Group
	VoxFileChunkName_nSHP, //Shape
	// VoxFileChunkName_LAYR, //Layer
	// VoxFileChunkName_MATL, //Material
	// VoxFileChunkName_rCAM, //Render Camera
	// VoxFileChunkName_rOBJ, //Render Object
	// VoxFileChunkName_NOTE, //Palette Note (color names)
	// VoxFileChunkName_IMAP, //Index MAP Chunk
	VoxFileChunkName_NumNames,
};
const char* GetVoxFileChunkNameStr(VoxFileChunkName_t enumValue)
{
	switch (enumValue)
	{
		case VoxFileChunkName_None: return "None";
		case VoxFileChunkName_MAIN: return "MAIN";
		case VoxFileChunkName_PACK: return "PACK";
		case VoxFileChunkName_SIZE: return "SIZE";
		case VoxFileChunkName_XYZI: return "XYZI";
		case VoxFileChunkName_RGBA: return "RGBA";
		case VoxFileChunkName_nTRN: return "nTRN";
		case VoxFileChunkName_nGRP: return "nGRP";
		case VoxFileChunkName_nSHP: return "nSHP";
		default: return "Unknown";
	}
}

START_PACK()

struct ATTR_PACKED VoxFileHeader_t
{
	char magicStr[VOX_FILE_MAGIC_LENGTH]; //Should be VOX_FILE_MAGIC_STR
	u32 versionNumber; //usually 150 or 200
};

struct ATTR_PACKED VoxFileChunkHeader_t
{
	char name[VOX_FILE_CHUNK_NAME_LENGTH];
	u32 dataSize;
	u32 childrenSize;
};

struct ATTR_PACKED VoxFileEncodedVoxel_t
{
	u8 x;
	u8 y;
	u8 z;
	u8 color; //palette index
};

END_PACK()

#define VOX_SCENE_NODE_MAX_CHILDREN   4 //nodes
#define VOX_SCENE_NODE_MAX_FRAMES     8 //frames

struct VoxSceneNodeFrame_t
{
	i32 frameIndex;
	VoxelRotation_t rotation;
	v3 translation;
};

struct VoxSceneNode_t
{
	i32 parentId;
	i32 id;
	i32 layerId;
	u64 numChildren;
	i32 childrenIds[VOX_SCENE_NODE_MAX_CHILDREN];
	MyStr_t name;
	bool isShape;
	bool hidden;
	u64 numFrames;
	VoxSceneNodeFrame_t frames[VOX_SCENE_NODE_MAX_FRAMES];
};

VoxSceneNode_t* FindVoxSceneNodeById(VarArray_t* nodesArray, i32 nodeId)
{
	NotNull(nodesArray);
	VarArrayLoop(nodesArray, nIndex)
	{
		VarArrayLoopGet(VoxSceneNode_t, sceneNode, nodesArray, nIndex);
		if (sceneNode->id == nodeId) { return sceneNode; }
	}
	return nullptr;
}

Result_t TryDeserVoxDictionary(VarArray_t* arrayOut, const void* dataPntr, u64 dataSize, u64* readIndexPntr)
{
	NotNull2(arrayOut, arrayOut->allocArena);
	Assert(arrayOut->itemSize == sizeof(MyStrPair_t));
	
	i32 numDictElements = 0;
	if (!BinDeser_ReadI32(dataPntr, dataSize, readIndexPntr, &numDictElements)) { return Result_EndOfFile; }
	if (numDictElements < 0) { return Result_InvalidSize; }
	
	VarArrayExpand(arrayOut, arrayOut->length + (u64)numDictElements);
	for (u64 eIndex = 0; eIndex < (u64)numDictElements; eIndex++)
	{
		i32 keyLength = 0;
		if (!BinDeser_ReadI32(dataPntr, dataSize, readIndexPntr, &keyLength)) { return Result_EndOfFile; }
		if (keyLength < 0) { return Result_InvalidSize; }
		const char* keyChars = (const char*)BinDeser_ReadBytes(dataPntr, dataSize, readIndexPntr, (u64)keyLength);
		if (keyChars == nullptr) { return Result_EndOfFile; }
		i32 valueLength = 0;
		if (!BinDeser_ReadI32(dataPntr, dataSize, readIndexPntr, &valueLength)) { return Result_EndOfFile; }
		if (valueLength < 0) { return Result_InvalidSize; }
		const char* valueChars = (const char*)BinDeser_ReadBytes(dataPntr, dataSize, readIndexPntr, (u64)valueLength);
		if (valueChars == nullptr) { return Result_EndOfFile; }
		
		MyStrPair_t* newEntry = VarArrayAdd(arrayOut, MyStrPair_t);
		NotNull(newEntry);
		newEntry->key = NewStr((u64)keyLength, keyChars);
		newEntry->value = NewStr((u64)valueLength, valueChars);
	}
	
	return Result_Success;
}

MyStr_t FindVoxDictionaryValue(const VarArray_t* dictionary, MyStr_t key, bool caseSensitive = true)
{
	NotNull2(dictionary, dictionary->allocArena);
	NotNullStr(&key);
	VarArrayLoop(dictionary, eIndex)
	{
		const VarArrayLoopGet(MyStrPair_t, entry, dictionary, eIndex);
		if ((caseSensitive && StrEquals(entry->key, key)) ||
			(!caseSensitive && StrEqualsIgnoreCase(entry->key, key)))
		{
			return entry->value;
		}
	}
	return MyStr_Empty;
}

VoxelRotation_t VoxelRotationFromVoxFileRotI8(i8 voxFileRotI8)
{
	//TODO: Implement me!
	return VoxelRotation_Default;
}

bool TryDeserVoxFile(MyStr_t voxFileContents, ProcessLog_t* log, VoxFrameSet_t* frameSetOut, MemArena_t* memArena)
{
	NotNull3(log, frameSetOut, memArena);
	NotNullStr(&voxFileContents);
	SetProcessLogName(log, NewStr("TryDeserVoxFile"));
	bool readSuccess = false;
	TryParseFailureReason_t parseFailureReason = TryParseFailureReason_None;
	
	if (voxFileContents.length < sizeof(VoxFileHeader_t))
	{
		LogPrintLine_E(log, "File is %llu bytes which is not large enough for the %u byte VoxFileHeader_t", voxFileContents.length, sizeof(VoxFileHeader_t));
		LogExitFailure(log, voxFileContents.length == 0 ? Result_EmptyFile : Result_MissingFilePrefix);
		return false;
	}
	if (voxFileContents.length < sizeof(VoxFileHeader_t) + sizeof(VoxFileChunkHeader_t))
	{
		LogPrintLine_E(log, "File is %llu bytes which is not large enough for the %u byte VoxFileHeader_t and the first %u byte chunk", voxFileContents.length, sizeof(VoxFileHeader_t), sizeof(VoxFileChunkHeader_t));
		LogExitFailure(log, Result_TooSmall);
		return false;
	}
	
	const VoxFileHeader_t* fileHeader = (const VoxFileHeader_t*)voxFileContents.bytes;
	if (MyMemCompare(&fileHeader->magicStr[0], VOX_FILE_MAGIC_STR, VOX_FILE_MAGIC_LENGTH) != 0)
	{
		LogPrintLine_E(log, "File does not contain correct %u byte magic word at beginning! Should be \"VOX \"", VOX_FILE_MAGIC_LENGTH);
		LogExitFailure(log, Result_InvalidFilePrefix);
		return false;
	}
	if (fileHeader->versionNumber != 150 && fileHeader->versionNumber != 200)
	{
		LogPrintLine_W(log, "Vox file is version %u which is not a version we know. Deser was implemented expecting version 150 or 200!", fileHeader->versionNumber);
		log->hadWarnings = true;
	}
	
	MemArena_t* scratch = GetScratchArena(memArena);
	VarArray_t dictElements;
	CreateVarArray(&dictElements, scratch, sizeof(MyStrPair_t), 16);
	VarArray_t sceneNodes;
	CreateVarArray(&sceneNodes, scratch, sizeof(VoxSceneNode_t), 32);
	
	bool foundMainHeader = false;
	bool foundPackHeader = false;
	u64 numModelsInFile = 0;
	bool foundRgbaHeader = false;
	Color_t* paletteColors = nullptr;
	bool foundSizeHeader = false;
	v3i nextFrameSize = Vec3i_Zero;
	
	InitVoxFrameSet(frameSetOut, memArena);
	
	u64 numParentChunks = 0;
	u64 parentChunkChildrenStartIndex[VOX_FILE_MAX_HIERARCHY_DEPTH];
	const VoxFileChunkHeader_t* parentChunkHeaders[VOX_FILE_MAX_HIERARCHY_DEPTH];
	ClearArray(parentChunkHeaders);
	
	u64 readByteIndex = sizeof(VoxFileHeader_t);
	while (readByteIndex + sizeof(VoxFileChunkHeader_t) <= voxFileContents.length)
	{
		//Pop parent chunks that we have read past the end index of their children area
		while (numParentChunks > 0)
		{
			const VoxFileChunkHeader_t* parentHeader = parentChunkHeaders[numParentChunks-1];
			u64 parentChildrenStartIndex = parentChunkChildrenStartIndex[numParentChunks-1];
			u64 childrenEndIndex = parentChildrenStartIndex + parentHeader->childrenSize;
			if (readByteIndex >= childrenEndIndex) { parentChunkHeaders[numParentChunks-1] = nullptr; numParentChunks--; }
			else { break; }
		}
		
		//read the next chunk
		u64 chunkHeaderByteIndex = readByteIndex;
		const VoxFileChunkHeader_t* chunkHeader = (VoxFileChunkHeader_t*)&voxFileContents.bytes[readByteIndex];
		readByteIndex += sizeof(VoxFileChunkHeader_t);
		const u8* chunkDataPntr = &voxFileContents.bytes[readByteIndex]; readByteIndex += chunkHeader->dataSize;
		u64 chunkReadIndex = 0;
		
		bool skipChildren = false;
		
		// +==============================+
		// |          MAIN Chunk          |
		// +==============================+
		if (MyMemCompare(&chunkHeader->name[0], GetVoxFileChunkNameStr(VoxFileChunkName_MAIN), VOX_FILE_CHUNK_NAME_LENGTH) == 0)
		{
			LogPrintLine_D(log, "Found MAIN chunk: %u byte%s", chunkHeader->childrenSize, Plural(chunkHeader->childrenSize, "s"));
			if (foundMainHeader)
			{
				LogPrintLine_E(log, "Found second MAIN chunk at offset 0x%08X", chunkHeaderByteIndex);
				LogExitFailure(log, Result_DuplicateItem);
				break;
			}
			if (chunkHeader->dataSize > 0) { LogPrintLine_W(log, "MAIN chunk has %llu bytes of data? It should be empty", chunkHeader->dataSize); }
			if (readByteIndex + chunkHeader->childrenSize != voxFileContents.length)
			{
				LogPrintLine_E(log, "MAIN chunk childrenSize does not span rest of file! It reaches 0x%08X not 0x%08X", readByteIndex + chunkHeader->childrenSize, voxFileContents.length);
				LogExitFailure(log, Result_MissingOrCorruptChildren);
				break;
			}
			foundMainHeader = true;
		}
		else if (!foundMainHeader)
		{
			LogPrintLine_E(log, "Found %.*s chunk before MAIN chunk!", VOX_FILE_CHUNK_NAME_LENGTH, &chunkHeader->name[0]);
			LogExitFailure(log, Result_WrongFirstItem);
			break;
		}
		// // +==============================+
		// // |          PACK Chunk          |
		// // +==============================+
		// else if (MyMemCompare(&chunkHeader->name[0], GetVoxFileChunkNameStr(VoxFileChunkName_PACK), VOX_FILE_CHUNK_NAME_LENGTH) == 0)
		// {
		// 	LogPrintLine_D(log, "Found PACK chunk: %u byte%s (children: %u byte%s)", chunkHeader->dataSize, Plural(chunkHeader->dataSize, "s"), chunkHeader->childrenSize, Plural(chunkHeader->childrenSize, "s"));
		// 	Unimplemented(); //TODO: Implement me!
		// }
		// +==============================+
		// |          SIZE Chunk          |
		// +==============================+
		else if (MyMemCompare(&chunkHeader->name[0], GetVoxFileChunkNameStr(VoxFileChunkName_SIZE), VOX_FILE_CHUNK_NAME_LENGTH) == 0)
		{
			LogPrintLine_D(log, "Found SIZE chunk: %u byte%s (children: %u byte%s)", chunkHeader->dataSize, Plural(chunkHeader->dataSize, "s"), chunkHeader->childrenSize, Plural(chunkHeader->childrenSize, "s"));
			if (foundSizeHeader)
			{
				LogPrintLine_E(log, "Found two SIZE chunks in a row with no XYZI chunk in between! Second chunk at 0x%08X", chunkHeaderByteIndex);
				LogExitFailure(log, Result_DuplicateItem);
				break;
			}
			if (chunkHeader->dataSize != sizeof(u32) * 3)
			{
				LogPrintLine_E(log, "SIZE chunk is %llu bytes when it should be %llu", chunkHeader->dataSize, sizeof(u32) * 3);
				LogExitFailure(log, Result_InvalidSize);
				break;
			}
			const u32* sizeU32Pntr = (const u32*)chunkDataPntr;
			//NOTE: We are intentionally swapping Y and Z axis here because we use Y as our up direction, while MagicaVoxel uses Z as up
			nextFrameSize = NewVec3i((i32)sizeU32Pntr[0], (i32)sizeU32Pntr[2], (i32)sizeU32Pntr[1]);
			u64 numVoxels = (u64)(nextFrameSize.width * nextFrameSize.height * nextFrameSize.depth);
			LogPrintLine_I(log, "Size: %dx%dx%d (%llu voxel%s)", nextFrameSize.width, nextFrameSize.height, nextFrameSize.depth, numVoxels, Plural(numVoxels, "s"));
			if (numVoxels * sizeof(VoxFileEncodedVoxel_t) > voxFileContents.length - readByteIndex)
			{
				LogPrintLine_E(log, "SIZE chunk says next frame has %llu voxels which is too many to fit in the file!", numVoxels);
				LogExitFailure(log, Result_CorruptValue);
				break;
			}
			if (nextFrameSize.width == 0 || nextFrameSize.height == 0 || nextFrameSize.depth == 0)
			{
				LogWriteLine_E(log, "SIZE chunk 0 for one of the vector components!");
				LogExitFailure(log, Result_InvalidSize);
				break;
			}
			
			foundSizeHeader = true;
		}
		// +==============================+
		// |          RGBA Chunk          |
		// +==============================+
		else if (MyMemCompare(&chunkHeader->name[0], GetVoxFileChunkNameStr(VoxFileChunkName_RGBA), VOX_FILE_CHUNK_NAME_LENGTH) == 0)
		{
			LogPrintLine_D(log, "Found RGBA chunk: %u byte%s (children: %u byte%s)", chunkHeader->dataSize, Plural(chunkHeader->dataSize, "s"), chunkHeader->childrenSize, Plural(chunkHeader->childrenSize, "s"));
			if (foundRgbaHeader)
			{
				LogPrintLine_E(log, "Found two RGBA chunks! Second chunk at 0x%08X", chunkHeaderByteIndex);
				LogExitFailure(log, Result_DuplicateItem);
				break;
			}
			if (chunkHeader->dataSize != 4*sizeof(u8) * VOX_FILE_PALETTE_SIZE)
			{
				LogPrintLine_E(log, "RGBA chunk is %llu bytes when it should be %llu", chunkHeader->dataSize, 4*sizeof(u8) * VOX_FILE_PALETTE_SIZE);
				LogExitFailure(log, Result_InvalidSize);
				break;
			}
			paletteColors = AllocArray(scratch, Color_t, VOX_FILE_PALETTE_SIZE);
			NotNull(paletteColors);
			for (u64 pIndex = 0; pIndex < VOX_FILE_PALETTE_SIZE; pIndex++)
			{
				const u8* paletteColorPntr = &chunkDataPntr[pIndex * 4*sizeof(u8)];
				paletteColors[pIndex] = NewColor(
					paletteColorPntr[0],
					paletteColorPntr[1],
					paletteColorPntr[2],
					paletteColorPntr[3]
				);
			}
			
			//If the palette comes after the size/data, we had stored the palette
			// color index in the alpha channel of each voxel, let's convert those
			// to actual colors now
			VarArrayLoop(&frameSetOut->frames, fIndex)
			{
				VarArrayLoopGet(VoxFrame_t, frame, &frameSetOut->frames, fIndex);
				for (u64 vIndex = 0; vIndex < frame->numVoxels; vIndex++)
				{
					u8 colorIndex = frame->colors[vIndex].a;
					frame->colors[vIndex] = (colorIndex != 0xFF ? paletteColors[colorIndex] : Transparent);
				}
			}
			
			foundRgbaHeader = true;
		}
		// +==============================+
		// |    nTRN/nGRP/nSHP Chunks     |
		// +==============================+
		else if (
			MyMemCompare(&chunkHeader->name[0], GetVoxFileChunkNameStr(VoxFileChunkName_nTRN), VOX_FILE_CHUNK_NAME_LENGTH) == 0 ||
			MyMemCompare(&chunkHeader->name[0], GetVoxFileChunkNameStr(VoxFileChunkName_nGRP), VOX_FILE_CHUNK_NAME_LENGTH) == 0 ||
			MyMemCompare(&chunkHeader->name[0], GetVoxFileChunkNameStr(VoxFileChunkName_nSHP), VOX_FILE_CHUNK_NAME_LENGTH) == 0)
		{
			MyStr_t chunkType = NewStr(VOX_FILE_CHUNK_NAME_LENGTH, &chunkHeader->name[0]);
			bool isTranslationNode = StrEquals(chunkType, GetVoxFileChunkNameStr(VoxFileChunkName_nTRN));
			bool isGroupNode = StrEquals(chunkType, GetVoxFileChunkNameStr(VoxFileChunkName_nGRP));
			bool isShapeNode = StrEquals(chunkType, GetVoxFileChunkNameStr(VoxFileChunkName_nSHP));
			// LogPrintLine_D(log, "Found %.*s chunk: %u byte%s (children: %u byte%s)", StrPrint(chunkType), chunkHeader->dataSize, Plural(chunkHeader->dataSize, "s"), chunkHeader->childrenSize, Plural(chunkHeader->childrenSize, "s"));
			i32 nodeId = 0; readSuccess = BinDeser_ReadI32(chunkDataPntr, chunkHeader->dataSize, &chunkReadIndex, &nodeId); Assert(readSuccess); //TODO: Turn this into an error!
			// LogPrintLine_D(log, "nodeId = %d", nodeId);
			
			VoxSceneNode_t* newNode = VarArrayAdd(&sceneNodes, VoxSceneNode_t);
			NotNull(newNode);
			ClearPointer(newNode);
			newNode->id = nodeId;
			newNode->isShape = isShapeNode;
			
			// Find parentId
			// TODO: Technically if you make a reference model in Magica Voxel,
			//  a single nSHP node might have more than one nTRN node that
			//  references it as a child
			newNode->parentId = -1;
			VarArrayLoop(&sceneNodes, nIndex)
			{
				VarArrayLoopGet(VoxSceneNode_t, otherNode, &sceneNodes, nIndex);
				if (otherNode->id != nodeId && otherNode->numChildren > 0)
				{
					for (u64 cIndex = 0; cIndex < otherNode->numChildren; cIndex++)
					{
						if (otherNode->childrenIds[cIndex] == newNode->id) { newNode->parentId = otherNode->id; break; }
					}
					if (newNode->parentId != -1) { break; }
				}
			}
			// LogPrintLine_D(log, "parentId = %d", newNode->parentId);
			
			// Parse node attributes
			Result_t dictReadError = TryDeserVoxDictionary(&dictElements, chunkDataPntr, chunkHeader->dataSize, &chunkReadIndex);
			if (dictReadError != Result_Success)
			{
				LogPrintLine_E(log, "Failed to read node attributes on %.*s chunk at 0x%08X: %s", StrPrint(chunkType), chunkHeaderByteIndex, GetResultStr(dictReadError));
				LogExitFailure(log, dictReadError);
				break;
			}
			if (isTranslationNode)
			{
				//TODO: Do these two attributes ever actually show up on the transform node??
				MyStr_t nameStr = FindVoxDictionaryValue(&dictElements, NewStr("_name"), false);
				if (!IsEmptyStr(nameStr))
				{
					// LogPrintLine_D(log, "Found name: \"%.*s\"", StrPrint(nameStr));
					newNode->name = nameStr;
				}
				MyStr_t hiddenStr = FindVoxDictionaryValue(&dictElements, NewStr("_hidden"), false);
				if (!IsEmptyStr(hiddenStr))
				{
					// LogPrintLine_D(log, "Found hidden: \"%.*s\"", StrPrint(hiddenStr));
					if (!TryParseBool(hiddenStr, &newNode->hidden, &parseFailureReason))
					{
						LogPrintLine_W(log, "\"_hidden\" attribute of %.*s chunk at 0x%08X was not a value bool: \"%.*s\"", StrPrint(chunkType), chunkHeaderByteIndex, StrPrint(hiddenStr));
						log->hadWarnings = true;
					}
				}
			}
			else if (dictElements.length > 0)
			{
				//NOTE: nGRP and nSHP chunks have no listed attributes, so we are printing a warning if we find any
				LogPrintLine_W(log, "%.*s chunk has %llu attribute%s:", StrPrint(chunkType), dictElements.length, Plural(dictElements.length, "s"));
				log->hadWarnings = true;
				VarArrayLoop(&dictElements, aIndex)
				{
					VarArrayLoopGet(MyStrPair_t, attribute, &dictElements, aIndex);
					LogPrintLine_D(log, "\"%.*s\" = \"%.*s\"", StrPrint(attribute->key), StrPrint(attribute->value));
				}
			}
			//TODO: Do we wanna report unknown attributes as warnings?
			VarArrayClear(&dictElements);
			
			// +==============================+
			// |          nTRN Chunk          |
			// +==============================+
			if (isTranslationNode)
			{
				i32 childNodeId = 0; readSuccess = BinDeser_ReadI32(chunkDataPntr, chunkHeader->dataSize, &chunkReadIndex, &childNodeId); Assert(readSuccess); //TODO: Turn this into an error!
				i32 reservedId = 0; readSuccess = BinDeser_ReadI32(chunkDataPntr, chunkHeader->dataSize, &chunkReadIndex, &reservedId); Assert(readSuccess); //TODO: Turn this into an error!
				i32 layerId = 0; readSuccess = BinDeser_ReadI32(chunkDataPntr, chunkHeader->dataSize, &chunkReadIndex, &layerId); Assert(readSuccess); //TODO: Turn this into an error!
				// LogPrintLine_D(log, "childNodeId = %d", childNodeId);
				// LogPrintLine_D(log, "reservedId = %d", reservedId);
				// LogPrintLine_D(log, "layerId = %d", layerId);
				newNode->numChildren = (childNodeId >= 0) ? 1 : 0;
				newNode->childrenIds[0] = childNodeId;
				newNode->layerId = layerId;
				
				// Read frames
				i32 numFrames = 0; readSuccess = BinDeser_ReadI32(chunkDataPntr, chunkHeader->dataSize, &chunkReadIndex, &numFrames); Assert(readSuccess); //TODO: Turn this into an error!
				// LogPrintLine_D(log, "%d frame%s:", numFrames, Plural(numFrames, "s"));
				if (numFrames > VOX_SCENE_NODE_MAX_FRAMES)
				{
					LogPrintLine_W(log, "One %.*s node had %d frames which is more than %u (aka VOX_SCENE_NODE_MAX_FRAMES)", StrPrint(chunkType), numFrames, VOX_SCENE_NODE_MAX_FRAMES);
					log->hadWarnings = true;
				}
				for (u64 fIndex = 0; fIndex < numFrames; fIndex++)
				{
					dictReadError = TryDeserVoxDictionary(&dictElements, chunkDataPntr, chunkHeader->dataSize, &chunkReadIndex);
					if (dictReadError != Result_Success)
					{
						LogPrintLine_E(log, "Failed to read frame[%llu] attributes on %.*s chunk at 0x%08X: %s", fIndex, StrPrint(chunkType), GetResultStr(dictReadError), chunkHeaderByteIndex);
						LogExitFailure(log, dictReadError);
						break;
					}
					
					// LogPrintLine_D(log, "Frame[%llu] has %llu attribute%s", fIndex, dictElements.length, Plural(dictElements.length, "s"));
					// VarArrayLoop(&dictElements, aIndex)
					// {
					// 	VarArrayLoopGet(MyStrPair_t, frameAttrib, &dictElements, aIndex);
					// 	LogPrintLine_D(log, "Frame Attribute[%llu] \"%.*s\" = \"%.*s\"", aIndex, StrPrint(frameAttrib->key), StrPrint(frameAttrib->value));
					// }
					if (newNode->numFrames < VOX_SCENE_NODE_MAX_FRAMES)
					{
						VoxSceneNodeFrame_t* newFrame = &newNode->frames[newNode->numFrames];
						newNode->numFrames++;
						ClearPointer(newFrame);
						newFrame->frameIndex = -1;
						MyStr_t frameIndexStr = FindVoxDictionaryValue(&dictElements, NewStr("_f"), false);
						if (!IsEmptyStr(frameIndexStr))
						{
							if (!TryParseI32(frameIndexStr, &newFrame->frameIndex, &parseFailureReason))
							{
								LogPrintLine_W(log, "%.*s chunk at 0x%08X frame[%llu] has invalid frame index value (not parsable as i32): \"%.*s\"", StrPrint(chunkType), chunkHeaderByteIndex, fIndex, StrPrint(frameIndexStr));
								log->hadWarnings = true;
							}
						}
						MyStr_t rotationStr = FindVoxDictionaryValue(&dictElements, NewStr("_r"), false);
						if (!IsEmptyStr(rotationStr))
						{
							// LogPrintLine_D(log, "frame[%llu] rotation: \"%.*s\"", fIndex, StrPrint(rotationStr));
							i8 voxFileRotI8 = 0;
							if (TryParseI8(rotationStr, &voxFileRotI8, &parseFailureReason))
							{
								newFrame->rotation = VoxelRotationFromVoxFileRotI8(voxFileRotI8);
							}
							else
							{
								LogPrintLine_W(log, "%.*s chunk at 0x%08X frame[%llu] has invalid rotation value (not parsable as int8): \"%.*s\"", StrPrint(chunkType), chunkHeaderByteIndex, fIndex, StrPrint(rotationStr));
								log->hadWarnings = true;
							}
						}
						MyStr_t translationStr = FindVoxDictionaryValue(&dictElements, NewStr("_t"), false);
						if (!IsEmptyStr(translationStr))
						{
							MyStr_t translationStrWithCommas = AllocString(scratch, &translationStr);
							StrReplaceInPlace(translationStrWithCommas, " ", ",");
							// LogPrintLine_D(log, "frame[%llu] translation: \"%.*s\"", fIndex, StrPrint(translationStr));
							if (!TryParseV3(translationStrWithCommas, &newFrame->translation, &parseFailureReason))
							{
								LogPrintLine_W(log, "%.*s chunk at 0x%08X frame[%llu] has invalid translation value (not parsable as v3): \"%.*s\"", StrPrint(chunkType), chunkHeaderByteIndex, fIndex, StrPrint(translationStr));
								log->hadWarnings = true;
							}
						}
						//TODO: Should we report unknown attribute keys as warnings?
					}
					
					VarArrayClear(&dictElements);
				}
				if (log->hadErrors) { break; }
			}
			// +==============================+
			// |          nGRP Chunk          |
			// +==============================+
			else if (isGroupNode)
			{
				i32 numChildren = 0; readSuccess = BinDeser_ReadI32(chunkDataPntr, chunkHeader->dataSize, &chunkReadIndex, &numChildren); Assert(readSuccess); //TODO: Turn this into an error!
				// LogPrintLine_D(log, "%d child%s:", numChildren, Plural(numChildren, "ren"));
				if (numChildren > VOX_SCENE_NODE_MAX_CHILDREN)
				{
					LogPrintLine_W(log, "One %.*s node had %d children which is more than %u (aka VOX_SCENE_NODE_MAX_CHILDREN)", StrPrint(chunkType), numChildren, VOX_SCENE_NODE_MAX_CHILDREN);
					log->hadWarnings = true;
				}
				for (u64 cIndex = 0; cIndex < numChildren; cIndex++)
				{
					i32 childId = 0; readSuccess = BinDeser_ReadI32(chunkDataPntr, chunkHeader->dataSize, &chunkReadIndex, &childId); Assert(readSuccess); //TODO: Turn this into an error!
					// LogPrintLine_D(log, "Child[%llu]: %d", cIndex, childId);
					if (newNode->numChildren < VOX_SCENE_NODE_MAX_CHILDREN)
					{
						newNode->childrenIds[newNode->numChildren] = childId;
						newNode->numChildren++;
					}
				}
				if (log->hadErrors) { break; }
			}
			// +==============================+
			// |          nSHP Chunk          |
			// +==============================+
			else if (isShapeNode)
			{
				i32 numModels = 0; readSuccess = BinDeser_ReadI32(chunkDataPntr, chunkHeader->dataSize, &chunkReadIndex, &numModels); Assert(readSuccess); //TODO: Turn this into an error!
				// LogPrintLine_D(log, "%d model%s:", numModels, Plural(numModels, "s"));
				if (numModels > VOX_SCENE_NODE_MAX_FRAMES)
				{
					LogPrintLine_W(log, "One %.*s node had %d models which is more than %u (aka VOX_SCENE_NODE_MAX_CHILDREN)", StrPrint(chunkType), numModels, VOX_SCENE_NODE_MAX_CHILDREN);
					log->hadWarnings = true;
				}
				for (u64 mIndex = 0; mIndex < numModels; mIndex++)
				{
					i32 modelId = 0; readSuccess = BinDeser_ReadI32(chunkDataPntr, chunkHeader->dataSize, &chunkReadIndex, &modelId); Assert(readSuccess); //TODO: Turn this into an error!
					if (newNode->numFrames < VOX_SCENE_NODE_MAX_FRAMES)
					{
						VoxSceneNodeFrame_t* newFrame = &newNode->frames[newNode->numFrames];
						newNode->numFrames++;
						ClearPointer(newFrame);
						newFrame->frameIndex = modelId;
					}
					dictReadError = TryDeserVoxDictionary(&dictElements, chunkDataPntr, chunkHeader->dataSize, &chunkReadIndex);
					// LogPrintLine_D(log, "Model[%llu]: %d (%llu attribute%s)", mIndex, modelId, dictElements.length, Plural(dictElements.length, "s"));
					if (dictReadError != Result_Success)
					{
						LogPrintLine_E(log, "Failed to read model attributes on %.*s chunk at 0x%08X model[%llu]: %s", StrPrint(chunkType), chunkHeaderByteIndex, mIndex, GetResultStr(dictReadError));
						LogExitFailure(log, dictReadError);
						break;
					}
					//NOTE: The documentation says the frame index will be stored in an attribute labelled "_f" but we
					//      have never actually seen this attribute. Instead, it seems the modelId stores the frame index
					// MyStr_t frameStr = FindVoxDictionaryValue(&dictElements, NewStr("_f"), false);
					VarArrayClear(&dictElements);
				}
				if (log->hadErrors) { break; }
			}
			else { Assert(false); }
		}
		// +==============================+
		// |          XYZI Chunk          |
		// +==============================+
		else if (MyMemCompare(&chunkHeader->name[0], GetVoxFileChunkNameStr(VoxFileChunkName_XYZI), VOX_FILE_CHUNK_NAME_LENGTH) == 0)
		{
			LogPrintLine_D(log, "Found XYZI chunk: %u byte%s (children: %u byte%s)", chunkHeader->dataSize, Plural(chunkHeader->dataSize, "s"), chunkHeader->childrenSize, Plural(chunkHeader->childrenSize, "s"));
			if (!foundSizeHeader)
			{
				LogPrintLine_E(log, "Found XYZI chunk at 0x%08X without a preceeding SIZE chunk", chunkHeaderByteIndex);
				LogExitFailure(log, Result_MissingSize);
				break;
			}
			const u32 numFilledVoxels = *(const u32*)&chunkDataPntr[0];
			u64 maxFilledVoxels = (u64)(nextFrameSize.width * nextFrameSize.height * nextFrameSize.depth);
			if (numFilledVoxels > maxFilledVoxels)
			{
				LogPrintLine_E(log, "XYZI chunk has %llu filled voxels for %dx%dx%d frame which can only fit %llu voxels", numFilledVoxels, nextFrameSize.width, nextFrameSize.height, nextFrameSize.depth, maxFilledVoxels);
				LogExitFailure(log, Result_TooLarge);
				break;
			}
			const VoxFileEncodedVoxel_t* encodedVoxels = (const VoxFileEncodedVoxel_t*)&chunkDataPntr[sizeof(u32)];
			
			//NOTE: We fill with White if we haven't found the palette chunk because that makes the alpha channel
			//      0xFF and also makes all the channels match so we can do a memset instead of a loop in AddVoxFrame
			VoxFrame_t* newFrame = AddVoxFrame(frameSetOut, nextFrameSize, foundRgbaHeader ? Transparent : White);
			NotNull(newFrame);
			
			for (u64 vIndex = 0; vIndex < numFilledVoxels; vIndex++)
			{
				//NOTE: We are intentionally swapping the Y and Z axis here, since Magica Voxel has Z as the up direction
				v3i coordinate = NewVec3i((i32)encodedVoxels[vIndex].x, (i32)encodedVoxels[vIndex].z, (i32)encodedVoxels[vIndex].y);
				if (coordinate.x >= newFrame->size.width ||
					coordinate.y >= newFrame->size.height ||
					coordinate.z >= newFrame->size.depth)
				{
					LogPrintLine_E(log, "XYZI chunk has filled voxel at (%d, %d, %d) which is outside the bounds of the %dx%dx%d frame", coordinate.x, coordinate.y, coordinate.z, nextFrameSize.width, nextFrameSize.height, nextFrameSize.depth);
					LogExitFailure(log, Result_IndexOutOfBounds);
					break;
				}
				u8 colorIndex = encodedVoxels[vIndex].color;
				Assert(colorIndex > 0);
				colorIndex--;
				u64 voxelIndex = INDEX_FROM_COORD3D(
					coordinate.x, coordinate.y, coordinate.z,
					newFrame->size.width, newFrame->size.height, newFrame->size.depth
				);
				if (foundRgbaHeader)
				{
					NotNull(paletteColors);
					newFrame->colors[voxelIndex] = paletteColors[colorIndex];
				}
				else
				{
					newFrame->colors[voxelIndex].a = colorIndex;
				}
			}
			if (log->hadErrors) { break; }
			
			//NOTE: We clear foundSizeHeader so we have to find another SIZE chunk before the next XYZI chunk
			foundSizeHeader = false;
		}
		// +==============================+
		// |        Unknown Chunk         |
		// +==============================+
		else
		{
			if (MyMemCompare(&chunkHeader->name[0], "LAYR", VOX_FILE_CHUNK_NAME_LENGTH) == 0) {} //26 bytes, 8 chunks
			else if (MyMemCompare(&chunkHeader->name[0], "MATL", VOX_FILE_CHUNK_NAME_LENGTH) == 0) {} //194 bytes, MANY chunks
			else if (MyMemCompare(&chunkHeader->name[0], "rCAM", VOX_FILE_CHUNK_NAME_LENGTH) == 0) {} //173 bytes, 10 chunks
			else if (MyMemCompare(&chunkHeader->name[0], "rOBJ", VOX_FILE_CHUNK_NAME_LENGTH) == 0) {} //35-160 bytes, 15 chunks
			else if (MyMemCompare(&chunkHeader->name[0], "NOTE", VOX_FILE_CHUNK_NAME_LENGTH) == 0) {} //216 bytes, 1 chunk (very end)
			else
			{
				LogPrintLine_W(log, "Unknown chunk name: \"%.*s\" %u byte%s (children: %u byte%s)",
					VOX_FILE_CHUNK_NAME_LENGTH, &chunkHeader->name[0],
					chunkHeader->dataSize, Plural(chunkHeader->dataSize, "s"),
					chunkHeader->childrenSize, Plural(chunkHeader->childrenSize, "s")
				);
				log->hadWarnings = true;
				skipChildren = true;
			}
		}
		
		//Track parent references
		if (chunkHeader->childrenSize > 0)
		{
			if (chunkHeader->childrenSize < sizeof(VoxFileChunkHeader_t))
			{
				LogPrintLine_E(log, "childrenSize on %.*s chunk was too small to contain a chunk header!", VOX_FILE_CHUNK_NAME_LENGTH, &chunkHeader->name[0]);
				LogExitFailure(log, Result_InvalidDataChunkSize);
				break;
			}
			if (numParentChunks >= VOX_FILE_MAX_HIERARCHY_DEPTH)
			{
				LogPrintLine_E(log, "Parent hierarchy is too deep! We only support %u parent references", VOX_FILE_MAX_HIERARCHY_DEPTH);
				LogExitFailure(log, Result_TooDeep);
				break;
			}
			
			parentChunkHeaders[numParentChunks] = chunkHeader;
			parentChunkChildrenStartIndex[numParentChunks] = readByteIndex;
			numParentChunks++;
		}
		
		if (skipChildren) { readByteIndex += chunkHeader->childrenSize; }
	}
	
	if (!log->hadErrors && !foundMainHeader)
	{
		LogWriteLine_E(log, "There was no MAIN chunk in the file!");
		LogExitFailure(log, Result_MissingHeader);
	}
	if (!log->hadErrors && !foundRgbaHeader)
	{
		LogWriteLine_E(log, "There was no RGBA chunk (aka palette chunk) in the file!");
		LogExitFailure(log, Result_MissingFormatChunk);
	}
	if (!log->hadErrors && frameSetOut->frames.length == 0)
	{
		LogWriteLine_E(log, "There were no XYZI chunks (aka data chunks) in the file!");
		LogExitFailure(log, Result_MissingDataChunks);
	}
	
	VarArrayLoop(&sceneNodes, nIndex)
	{
		VarArrayLoopGet(VoxSceneNode_t, sceneNode, &sceneNodes, nIndex);
		if (sceneNode->isShape && sceneNode->numFrames > 0)
		{
			VoxFrame_t* frame = nullptr;
			for (u64 fIndex = 0; fIndex < sceneNode->numFrames; fIndex++)
			{
				VoxSceneNodeFrame_t* nodeFrame = &sceneNode->frames[fIndex];
				if (nodeFrame->frameIndex >= 0 && (u64)nodeFrame->frameIndex < frameSetOut->frames.length)
				{
					frame = VarArrayGetHard(&frameSetOut->frames, (u64)nodeFrame->frameIndex, VoxFrame_t);
					break;
				}
			}
			
			if (frame != nullptr)
			{
				frame->position = Vec3_Zero;
				VoxSceneNode_t* parent = sceneNode;
				while (parent != nullptr)
				{
					if (!IsEmptyStr(parent->name) && IsEmptyStr(frame->name))
					{
						frame->name = AllocString(frameSetOut->allocArena, &parent->name);
					}
					
					for (u64 fIndex = 0; fIndex < parent->numFrames; fIndex++)
					{
						VoxSceneNodeFrame_t* parentFrame = &parent->frames[fIndex];
						frame->position += NewVec3(parentFrame->translation.x, parentFrame->translation.z, parentFrame->translation.y); //swap y and z axis!
					}
					
					Assert(parent->parentId != parent->id);
					parent = FindVoxSceneNodeById(&sceneNodes, parent->parentId);
				}
			}
		}
	}
	
	if (log->hadErrors)
	{
		FreeVoxFrameSet(frameSetOut);
		FreeScratchArena(scratch);
		return false;
	}
	
	FreeScratchArena(scratch);
	frameSetOut->isValid = true;
	return true;
}

bool TryLoadVoxFrameSet(MyStr_t filePath, MemArena_t* memArena, VoxFrameSet_t* frameSetOut)
{
	PlatFileContents_t voxFile = {};
	MemArena_t* scratch = GetScratchArena();
	if (!plat->ReadFileContents(filePath, scratch, false, &voxFile))
	{
		PrintLine_E("Failed to open \"%.*s\"", StrPrint(filePath));
		FreeScratchArena(scratch);
		frameSetOut->error = Result_CouldntOpenFile;
		return false;
	}
	
	MyStr_t voxFileContents = NewStr(voxFile.size, voxFile.chars);
	ProcessLog_t deserLog;
	CreateProcessLog(&deserLog, Megabytes(1), scratch, mainHeap);
	
	VoxFrameSet_t tempFrameSet = {};
	bool deserSuccess = TryDeserVoxFile(voxFileContents, &deserLog, &tempFrameSet, scratch);
	if (deserLog.hadWarnings || deserLog.hadErrors) { DumpProcessLog(&deserLog, "LoadVoxFrameSet Log"); }
	if (!deserSuccess) { frameSetOut->error = (Result_t)deserLog.errorCode; }
	FreeProcessLog(&deserLog);
	
	if (deserSuccess)
	{
		Assert(tempFrameSet.isValid);
		CopyVoxFrameSet(frameSetOut, &tempFrameSet, memArena);
		TriangulateVoxFrameSet(frameSetOut);
		// PrintLine_D("Loaded %llu frame%s", frameSetOut->frames.length, Plural(frameSetOut->frames.length, "s"));
	}
	
	FreeScratchArena(scratch);
	return deserSuccess;
}
