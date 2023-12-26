/*
File:   performance.cpp
Author: Taylor Robbins
Date:   12\26\2023
Description: 
	** Holds functions that help us measure performance of sections of code.
	** NOTE: Only one timer can be running at a time!
*/

#define MAX_NUM_PERF_TIMERS 254

#define PERF_FORMAT_STR               "%llu.%03ums"
#define PERF_FORMAT(timeValueUsU64)   timeValueUsU64/1000, (u32)(timeValueUsU64%1000)

static u8 nextPerfTimerIndex = 0;
static float perfTimers[MAX_NUM_PERF_TIMERS];

u8 StartPerfTime()
{
	if (nextPerfTimerIndex > 0)
	{
		float elapsedTimeSecs = pd->system->getElapsedTime();
		for (u8 tIndex = 0; tIndex < nextPerfTimerIndex; tIndex++) { perfTimers[tIndex] += elapsedTimeSecs; }
	}
	u8 resultTimerIndex = nextPerfTimerIndex;
	perfTimers[nextPerfTimerIndex] = 0.0f;
	nextPerfTimerIndex++;
	pd->system->resetElapsedTime();
	return resultTimerIndex;
}

//Returns number of microsends since calling StartPerfTime()
u64 EndPerfTime(u8 timerIndex)
{
	r32 elapsedTimeSecs = pd->system->getElapsedTime();
	Assert(timerIndex == nextPerfTimerIndex-1);
	for (u8 tIndex = 0; tIndex < nextPerfTimerIndex; tIndex++) { perfTimers[tIndex] += elapsedTimeSecs; }
	u64 totalElapsedTimeUs = (u64)(perfTimers[timerIndex] * (r32)NUM_US_PER_SECOND);
	nextPerfTimerIndex--;
	pd->system->resetElapsedTime();
	return totalElapsedTimeUs;
}

u64 MarkPerfTime(u8 timerIndex)
{
	u64 result = EndPerfTime(timerIndex);
	u8 newTimerIndex = StartPerfTime();
	Assert(newTimerIndex == timerIndex);
	return result;
}
