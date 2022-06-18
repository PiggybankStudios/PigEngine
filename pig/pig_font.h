/*
File:   pig_font.h
Author: Taylor Robbins
Date:   12\22\2021
*/

#ifndef _PIG_FONT_H
#define _PIG_FONT_H

struct FontCharInfo_t
{
	u64 charIndex;
	u32 codepoint;
	union
	{
		reci sourceRec;
		struct { v2i sourcePos, sourceSize; };
		struct { v2i unused, renderSize; };
	};
	union
	{
		rec logicalRec;
		struct { v2 logicalOffset, logicalSize; };
	};
	v2 renderOffset;
	r32 advanceX;
};

struct FontRange_t
{
	u64 rangeIndex;
	bool contiguous;
	u32 minCodepoint;
	u32 maxCodepoint; //not-inclusive
	VarArray_t charInfos;
};

enum FontBakeFlag_t
{
	FontBakeFlag_IsValid      = 0x01,
	FontBakeFlag_IsSpriteFont = 0x02,
	FontBakeFlag_IsScalable   = 0x04,
	FontBakeFlag_IsColored    = 0x08,
	
	FontBakeFlag_None = 0x00,
	FontBakeFlag_All  = 0x0F,
	FontBakeFlag_NumFlags = 4,
};

struct FontBake_t
{
	u64 bakeIndex;
	u8 flags;
	Texture_t bitmap;
	VarArray_t ranges;
	PlatRectPackContext_t rectPackContext;
};

enum FontFaceFlag_t
{
	FontFaceFlag_IsValid    = 0x0001, //false if any bakes failed
	FontFaceFlag_IsBold     = 0x0002,
	FontFaceFlag_IsItalic   = 0x0004,
	FontFaceFlag_IsDefault  = 0x0008, //should only be set on a single face in the font and correspond to defaultFaceIndex
	FontFaceFlag_IsActive   = 0x0010, //whether the face holds an active handle to a font file for generating new bakes
	
	FontFaceFlag_BoldItalic = (FontFaceFlag_IsBold|FontFaceFlag_IsItalic),
	
	FontFaceFlag_None = 0x0000,
	FontFaceFlag_All  = 0x000F,
	FontFaceFlag_NumFlags = 5,
};

struct FontFace_t
{
	u64 faceIndex;
	u16 flags;
	i32 fontSize;
	r32 underlineOffset;
	r32 underlineThickness;
	
	u32 minCodepoint;
	u32 maxCodepoint;
	u32 numCharacters;
	VarArray_t bakes;
	
	PlatFileContents_t fontFile;
	
	r32 lineHeight;
	r32 maxAscend;
	r32 maxDescend;
	r32 renderLineHeight;
	r32 renderMaxAscend;
	r32 renderMaxDescend;
};

struct Font_t
{
	bool isValid; //at least one face is valid
	bool hasFailures; //at least one face is invalid
	MemArena_t* allocArena;
	
	u64 defaultFaceIndex;
	VarArray_t faces;
};

// +--------------------------------------------------------------+
// |                       Font Flow Types                        |
// +--------------------------------------------------------------+
struct FontFaceSelector_t
{
	bool selectDefault; //if true, none of the other options matter
	i32 fontSize; //0 = any size
	bool bold;
	bool italic;
};
FontFaceSelector_t SelectDefaultFontFace()
{
	FontFaceSelector_t result = {};
	result.selectDefault = true;
	return result;
}
FontFaceSelector_t SelectFontFace(i32 fontSize, bool bold = false, bool italic = false)
{
	FontFaceSelector_t result;
	result.selectDefault = false;
	result.fontSize = fontSize;
	result.bold = bold;
	result.italic = italic;
	return result;
}

struct TextMeasure_t
{
	v2 size;
	v2 offset;
};

enum TextAlignment_t
{
	TextAlignment_Left = 0,
	TextAlignment_Center,
	TextAlignment_Right,
	TextAlignment_NumOptions,
};
const char* GetTextAlignmentStr(TextAlignment_t alignment)
{
	switch (alignment)
	{
		case TextAlignment_Left: return "Left";
		case TextAlignment_Center: return "Center";
		case TextAlignment_Right: return "Right";
		default: return "Unknown";
	}
}

