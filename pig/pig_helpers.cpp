/*
File:   pig_helpers.cpp
Author: Taylor Robbins
Date:   09\26\2021
Description: 
	** Holds a bunch of useful functions and macros
*/

#define AssertSingleThreaded() Assert(plat != nullptr && pig != nullptr && plat->GetThisThreadId() == pig->mainThreadId)
#define AssertSingleThreaded_() Assert_(plat != nullptr && pig != nullptr && plat->GetThisThreadId() == pig->mainThreadId)

#define TIME_SCALED_ANIM(animTimeMs, timeScale) (((r32)PIG_DEFAULT_FRAME_TIME / (animTimeMs)) * (r32)(timeScale))

void* PlatAllocFunc(u64 size)
{
	NotNull2(plat, plat->AllocateMemory);
	return plat->AllocateMemory(size, AllocAlignment_None);
}
void* PlatReallocFunc(void* allocPntr, u64 newSize, u64 oldSize = 0)
{
	NotNull2(plat, plat->AllocateMemory);
	return plat->ReallocMemory(allocPntr, newSize, oldSize, AllocAlignment_None);
}
void PlatFreeFunc(void* allocPntr)
{
	NotNull2(plat, plat->AllocateMemory);
	plat->FreeMemory(allocPntr, 0, nullptr);
}

//returns true on 1 frame, when animTimerPntr reaches target
bool UpdateAnimationUpTo(r32* animTimerPntr, r32 animationTimeMs, r32 target)
{
	NotNull(animTimerPntr);
	if (*animTimerPntr < target)
	{
		*animTimerPntr += TIME_SCALED_ANIM(animationTimeMs, (r32)TimeScale);
		if (*animTimerPntr >= target)
		{
			*animTimerPntr = target;
			return true;
		}
	}
	return false;
}
bool UpdateAnimationUp(r32* animTimerPntr, r32 animationTimeMs)
{
	return UpdateAnimationUpTo(animTimerPntr, animationTimeMs, 1.0f);
}

//returns true on 1 frame, when animTimerPntr reaches target
bool UpdateAnimationDownTo(r32* animTimerPntr, r32 animationTimeMs, r32 target)
{
	NotNull(animTimerPntr);
	if (*animTimerPntr > target)
	{
		*animTimerPntr -= TIME_SCALED_ANIM(animationTimeMs, TimeScale);
		if (*animTimerPntr <= target)
		{
			*animTimerPntr = target;
			return true;
		}
	}
	return false;
}
bool UpdateAnimationDown(r32* animTimerPntr, r32 animationTimeMs)
{
	return UpdateAnimationDownTo(animTimerPntr, animationTimeMs, 0.0f);
}

//TODO: These functions are totally just made up to try out a log base 2 approach to see if it "sounds right"
//      I need to do more research and learning about audio and the human ear to know what the real function should be
//      These functions also don't perfectly go through (1,1) so clamping IS necassary
r32 ConvertVolumeToLoudness(r32 volume)
{
	return ClampR32(EaseLogTwoOutCustom(volume), 0.0f, 1.0f);
}
r32 ConvertLoudnessToVolume(r32 loudness)
{
	return ClampR32(EaseLogTwoInCustom(loudness), 0.0f, 1.0f);
}
//TODO: Make these calculate in r64 precision
r64 ConvertVolumeToLoudness(r64 volume)
{
	return (r64)ClampR32(EaseLogTwoOutCustom((r32)volume), 0.0f, 1.0f);
}
r64 ConvertLoudnessToVolume(r64 loudness)
{
	return (r64)ClampR32(EaseLogTwoInCustom((r32)loudness), 0.0f, 1.0f);
}

//WARNING: Stops at the first period it sees to try and prevent making a folder with the intended file's name
bool CreateFoldersForPath(MyStr_t folderOrFilePath)
{
	NotNull(plat);
	NotNullStr(&folderOrFilePath);
	u64 prevFolderByteIndex = 0;
	u32 previousCodepoint = 0;
	for (u64 bIndex = 0; bIndex <= folderOrFilePath.length; )
	{
		u32 codepoint = 0;
		u8 codepointSize = 0;
		if (bIndex < folderOrFilePath.length)
		{
			codepointSize = GetCodepointForUtf8Str(folderOrFilePath, bIndex, &codepoint);
			if (codepointSize == 0)
			{
				codepointSize = 1;
				codepoint = CharToU32(folderOrFilePath.pntr[bIndex]);
			}
		}
		else
		{
			codepoint = CharToU32('/');
			codepointSize = 1;
		}
		
		if (codepoint == '.')
		{
			return true;
		}
		else if ((codepoint == '/' || codepoint == '\\') && bIndex > 0)
		{
			if (bIndex > prevFolderByteIndex && !(bIndex == 2 && previousCodepoint == ':'))
			{
				MyStr_t parentFolder = StrSubstring(&folderOrFilePath, 0, bIndex);
				bool doesFolderExist = false;
				plat->DoesFileExist(parentFolder, &doesFolderExist);
				if (!doesFolderExist)
				{
					if (!plat->CreateFolder(parentFolder))
					{
						PrintLine_E("Failed to create folder at \"%.*s\"", StrPrint(parentFolder));
						return false;
					}
				}
			}
			prevFolderByteIndex = bIndex+codepointSize;
		}
		
		previousCodepoint = codepoint;
		bIndex += codepointSize;
	}
	return true;
}

// +--------------------------------------------------------------+
// |                      Bool Array Helpers                      |
// +--------------------------------------------------------------+
bool GetBoolArrayValue(bool* boolArray, v2i index, v2i size)
{
	Assert(index.x >= 0 && index.y >= 0);
	Assert(index.x < size.width);
	Assert(index.y < size.height);
	return boolArray[(u64)(index.y * size.width) + (u64)index.x];
}
void SetBoolArrayValue(bool* boolArray, v2i index, v2i size, bool newValue)
{
	Assert(index.x >= 0 && index.y >= 0);
	Assert(index.x < size.width);
	Assert(index.y < size.height);
	boolArray[(u64)(index.y * size.width) + (u64)index.x] = newValue;
}
bool* GetBoolArrayPntr(bool* boolArray, v2i index, v2i size)
{
	Assert(index.x >= 0 && index.y >= 0);
	Assert(index.x < size.width);
	Assert(index.y < size.height);
	return &boolArray[(u64)(index.y * size.width) + (u64)index.x];
}
