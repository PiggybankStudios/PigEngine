/*
File:   pig_helpers.cpp
Author: Taylor Robbins
Date:   09\10\2023
Description: 
	** Functions that are useful to have around for all games
*/

r32 AnimateBy(u64 timeSource, r32 min, r32 max, u64 periodMs, u64 offset = 0)
{
	r32 lerpValue = ((timeSource + offset) % periodMs) / (r32)periodMs;
	return min + (max - min) * lerpValue;
}
r32 Animate(r32 min, r32 max, u64 periodMs, u64 offset = 0)
{
	return AnimateBy(ProgramTime, min, max, periodMs, offset);
}
r32 AnimateAndPauseBy(u64 timeSource, r32 min, r32 max, u64 animationTime, u64 pauseTime, u64 offset = 0)
{
	u64 lerpIntValue = (u64)((timeSource + offset) % (animationTime + pauseTime));
	if (lerpIntValue >= animationTime) { lerpIntValue = animationTime; }
	return min + (max - min) * (lerpIntValue / (r32)animationTime);
}

u64 AnimateByU64(u64 timeSource, u64 min, u64 max, u64 periodMs, u64 offset = 0)
{
	Assert(periodMs > 0);
	if (min == max) { return min; }
	bool reversed = false;
	if (max < min) { SWAP_VARIABLES(u64, min, max); reversed = true; }
	u64 bucketTime = periodMs / (max-min);
	u64 inLoopTime = ((timeSource + offset) % periodMs);
	if (reversed) { inLoopTime = periodMs-1 - inLoopTime; }
	u64 result = ClampU64(inLoopTime / bucketTime, min, max-1);
	return result;
}
u64 AnimateU64(u64 min, u64 max, u64 periodMs, u64 offset = 0)
{
	return AnimateByU64(ProgramTime, min, max, periodMs, offset);
}
u64 AnimateAndPauseByU64(u64 timeSource, u64 min, u64 max, u64 animationTime, u64 pauseTime, u64 offset = 0, bool useFirstFrameForPause = false)
{
	Assert(animationTime > 0);
	if (min == max) { return min; }
	bool reversed = false;
	if (max < min) { SWAP_VARIABLES(u64, min, max); reversed = true; }
	u64 bucketTime = animationTime / (max - min);
	u64 inLoopTime = ((timeSource + offset) % (animationTime + pauseTime));
	if (inLoopTime > animationTime) { inLoopTime = (useFirstFrameForPause ? 0 : animationTime-1); }
	if (reversed) { inLoopTime = animationTime-1 - inLoopTime; }
	u64 result = ClampU64(inLoopTime / bucketTime, min, max-1);
	return result;
}

r32 OscillateBy(u64 timeSource, r32 min, r32 max, u64 periodMs, u64 offset = 0)
{
	r32 lerpValue = (SinR32((((timeSource + offset) % periodMs) / (r32)periodMs) * 2*Pi32) + 1.0f) / 2.0f;
	return min + (max - min) * lerpValue;
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
