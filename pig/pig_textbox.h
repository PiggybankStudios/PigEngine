/*
File:   pig_textbox.h
Author: Taylor Robbins
Date:   01\11\2022
*/

#ifndef _PIG_TEXTBOX_H
#define _PIG_TEXTBOX_H

struct Textbox_t
{
	u64 id;
	MemArena_t* allocArena;
	MyStr_t hintText;
	FontHandle_t font;
	FontFaceSelector_t fontFaceSelector;
	r32 fontScale;
	v2 innerMargin;
	
	union //Flags
	{
		struct
		{
			u8 configurationFlags;
			u8 stateFlags;
			u8 changeFlags;
		};
		struct
		{
			u8 isEditable:1;
			u8 isMultiline:1;
			u8 isFixedSizedBuffer:1;
			u8 autoSizeHeight:1;
			u8 autoSelectOnFocus:1;
			u8 skipNextUnfocusClick:1;
			u8 unusedConfigFlag1:1;
			u8 unusedConfigFlag2:1;
			
			u8 isVisible:1;
			u8 needToRemeasure:1;
			u8 selectionActive:1;
			u8 isSelectDragging:1;
			u8 hasTargetRec:1;
			u8 targetRecMoved:1;
			u8 mouseHovering:1;
			u8 unused1:1;
			
			u8 textChanged:1;
			u8 selectionChanged:1;
		};
	};
	
	u64 textAllocLength;
	MyStr_t text;
	TextMeasure_t textMeasure;
	FontFlowInfo_t flowInfo;
	
	u64 selectionStartIndex;
	u64 selectionEndIndex;
	u64 lastSelectionMoveTime;
	
	rec targetRec;
	
	u64 mouseHoverIndex;
	v2 mouseHoverPos;
	
	u64 lastMouseLeftClickTime;
	v2 lastMouseLeftClickPos;
	u64 mouseQuickLeftClickCount;
	u64 lastMouseRightClickTime;
	v2 lastMouseRightClickPos;
	u64 mouseQuickRightClickCount;
	
	v2 scroll;
	v2 scrollGoto;
	v2 contentSize;
	v2 scrollMax;
	
	rec mainRec;
	rec usableRec;
	v2 textPos;
	
	v2 selectionStartPos;
	v2 selectionEndPos;
	rec selectionRec;
};

struct TextboxColors_t
{
	Color_t backFocusedColor;
	Color_t backUnfocusedColor;
	
	Color_t textFocusedColor;
	Color_t textUnfocusedColor;
	
	Color_t textSelectedFocusedColor;
	Color_t textSelectedUnfocusedColor;
	Color_t selectionFocusedColor;
	Color_t selectionUnfocusedColor;
	Color_t cursorColor;
	Color_t textHintColor;
	
	Color_t outlineFocusedColor;
	Color_t outlineUnfocusedColor;
};

#endif //  _PIG_TEXTBOX_H
