/*
File:   pig_confirm_dialog.h
Author: Taylor Robbins
Date:   09\20\2022
*/

#ifndef _PIG_CONFIRM_DIALOG_H
#define _PIG_CONFIRM_DIALOG_H

#define CONFIRM_DIALOG_MAX_NUM_BUTTONS   4
#define CONFIRM_DIALOG_OPEN_ANIM_TIME    300 //px
#define CONFIRM_DIALOG_CLOSE_ANIM_TIME   200 //px
#define CONFIRM_DIALOG_CONTENT_APPEAR_OPEN_PERCENT       0.6f

#define CONFIRM_DIALOG_CORNER_CIRCLE_RADIUS              24 //px
#define CONFIRM_DIALOG_CORNER_CIRCLE_MARGIN              3 //px
#define CONFIRM_DIALOG_OUTLINE_THICKNESS                 4 //px
#define CONFIRM_DIALOG_CORNER_CIRCLE_OUTLINE_THICKNESS   1.5f //px
#define CONFIRM_DIALOG_TEXT_MARGIN_LEFTRIGHT             50 //px
#define CONFIRM_DIALOG_TEXT_MARGIN_TOP                   40 //px
#define CONFIRM_DIALOG_TEXT_MARGIN_BOTTOM                30 //px
#define CONFIRM_DIALOG_BTN_MARGIN_BOTTOM                 15 //px
#define CONFIRM_DIALOG_BTN_MARGIN_LEFTRIGHT              40 //px
#define CONFIRM_DIALOG_BTN_PADDING                       15 //px
#define CONFIRM_DIALOG_BTN_INNER_MARGIN                  10 //px
#define CONFIRM_DIALOG_MIN_WIDTH                         400 //px
#define CONFIRM_DIALOG_MIN_HEIGHT                        300 //px
#define CONFIRM_DIALOG_MAX_WIDTH                         800 //px
#define CONFIRM_DIALOG_BTN_MIN_HEIGHT                    75 //px
#define CONFIRM_DIALOG_BTN_STYLE                         ButtonStyle_Bordered
#define CONFIRM_DIALOG_BTN_CORNER_RADIUS                 10 //px
#define CONFIRM_DIALOG_BTN_BORDER_THICKNESS              2 //px
#define CONFIRM_DIALOG_BTN_YES_COLOR                     NewColor(0xA0496804)
#define CONFIRM_DIALOG_BTN_NO_COLOR                      NewColor(0x80000000) //NewColor(0xFF6D0409)
#define CONFIRM_DIALOG_BTN_TEXT_COLOR                    MonokaiWhite
#define CONFIRM_DIALOG_BTN_BORDER_COLOR                  NewColor(0xFFDA8F01)

#define CONFIRM_DIALOG_CALLBACK_DEF(functionName) void functionName(struct ConfirmDialog_t* dialog, struct ConfirmDialogBtn_t* selectedButton, u64 selectedValue, void* contextPntr)
typedef CONFIRM_DIALOG_CALLBACK_DEF(ConfirmDialogCallback_f);

struct ConfirmDialogBtn_t
{
	u64 index;
	u64 value;
	Button_t btn;
};

struct ConfirmDialog_t
{
	u64 id;
	MemArena_t* allocArena;
	
	bool isOpen;
	bool autoDispose;
	r32 openAnimProgress;
	
	MyStr_t messageStr;
	i64 escapeButtonIndex;
	bool allowClickOutsideToCloseDialog;
	u64 numButtons;
	ConfirmDialogBtn_t buttons[CONFIRM_DIALOG_MAX_NUM_BUTTONS];
	
	bool buttonSelected;
	u64 buttonSelectIndex;
	u64 buttonSelectValue;
	ConfirmDialogCallback_f* callback;
	void* callbackContext;
	
	rec mainRec;
	rec contentRec;
	rec buttonsRec;
	TextMeasure_t messageMeasure;
	rec messageRec;
};

#endif //  _PIG_CONFIRM_DIALOG_H
