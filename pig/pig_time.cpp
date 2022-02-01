/*
File:   pig_time.cpp
Author: Taylor Robbins
Date:   10\10\2021
Description:
	** Holds a bunch of functions that help us manipulate and process timing information coming from the platform layer
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

v2i FrameIndexToSheetIndex(u64 frameIndex, v2i numFrames)
{
	v2i result;
	result.x = (i32)(frameIndex % numFrames.width);
	result.y = (i32)((frameIndex / numFrames.width) % numFrames.height);
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

r32 OscillatePhaseBy(u64 timeSource, r32 min, r32 max, u64 periodMs, u64 offset = 0)
{
	r32 lerpValue = (SawR32((((timeSource + offset) % periodMs) / (r32)periodMs) * 2*Pi32) + 1.0f) / 2.0f;
	lerpValue = Ease(EasingStyle_CubicOut, lerpValue);
	return min + (max - min) * lerpValue;
}
r32 OscillatePhase(r32 min, r32 max, u64 periodMs, u64 offset = 0)
{
	return OscillatePhaseBy(ProgramTime, min, max, periodMs, offset);
}

r32 OscillateSawBy(u64 timeSource, r32 min, r32 max, u64 periodMs, u64 offset = 0)
{
	r32 lerpValue = (SawR32((((timeSource + offset) % periodMs) / (r32)periodMs) * 2*Pi32) + 1.0f) / 2.0f;
	return min + (max - min) * lerpValue;
}
r32 OscillateSaw(r32 min, r32 max, u64 periodMs, u64 offset = 0)
{
	return OscillateSawBy(ProgramTime, min, max, periodMs, offset);
}

u64 TimeSinceBy(u64 timeSource, u64 programTimeSnapshot)
{
	if (programTimeSnapshot <= timeSource)
	{
		return timeSource - programTimeSnapshot;
	}
	else 
	{
		return 0;
	}
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

MyStr_t GetElapsedString(MemArena_t* memArena, u64 timespanInSecs)
{
	NotNull(memArena);
	MyStr_t result = MyStr_Empty;
	
	u64 numDays = (u64)(timespanInSecs/(60*60*24));
	u64 numHours = (u64)((timespanInSecs/(60*60)) - (numDays*24));
	u64 numMinutes = (u64)((timespanInSecs/60) - (numDays*60*24) - (numHours*60));
	u64 numSeconds = (u64)((timespanInSecs) - (numDays*60*60*24) - (numHours*60*60) - (numMinutes*60));
	if (numDays > 0)
	{
		result = PrintInArenaStr(memArena, "%llud %lluh %llum %llus", numDays, numHours, numMinutes, numSeconds);
	}
	else if (numHours > 0)
	{
		result = PrintInArenaStr(memArena, "%lluh %llum %llus", numHours, numMinutes, numSeconds);
	}
	else if (numMinutes > 0)
	{
		result = PrintInArenaStr(memArena, "%llum %llus", numMinutes, numSeconds);
	}
	else
	{
		result = PrintInArenaStr(memArena, "%llus", numSeconds);
	}
	
	return result;
}
