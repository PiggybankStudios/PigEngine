/*
File:   font_range.h
Author: Taylor Robbins
Date:   09\09\2023
*/

#ifndef _FONT_RANGE_H
#define _FONT_RANGE_H

enum FontRange_t
{
	FontRange_None        = 0x0000,
	
	FontRange_Space          = 0x0001,
	FontRange_Uppercase      = 0x0002,
	FontRange_Lowercase      = 0x0004,
	FontRange_Numbers        = 0x0008,
	FontRange_Punctuation    = 0x0010,
	FontRange_AsciiSyntax    = 0x0020,
	FontRange_Hiragana       = 0x0040,
	FontRange_HiraganaMarked = 0x0080,
	FontRange_HiraganaSmall  = 0x0100,
	FontRange_Katakana       = 0x0200,
	FontRange_KatakanaMarked = 0x0400,
	FontRange_KatakanaSmall  = 0x0800,
	
	FontRanges_Alphabet   = (FontRange_Uppercase|FontRange_Lowercase),
	FontRanges_English    = (FontRange_Space|FontRange_Uppercase|FontRange_Lowercase|FontRange_Numbers|FontRange_Punctuation),
	FontRanges_ASCII      = (FontRange_Space|FontRange_Uppercase|FontRange_Lowercase|FontRange_Numbers|FontRange_Punctuation|FontRange_AsciiSyntax),
	FontRanges_Japanese   = (FontRange_Hiragana|FontRange_HiraganaMarked|FontRange_HiraganaSmall|FontRange_Katakana|FontRange_KatakanaMarked|FontRange_KatakanaSmall),
	FontRanges_All        = 0x0FFF,
	FontRange_NumRanges   = 12,
};

const char* GetFontRangeStr(FontRange_t range)
{
	switch (range)
	{
		case FontRange_Space:          return "Space";
		case FontRange_Uppercase:      return "Uppercase";
		case FontRange_Lowercase:      return "Lowercase";
		case FontRange_Numbers:        return "Numbers";
		case FontRange_Punctuation:    return "Punctuation";
		case FontRange_AsciiSyntax:    return "AsciiSyntax";
		case FontRange_Hiragana:       return "Hiragana";
		case FontRange_HiraganaMarked: return "HiraganaMarked";
		case FontRange_HiraganaSmall:  return "HiraganaSmall";
		case FontRange_Katakana:       return "Katakana";
		case FontRange_KatakanaMarked: return "KatakanaMarked";
		case FontRange_KatakanaSmall:  return "KatakanaSmall";
		default: return "Unknown";
	}
}

#define UNKNOWN_CHAR_STR       "�"
#define UNKNOWN_CHAR_CODEPOINT 0xFFFD

