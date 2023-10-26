/*
File:   pig_time.cpp
Author: Taylor Robbins
Date:   10\10\2021
Description:
	** Holds a bunch of functions that help us manipulate and process timing information coming from the platform layer
*/

r32 Animate(r32 min, r32 max, u64 periodMs, u64 offset = 0)
{
	return AnimateBy(ProgramTime, min, max, periodMs, offset);
}
u64 AnimateU64(u64 min, u64 max, u64 periodMs, u64 offset = 0)
{
	return AnimateByU64(ProgramTime, min, max, periodMs, offset);
}

v2i FrameIndexToSheetIndex(u64 frameIndex, v2i numFrames)
{
	v2i result;
	result.x = (i32)(frameIndex % numFrames.width);
	result.y = (i32)((frameIndex / numFrames.width) % numFrames.height);
	return result;
}

r32 Oscillate(r32 min, r32 max, u64 periodMs, u64 offset = 0)
{
	return OscillateBy(ProgramTime, min, max, periodMs, offset);
}

r32 OscillatePhase(r32 min, r32 max, u64 periodMs, u64 offset = 0)
{
	return OscillatePhaseBy(ProgramTime, min, max, periodMs, offset);
}

r32 OscillateSaw(r32 min, r32 max, u64 periodMs, u64 offset = 0)
{
	return OscillateSawBy(ProgramTime, min, max, periodMs, offset);
}

u64 TimeSince(u64 programTimeSnapshot)
{
	return TimeSinceBy(ProgramTime, programTimeSnapshot);
}
