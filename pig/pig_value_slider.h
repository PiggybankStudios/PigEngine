/*
File:   pig_value_slider.h
Author: Taylor Robbins
Date:   07\08\2022
*/

#ifndef _PIG_VALUE_SLIDER_H
#define _PIG_VALUE_SLIDER_H

enum ValueSliderStyle_t
{
	ValueSliderStyle_Default = 0,
	ValueSliderStyle_Bar, //instead of a slider, we have a loading bar style fill rec
	ValueSliderStyle_NumStyles,
};
const char* GetValueSliderStyleStr(ValueSliderStyle_t sliderStyle)
{
	switch (sliderStyle)
	{
		case ValueSliderStyle_Default: return "Default";
		case ValueSliderStyle_Bar:     return "Bar";
		default: return "Unknown";
	}
}

struct ValueSlider_t
{
	u64 id;
	MemArena_t* allocArena;
	ValueSliderStyle_t style;
	r32 minValue;
	r32 maxValue;
	r32 notchSize;
	SpriteSheet_t* spriteSheet;
	
	rec targetRec;
	r32 sliderWidth;
	
	r32 sliderPos; //percentage [0-1]
	bool valueChanged;
	r32 currentValue;
	
	bool isDraggingSlider;
	v2 dragSliderOffset;
	
	rec mainRec;
	rec usableRec;
	rec fillRec;
	rec sliderRec;
};

#endif //  _PIG_VALUE_SLIDER_H
