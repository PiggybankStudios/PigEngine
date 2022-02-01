/*
File:   pig_textbox.h
Author: Taylor Robbins
Date:   01\11\2022
*/

#ifndef _PIG_TEXTBOX_H
#define _PIG_TEXTBOX_H

struct Textbox_t
{
	MemArena_t* allocArena;
	u64 id;
	bool editable;
	bool visible;
	bool hasFocus;
	bool isMultiline;
	bool fixedTextBuffer;
	MyStr_t hintText;
	Font_t* font;
	FontFaceSelector_t fontFaceSelector;
	bool autoSizeHeight;
	bool autoSelectOnFocus;
	v2 innerMargin;
	
	u64 textAllocLength;
	MyStr_t text;
	
	bool selectionActive;
	u64 selectionStart;
	u64 selectionEnd;
	
	bool mouseHovering;
	u64 mouseHoverIndex;
	
	bool isSelectingWithMouse;
	
	v2 scroll;
	v2 scrollGoto;
	v2 contentSize;
	v2 scrollMax;
	
	rec mainRec;
	rec usableRec;
	v2 textPos;
};

#endif //  _PIG_TEXTBOX_H