struct FontFlowState_t
{
	MyStr_t text;
	const Font_t* font; //Call ClearFlowStateCache if changed mid-flow
	FontFaceSelector_t faceSelector; //Call ClearFlowStateCache if changed mid-flow
	Color_t color;
	v2 startPos;
	r32 scale;
	r32 maxWidth;
	TextAlignment_t alignment;
	bool justMeasuring;
	
	bool underlined;
	v2 underlineStartPos;
	
	u64 byteIndex;
	u64 charIndex;
	u64 lineIndex;
	u64 columnIndex;
	bool consumeCharAtLineBreak;
	u64 nextLineBreakIndex;
	r32 widthToLineBreak;
	bool calledBeforeLineOnThisLine;
	bool calledAfterLineOnThisLine;
	v2 position;
	v2 lineStartPos;
	rec lineRec;
	r32 thisLineHeight;
	const FontFace_t* selectedFaceCache;
	const FontBake_t* selectedBakeCache;
};
void ClearFlowStateCache(FontFlowState_t* flowState)
{
	NotNull(flowState);
	flowState->selectedFaceCache = nullptr;
	flowState->selectedBakeCache = nullptr;
}

//NOTE: FFCB stands for Font Flow CallBack
#define FFCB_BEFORE_CHAR_DEFINITION(functionName) bool functionName(u32 codepoint, const FontCharInfo_t* charInfo, rec logicalRec, rec renderRec, FontFlowState_t* state, void* context)
typedef FFCB_BEFORE_CHAR_DEFINITION(FontFlowBeforeCharCallback_f);
#define FFCB_AFTER_CHAR_DEFINITION(functionName) void functionName(u32 codepoint, const FontCharInfo_t* charInfo, rec logicalRec, rec renderRec, FontFlowState_t* state, void* context)
typedef FFCB_AFTER_CHAR_DEFINITION(FontFlowAfterCharCallback_f);
#define FFCB_BETWEEN_CHAR_DEFINITION(functionName) void functionName(u64 byteIndex, u64 charIndex, v2 position, FontFlowState_t* state, void* context)
typedef FFCB_BETWEEN_CHAR_DEFINITION(FontFlowBetweenCharCallback_f);
#define FFCB_BEFORE_LINE_DEFINITION(functionName) void functionName(u64 lineIndex, u64 byteIndex, FontFlowState_t* state, void* context)
typedef FFCB_BEFORE_LINE_DEFINITION(FontFlowBeforeLineCallback_f);
#define FFCB_AFTER_LINE_DEFINITION(functionName) void functionName(bool isLineWrap, u64 lineIndex, u64 byteIndex, FontFlowState_t* state, void* context)
typedef FFCB_AFTER_LINE_DEFINITION(FontFlowAfterLineCallback_f);

struct FontFlowCallbacks_t
{
	void* context;
	FontFlowBeforeCharCallback_f* beforeChar;
	FontFlowAfterCharCallback_f* afterChar;
	FontFlowBetweenCharCallback_f* betweenChar;
	FontFlowBeforeLineCallback_f* beforeLine;
	FontFlowAfterLineCallback_f* afterLine;
};

struct FontFlowInfo_t
{
	u64 numCharactersRendered; //including invalid chars but not whitespace or control chars
	u64 numInvalidCharacters; //any characters we couldn't find a glyph for get rendered with a purple box
	u64 numControlCharacters; //like \b \a and \f that control bold, italic, and underline respectively
	u64 numWhitespaceCharacters; //includes space and tabs and also any characters that weren't rendered because before char callback returned false
	u64 numNewLineCharacters;
	
	u64 numLines; // = numLineWraps + numNewLineChars (+1 if the final line contains any renderable characters)
	u64 numLineWraps;
	
	rec logicalRec;
	rec renderRec;
	v2 startPos;
	v2 endPos;
};

#endif //  _PIG_FONT_H
