/*
File:   pig_helpers.cpp
Author: Taylor Robbins
Date:   09\10\2023
Description: 
	** Functions that are useful to have around for all games
*/

r32 Animate(r32 min, r32 max, u64 periodMs, u64 offset = 0)
{
	return AnimateBy(ProgramTime, min, max, periodMs, offset);
}

u64 AnimateU64(u64 min, u64 max, u64 periodMs, u64 offset = 0)
{
	return AnimateByU64(ProgramTime, min, max, periodMs, offset);
}

r32 Oscillate(r32 min, r32 max, u64 periodMs, u64 offset = 0)
{
	return OscillateBy(ProgramTime, min, max, periodMs, offset);
}

u64 TimeSince(u64 programTimeSnapshot)
{
	if (programTimeSnapshot <= ProgramTime)
	{
		return ProgramTime - programTimeSnapshot;
	}
	else 
	{
		return 0;
	}
}

#define TIME_SCALED_ANIM(animTimeMs, timeScale) (((r32)TARGET_FRAME_TIME / (animTimeMs)) * (r32)(timeScale))

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
