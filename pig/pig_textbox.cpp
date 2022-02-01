/*
File:   pig_textbox.cpp
Author: Taylor Robbins
Date:   01\11\2022
Description: 
	** Holds the functions that help us update and render a Textbox_t
	** The Textbox_t handles text input from the user in single or multi-line format
*/

// +--------------------------------------------------------------+
// |                       Create and Free                        |
// +--------------------------------------------------------------+
void FreeTextbox(Textbox_t* tb)
{
	NotNull(tb);
	if (tb->hintText.pntr != nullptr)
	{
		NotNull(tb->allocArena);
		FreeString(tb->allocArena, &tb->hintText);
	}
	if (tb->text.pntr != nullptr)
	{
		NotNull(tb->allocArena);
		Assert(tb->textAllocLength > 0);
		FreeMem(tb->allocArena, tb->text.pntr, tb->textAllocLength);
	}
	ClearPointer(tb);
}

void CreateTextbox(Textbox_t* tb, MemArena_t* memArena, u64 textBufferSize, bool fixedTextBuffer, bool isMultiline)
{
	AssertSingleThreaded();
	NotNull(tb);
	NotNull(memArena);
	Assert(textBufferSize > 0);
	ClearPointer(tb);
	tb->allocArena = memArena;
	tb->editable = true;
	tb->visible = true;
	tb->hasFocus = false;
	tb->isMultiline = isMultiline;
	tb->fixedTextBuffer = fixedTextBuffer;
	tb->hintText = MyStr_Empty;
	tb->font = &pig->resources.debugFont;
	tb->fontFaceSelector = SelectDefaultFontFace();
	tb->selectionActive = false;
	tb->mouseHovering = false;
	tb->isSelectingWithMouse = false;
	tb->autoSizeHeight = !isMultiline;
	tb->autoSelectOnFocus = false;
	tb->innerMargin = NewVec2(2, 2);
	
	tb->id = pig->nextUiId;
	pig->nextUiId++;
	
	tb->textAllocLength = textBufferSize;
	tb->text.pntr = AllocArray(memArena, char, textBufferSize);
	tb->text.length = 0;
	tb->text.pntr[0] = '\0';
}

// +--------------------------------------------------------------+
// |                           Helpers                            |
// +--------------------------------------------------------------+
void SetTextboxHintText(Textbox_t* tb, MyStr_t hintText)
{
	NotNull(tb);
	NotNull(tb->allocArena);
	FreeString(tb->allocArena, &tb->hintText);
	tb->hintText = AllocString(tb->allocArena, &hintText);
}

void SetTextboxFont(Textbox_t* tb, Font_t* font, FontFaceSelector_t faceSelector)
{
	NotNull(tb);
	NotNull(tb->allocArena);
	NotNull(font);
	tb->font = font;
	tb->fontFaceSelector = faceSelector;
}

// +--------------------------------------------------------------+
// |                   Layout and Mouse Capture                   |
// +--------------------------------------------------------------+
void TextboxLayout(Textbox_t* tb)
{
	NotNull(tb);
	NotNull(tb->font);
	
	FontFlowInfo_t flowInfo;
	TextMeasure_t textMeasure = MeasureTextInFont(tb->text, tb->font, tb->fontFaceSelector, 1.0f, 0.0f, &flowInfo);
	
	if (tb->autoSizeHeight)
	{
		
	}
}

void TextboxCaptureMouse(Textbox_t* tb)
{
	NotNull(tb);
	TextboxLayout(tb);
	if (tb->visible)
	{
		MouseHitRecPrint(tb->mainRec, "Textbox%llu", tb->id);
	}
}

// +--------------------------------------------------------------+
// |                            Update                            |
// +--------------------------------------------------------------+
void UpdateTextbox(Textbox_t* tb)
{
	NotNull(tb);
	
}

// +--------------------------------------------------------------+
// |                            Render                            |
// +--------------------------------------------------------------+
void RenderTextbox(Textbox_t* tb)
{
	NotNull(tb);
	
}
