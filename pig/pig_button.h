/*
File:   pig_button.h
Author: Taylor Robbins
Date:   09\21\2022
*/

#ifndef _PIG_BUTTON_H
#define _PIG_BUTTON_H

enum ButtonStyle_t
{
	ButtonStyle_None = 0,
	ButtonStyle_Default, //rounded
	ButtonStyle_Bordered,
	ButtonStyle_NumStyles,
};
const char* GetButtonStyleStr(ButtonStyle_t style)
{
	switch (style)
	{
		case ButtonStyle_None:     return "None";
		case ButtonStyle_Default:  return "Default";
		case ButtonStyle_Bordered: return "Bordered";
		default: return "Unknown";
	}
}

struct Button_t
{
	u64 id;
	MemArena_t* allocArena;
	
	ButtonStyle_t style;
	MyStr_t text;
	MyStr_t mouseCapturePrefix;
	FontHandle_t font;
	FontFaceSelector_t fontFaceSelector;
	r32 fontScale;
	v2 innerMargin;
	r32 cornerRadius;
	r32 borderThickness;
	Color_t backColor;
	Color_t textColor;
	Color_t borderColor;
	bool isVisible;
	bool isEnabled;
	r32 opacity;
	
	bool hasTargetRec;
	bool targetRecMoved;
	bool autoSizeHeight;
	bool autoSizeWidth;
	rec targetRec;
	
	bool isDown;
	bool clicked;
	
	rec mainRec;
	rec usableRec;
	TextMeasure_t textMeasure;
	rec textRec;
};

#endif //  _PIG_BUTTON_H
