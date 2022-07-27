/*
File:   pig_helpers.cpp
Author: Taylor Robbins
Date:   09\26\2021
Description: 
	** Holds a bunch of useful functions and macros
*/

#define AssertSingleThreaded() Assert(plat != nullptr && pig != nullptr && plat->GetThisThreadId() == pig->mainThreadId)
#define AssertSingleThreaded_() Assert_(plat != nullptr && pig != nullptr && plat->GetThisThreadId() == pig->mainThreadId)

#define AssertNormalEntry() do                            \
{                                                         \
	NotNull(pig);                                         \
	NotNull(plat);                                        \
	Assert(pigEntryPoint != PigEntryPoint_None);          \
	Assert(plat->GetThisThreadId() == pig->mainThreadId); \
} while(0)
#define AssertNormalEntry_() do                            \
{                                                          \
	NotNull_(pig);                                         \
	NotNull_(plat);                                        \
	Assert(pigEntryPoint != PigEntryPoint_None);           \
	Assert_(plat->GetThisThreadId() == pig->mainThreadId); \
} while(0)

#define TIME_SCALED_ANIM(animTimeMs, timeScale) (((1000.0f / PIG_TARGET_FRAMERATE) / (animTimeMs)) * (r32)(timeScale))

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

// +--------------------------------------------------------------+
// |                       Two Pass Helpers                       |
// +--------------------------------------------------------------+
void TwoPassPrint(char* resultPntr, u64 resultLength, u64* currentByteIndex, const char* formatString, ...)
{
	Assert_(resultPntr == nullptr || resultLength > 0);
	NotNull_(currentByteIndex);
	NotNull_(formatString);
	
	u64 printSize = 0;
	va_list args;
	
	va_start(args, formatString);
	int printResult = PrintVa_Measure(formatString, args);
	va_end(args);
	
	if (printResult >= 0)
	{
		printSize = (u64)printResult;
		if (resultPntr != nullptr)
		{
			Assert_(*currentByteIndex <= resultLength);
			u64 spaceLeft = resultLength - *currentByteIndex;
			Assert_(printSize <= spaceLeft);
			va_start(args, formatString);
			PrintVa_Print(formatString, args, &resultPntr[*currentByteIndex], printResult);
			va_end(args);
		}
	}
	else
	{
		//Print error. Use the formatString as a stand-in to indicate an error has occurred in the print formatting
		printSize = MyStrLength64(formatString);
		if (resultPntr != nullptr)
		{
			Assert_(*currentByteIndex <= resultLength);
			u64 spaceLeft = resultLength - *currentByteIndex;
			Assert_(printSize <= spaceLeft);
			MyMemCopy(&resultPntr[*currentByteIndex], formatString, printSize);
		}
	}
	
	*currentByteIndex += printSize;
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