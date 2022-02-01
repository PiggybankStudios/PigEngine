/*
File:   pig_func_defs.h
Author: Taylor Robbins
Date:   01\02\2022
Description:
	** Because of the way we #include mostly .cpp files and in a specific order based on dependencies, sometimes we
	** have the need to pre-declare specific functions so that a dependency loop can be resolved. Functions in this
	** file are implemented in various other .cpp files (generally files in the pig folder and with pig_ prefix)
*/

#ifndef _PIG_FUNC_DEFS_H
#define _PIG_FUNC_DEFS_H

//pig_debug_console.cpp
void DebugConsoleLineAdded(DebugConsole_t* console, StringFifoLine_t* newLine);

//pig_perf_graph.cpp
void PigPerfGraphMark_(PigPerfGraph_t* graph);
#define PigPerfGraphMark() PigPerfGraphMark_(&pig->perfGraph)

#endif //  _PIG_FUNC_DEFS_H
