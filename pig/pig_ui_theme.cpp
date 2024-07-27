/*
File:   pig_ui_theme.cpp
Author: Taylor Robbins
Date:   07\27\2024
Description: 
	** Themes are collections of colors that may have inter-dependent values
	** This way code constructing a theme structure can be as simple or precise
	** as it wants. If it only wants to change the highlight color for a particular
	** UI element it can set that value and let the other dependent values be
	** extrapolated from that color choice. If a particular color option isn't
	** important, it can be sources from the GlobalTheme_t structure
*/

#define DeriveThemeColor(colorPntr, valueIfNotSet) do { if ((colorPntr)->value == 0x00000000) { *(colorPntr) = (valueIfNotSet); } } while(0)

void FillTheme(GlobalTheme_t* theme)
{
	NotNull(theme);
	Assert(theme->isFilled == false);
	
	DeriveThemeColor(&theme->background,     NewColor(0xFF323232));
	DeriveThemeColor(&theme->backgroundDark, ColorDarken(theme->background, 7)); //0xFF2B2B2B
	DeriveThemeColor(&theme->panelOutline,   ColorLighten(theme->background, 35)); //0xFF555555
	
	DeriveThemeColor(&theme->text,           NewColor(0xFFF2F2F2));
	DeriveThemeColor(&theme->textFaded,      ColorLerp(theme->text, theme->background, 0.4f)); //0xFF7F7F7F
	
	DeriveThemeColor(&theme->foreground,     NewColor(0xFF404346));
	
	DeriveThemeColor(&theme->selected,       NewColor(0xFF9876AA));
	DeriveThemeColor(&theme->hover,          ColorDesaturate(ColorDarkenPercent(theme->selected, 0.5f), 0.8f)); //0x?
	DeriveThemeColor(&theme->highlight,      ColorDesaturate(ColorDarkenPercent(theme->selected, 0.22f), 0.95f)); //0x?
	
	theme->isFilled = true;
}
