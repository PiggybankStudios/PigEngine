/*
File:   pig_checkbox.h
Author: Taylor Robbins
Date:   07\09\2022
*/

#ifndef _PIG_CHECKBOX_H
#define _PIG_CHECKBOX_H

#define CHECKBOX_CHECK_ANIM_SPEED  120 //ms
#define CHECKBOX_OUTLINE_THICKNESS 2 //px

struct Checkbox_t
{
	u64 id;
	MemArena_t* allocArena;
	bool enabled;
	
	r32 animProgress;
	bool valueChanged;
	bool currentValue;
	
	rec targetRec;
	
	rec mainRec;
};

#endif //  _PIG_CHECKBOX_H
