/*
File:   common_performance.cpp
Author: Taylor Robbins
Date:   01\06\2022
Description: 
	** Holds some macros that both the platform layer AND the engine can use to measure performance
*/

#if PLATFORM_LAYER
	#if WINDOWS_COMPILATION
	#define COMMON_GET_PERF_TIME() Win32_GetPerfTime()
	#define COMMON_GET_PERF_TIME_DIFF(startPntr, endPntr) Win32_GetPerfTimeDiff((startPntr), (endPntr))
	#else
	#error Unsupported platform in common_performance.h
	#endif
#elif ENGINE_LAYER
	#define COMMON_GET_PERF_TIME() plat->GetPerfTime()
	#define COMMON_GET_PERF_TIME_DIFF(startPntr, endPntr) plat->GetPerfTimeDiff((startPntr), (endPntr))
#else
#error Unsupported layer in common_performance.h
#endif

#define PerfSection(sectionName) do                                   \
{                                                                     \
	if (perfSectionsEnabled)                                          \
	{                                                                 \
		perfSections[perfSectionIndex].time = COMMON_GET_PERF_TIME(); \
		perfSections[perfSectionIndex].name = sectionName;            \
		perfSectionIndex++;                                           \
	}                                                                 \
} while(0)
#define StartPerfSections(maxNumSections, firstSectionName, enabled) PerfSection_t perfSections[(maxNumSections)+1]; u64 perfSectionIndex = 0; bool perfSectionsEnabled = (enabled); PerfSection(firstSectionName)
#define EndPerfSections() PerfSection(nullptr)
#define ArePerfSectionsEnabled() (perfSectionsEnabled)
#define GetNumPerfSections() (perfSectionsEnabled ? (perfSectionIndex-1) : 0)

const char* GetPerfSectionName_(const PerfSection_t* sections, u64 numSections, u64 maxNumSections, u64 sectionIndex)
{
	NotNull_(sections);
	Assert_(sectionIndex+1 < numSections);
	Assert_(numSections <= maxNumSections);
	return sections[sectionIndex].name;
}
r64 GetPerfSectionTimeByIndex_(const PerfSection_t* sections, u64 numSections, u64 maxNumSections, u64 sectionIndex)
{
	NotNull_(sections);
	Assert_(sectionIndex+1 < numSections);
	Assert_(numSections <= maxNumSections);
	const PerfSection_t* section = &sections[sectionIndex];
	const PerfSection_t* nextSection = &sections[sectionIndex+1];
	return COMMON_GET_PERF_TIME_DIFF(&section->time, &nextSection->time);
}
r64 GetPerfSectionTime_(const PerfSection_t* sections, u64 numSections, u64 maxNumSections, const char* sectionName)
{
	bool foundSection = false;
	NotNull_(sections);
	Assert_(numSections <= maxNumSections);
	NotNull_(sectionName);
	for (u64 sIndex = 0; sIndex < numSections-1; sIndex++)
	{
		if (sections[sIndex].name != nullptr && MyStrCompareNt(sections[sIndex].name, sectionName) == 0)
		{
			foundSection = true;
			return GetPerfSectionTimeByIndex_(sections, numSections, maxNumSections, sIndex);
		}
	}
	Assert_(foundSection);
	return 0.0f;
}
r64 GetTotalPerfSectionsTime_(const PerfSection_t* sections, u64 numSections, u64 maxNumSections)
{
	NotNull_(sections);
	Assert_(numSections <= maxNumSections);
	if (numSections == 0) { return 0; }
	r64 result = 0.0;
	for (u64 sIndex = 0; sIndex < numSections-1; sIndex++)
	{
		result += COMMON_GET_PERF_TIME_DIFF(&sections[sIndex].time, &sections[sIndex+1].time);
	}
	return result;
}

void BundlePerfSections_(const PerfSection_t* sections, u64 numSections, u64 maxNumSections, MemArena_t* memArena, PerfSectionBundle_t* bundleOut)
{
	NotNull_(sections);
	NotNull_(memArena);
	NotNull_(bundleOut);
	UNUSED(maxNumSections);
	
	ClearPointer(bundleOut);
	bundleOut->allocArena = memArena;
	CreateVarArray(&bundleOut->sections, memArena, sizeof(PerfSection_t), numSections);
	for (u64 sIndex = 0; sIndex < numSections; sIndex++)
	{
		PerfSection_t* newSection = VarArrayAdd(&bundleOut->sections, PerfSection_t);
		NotNull_(newSection);
		ClearPointer(newSection);
		if (sections[sIndex].name != nullptr)
		{
			newSection->name = AllocCharsAndFillNt(memArena, sections[sIndex].name);
			NotNull_(newSection->name);
		}
		newSection->time = sections[sIndex].time;
	}
}

#define GetPerfSectionName(sectionIndex) GetPerfSectionName_(&perfSections[0], perfSectionIndex, ArrayCount(perfSections), (sectionIndex))
#define GetPerfSectionTimeByIndex(sectionIndex) GetPerfSectionTimeByIndex_(&perfSections[0], perfSectionIndex, ArrayCount(perfSections), (sectionIndex))
#define GetPerfSectionTime(sectionName) GetPerfSectionTime_(&perfSections[0], perfSectionIndex, ArrayCount(perfSections), (sectionName))
#define GetTotalPerfSectionsTime() GetTotalPerfSectionsTime_(&perfSections[0], perfSectionIndex, ArrayCount(perfSections))
#define BundlePerfSections(memArena, bundleOut) BundlePerfSections_(&perfSections[0], perfSectionIndex, ArrayCount(perfSections), (memArena), (bundleOut))

#define PrintPerfSections(printFunc, linePrefix) do                                                                              \
{                                                                                                                                \
	if (ArePerfSectionsEnabled())                                                                                                \
	{                                                                                                                            \
		for (u64 sectionIndex = 0; sectionIndex < GetNumPerfSections(); sectionIndex++)                                          \
		{                                                                                                                        \
			printFunc("%s%s: %.1lfms", (linePrefix), GetPerfSectionName(sectionIndex), GetPerfSectionTimeByIndex(sectionIndex)); \
		}                                                                                                                        \
		printFunc("%sTotal: %.1lfms", (linePrefix), GetTotalPerfSectionsTime());                                                 \
	}                                                                                                                            \
} while(0)