#define FONT_RANGE_CHARS_WIDTH   46
const char* FontRangeChars[FontRange_NumRanges][FONT_RANGE_CHARS_WIDTH] =
{
	{ " ",  "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0" }, //FontRange_Space
	{ "A",  "B",  "C",  "D",  "E",  "F",  "G",  "H",  "I",  "J",  "K",  "L",  "M",  "N",  "O",  "P",  "Q",  "R",  "S",  "T",  "U",  "V",  "W",  "X",  "Y",  "Z",  "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0" }, //FontRange_Uppercase
	{ "a",  "b",  "c",  "d",  "e",  "f",  "g",  "h",  "i",  "j",  "k",  "l",  "m",  "n",  "o",  "p",  "q",  "r",  "s",  "t",  "u",  "v",  "w",  "x",  "y",  "z",  "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0" }, //FontRange_Lowercase
	{ "0",  "1",  "2",  "3",  "4",  "5",  "6",  "7",  "8",  "9",  "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0" }, //FontRange_Numbers
	{ ".",  ",",  "!",  "?",  "\'", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0" }, //FontRange_Punctuation
	{ "\"", "#",  "$",  "%",  "&",  "(",  ")",  "*",  "+",  ",",  "-",  ".",  "/",  ":",  ";",  "<",  "=",  ">",  "@",  "[",  "\\", "]",  "^",  "_",  "`",  "{",  "|",  "}",  "~",  "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0" }, //FontRange_AsciiSyntax
	{ "あ",  "い", "う",  "え",  "お",  "か", "き",  "く",  "け",  "こ",  "さ",  "し", "す",  "せ",  "そ",  "た", "ち",  "つ",  "て", "と",  "な",  "に", "ぬ",  "ね",  "の", "は",  "ひ", "ふ",  "へ",  "ほ", "ま",  "み",  "む", "め",  "も", "や",  "ゆ",  "よ", "ら",  "り",  "る",  "れ", "ろ",  "わ",  "を", "ん"  }, //FontRange_Hiragana
	{ "が",  "ぎ", "ぐ",  "げ",  "ご",  "ざ", "じ",  "ず",  "ぜ",  "ぞ", "だ",  "ぢ", "づ",  "で",  "ど",  "ば", "び",  "ぶ",  "べ", "ぼ",  "ぱ",  "ぴ", "ぷ", "ぺ",  "ぽ",  "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0" }, //FontRange_HiraganaMarked
	{ "ゃ",  "ゅ", "ょ",  "\0",  "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0" }, //FontRange_HiraganaSmall
	{ "ア",  "イ", "ウ",  "エ",  "オ",  "カ",  "キ",  "ク", "ケ",  "コ",  "サ",  "シ", "ス",  "セ",  "ソ",  "タ", "チ",  "ツ",  "テ", "ト",  "ナ",  "ニ", "ヌ",  "ネ",  "ノ",  "ハ",  "ヒ", "フ",  "ヘ",  "ホ", "マ",  "ミ",   "ム", "メ",  "モ", "ヤ",  "ユ",  "ヨ", "ラ",  "リ",  "ル",  "レ",  "ロ",  "ワ",  "ヲ", "ン"  }, //FontRange_Katakana
	{ "ガ",  "ギ", "グ",  "ゲ",  "ゴ",  "ザ", "ジ",  "ズ",  "ゼ", "ゾ",  "ダ",  "ヂ", "ヅ",  "デ",  "ド",  "バ", "ビ",  "ブ",  "ベ", "ボ",  "パ",  "ピ", "プ",  "ペ",  "ポ",  "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0" }, //FontRange_KatakanaMarked
	{ "ャ",  "ュ", "ョ",  "\0",  "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0" }, //FontRange_KatakanaSmall
};

u8 GetNumCharsInFontRange(FontRange_t range)
{
	switch (range)
	{
		case FontRange_Space:          return 1;
		case FontRange_Uppercase:      return 26;
		case FontRange_Lowercase:      return 26;
		case FontRange_Numbers:        return 10;
		case FontRange_Punctuation:    return 5;
		case FontRange_AsciiSyntax:    return 29;
		case FontRange_Hiragana:       return 46;
		case FontRange_HiraganaMarked: return 25;
		case FontRange_HiraganaSmall:  return 3;
		case FontRange_Katakana:       return 46;
		case FontRange_KatakanaMarked: return 25;
		case FontRange_KatakanaSmall:  return 3;
		default: return 0;
	}
}

u8 GetFontRangeIndex(FontRange_t range)
{
	switch (range)
	{
		case FontRange_Space:          return 0;
		case FontRange_Uppercase:      return 1;
		case FontRange_Lowercase:      return 2;
		case FontRange_Numbers:        return 3;
		case FontRange_Punctuation:    return 4;
		case FontRange_AsciiSyntax:    return 5;
		case FontRange_Hiragana:       return 6;
		case FontRange_HiraganaMarked: return 7;
		case FontRange_HiraganaSmall:  return 8;
		case FontRange_Katakana:       return 9;
		case FontRange_KatakanaMarked: return 10;
		case FontRange_KatakanaSmall:  return 11;
		default: DebugAssert(false); return 0;
	}
}

FontRange_t FontRangeByIndex(u64 index)
{
	return (FontRange_t)((u16)1 << (u16)(index % FontRange_NumRanges));
}

const char* GetFontRangeCharStr(FontRange_t range, u64 charIndex)
{
	return FontRangeChars[GetFontRangeIndex(range)][charIndex % FONT_RANGE_CHARS_WIDTH];
}

u32 GetFontRangeChar(FontRange_t range, u64 charIndex)
{
	const char* charStr = GetFontRangeCharStr(range, charIndex);
	if (charStr[0] == '\0') { return 0; }
	u32 codepoint = 0;
	u8 codepointWidth = GetCodepointForUtf8(MyStrLength64(charStr), charStr, &codepoint);
	Assert(codepointWidth != 0);
	return codepoint;
}

#endif //  _FONT_RANGE_H
