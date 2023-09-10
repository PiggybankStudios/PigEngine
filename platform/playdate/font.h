/*
File:   font.h
Author: Taylor Robbins
Date:   09\09\2023
*/

#ifndef _FONT_H
#define _FONT_H

struct Font_t
{
	bool isValid;
	u64 numChars;
	u16 ranges; //FontRange_t
	u16 partialRanges; //FontRange_t
	u8 numRanges;
	u8 numPartialRanges;
	i32 lineHeight;
	
	LCDFont* font;
};

//works on multiple ranges too
bool FontHasRange(Font_t font, u16 range, bool partial = false)
{
	return (((partial ? font.partialRanges : font.ranges) & range) == range);
}

const char* GetFontCapsStr(Font_t font)
{
	if (FontHasRange(font, FontRanges_All)) { return "All"; }
	if (FontHasRange(font, FontRanges_ASCII))    { return (font.partialRanges == FontRanges_ASCII)    ? "ASCII"     : "ASCII+";     }
	if (FontHasRange(font, FontRanges_English))  { return (font.partialRanges == FontRanges_English)  ? "English"   : "English+";   }
	if (FontHasRange(font, FontRanges_Japanese)) { return (font.partialRanges == FontRanges_Japanese) ? "Japanese"  : "Japanese+";  }
	if (FontHasRange(font, FontRanges_Alphabet)) { return (font.partialRanges == FontRanges_Alphabet) ? "Alphabet"  : "Alphabet+";  }
	if (FontHasRange(font, FontRange_Uppercase)) { return (font.partialRanges == FontRange_Uppercase) ? "Uppercase" : "Uppercase+"; }
	if (FontHasRange(font, FontRange_Numbers))   { return (font.partialRanges == FontRange_Numbers)   ? "Numbers"   : "Numbers+";   }
	if (FontHasRange(font, FontRanges_ASCII,    true)) { return "(ASCII)";     }
	if (FontHasRange(font, FontRanges_English,  true)) { return "(English)";   }
	if (FontHasRange(font, FontRanges_Japanese, true)) { return "(Japanese)";  }
	if (FontHasRange(font, FontRanges_Alphabet, true)) { return "(Alphabet)";  }
	if (FontHasRange(font, FontRange_Uppercase, true)) { return "(Uppercase)"; }
	if (FontHasRange(font, FontRange_Numbers,   true)) { return "(Numbers)";   }
	if (font.ranges != FontRange_None) { return "Some"; }
	if (font.partialRanges != FontRange_None) { return "(Some)"; }
	return "None";
}

#endif //  _FONT_H
