/*
File:   pig_ui_theme.h
Author: Taylor Robbins
Date:   07\27\2024
*/

#ifndef _PIG_UI_THEME_H
#define _PIG_UI_THEME_H

struct GlobalTheme_t
{
	bool isFilled;
	Color_t background;
	Color_t backgroundDark;
	Color_t panelOutline;
	Color_t text;
	Color_t textFaded;
	Color_t foreground;
	Color_t selected;
	Color_t hover;
	Color_t highlight;
};

#endif //  _PIG_UI_THEME_H
