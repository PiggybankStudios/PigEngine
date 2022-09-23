/*
File:   pig_confirm_dialog.cpp
Author: Taylor Robbins
Date:   09\20\2022
Description: 
	** Holds functions that run the Confirmation Dialog that shows a message and
	** allows the user to click 1, 2, or more buttons (Often "yes" and "no" or similar)
*/

//TODO: Make the font for dialogs selectable
//TODO: Make the buttons look a bit nicer?
//TODO: Add controller support?


// +--------------------------------------------------------------+
// |                        Free Function                         |
// +--------------------------------------------------------------+
void FreeConfirmDialog(ConfirmDialog_t* dialog)
{
	NotNull(dialog);
	if (dialog->messageStr.pntr != nullptr)
	{
		NotNull(dialog->allocArena);
		FreeString(dialog->allocArena, &dialog->messageStr);
	}
	for (u64 bIndex = 0; bIndex < dialog->numButtons; bIndex++)
	{
		ConfirmDialogBtn_t* button = &dialog->buttons[bIndex];
		FreeButton(&button->btn);
	}
	ClearPointer(dialog);
}

// +--------------------------------------------------------------+
// |                        Get Functions                         |
// +--------------------------------------------------------------+
ConfirmDialog_t* GetConfirmDialogById(u64 dialogId)
{
	VarArrayLoop(&pig->confirmDialogs, dIndex)
	{
		VarArrayLoopGet(ConfirmDialog_t, dialog, &pig->confirmDialogs, dIndex);
		if (dialog->id == dialogId) { return dialog; }
	}
	return nullptr;
}
ConfirmDialogBtn_t* GetConfirmDialogBtn(ConfirmDialog_t* dialog, u64 buttonValue)
{
	for (u64 bIndex = 0; bIndex < dialog->numButtons; bIndex++)
	{
		if (dialog->buttons[bIndex].value == buttonValue) { return &dialog->buttons[bIndex]; }
	}
	return nullptr;
}
bool IsConfirmDialogDone(u64 dialogId, bool resultOnFailure = true)
{
	ConfirmDialog_t* dialog = GetConfirmDialogById(dialogId);
	if (dialog == nullptr) { return resultOnFailure; }
	return dialog->buttonSelected;
}
bool GetConfirmDialogResult(u64 dialogId, u64* resultOut, u64 resultOnFailure = 0)
{
	ConfirmDialog_t* dialog = GetConfirmDialogById(dialogId);
	if (dialog == nullptr)
	{
		if (resultOut != nullptr) { *resultOut = resultOnFailure; }
		return true;
	}
	if (dialog->buttonSelected)
	{
		Assert(dialog->buttonSelectIndex < dialog->numButtons);
		if (resultOut != nullptr) { *resultOut = dialog->buttonSelectValue; }
		return true;
	}
	return false;
}

bool IsDialogOpen()
{
	VarArrayLoop(&pig->confirmDialogs, dIndex)
	{
		VarArrayLoopGet(ConfirmDialog_t, dialog, &pig->confirmDialogs, dIndex);
		if (dialog->isOpen)
		{
			return true;
		}
	}
	return false;
}

// +--------------------------------------------------------------+
// |                       Dismiss Function                       |
// +--------------------------------------------------------------+
void DisposeConfirmDialog(ConfirmDialog_t* dialog)
{
	NotNull(dialog);
	Assert(VarArrayContains(&pig->confirmDialogs, dialog));
	u64 dialogIndex = 0;
	VarArrayGetIndexOf(&pig->confirmDialogs, dialog, &dialogIndex, ConfirmDialog_t);
	FreeConfirmDialog(dialog);
	VarArrayRemove(&pig->confirmDialogs, dialogIndex, ConfirmDialog_t);
}

// +--------------------------------------------------------------+
// |                   Alloc and Show Functions                   |
// +--------------------------------------------------------------+
ConfirmDialog_t* AllocConfirmDialog(MyStr_t message, MemArena_t* memArena)
{
	AssertSingleThreaded();
	NotNullStr(&message);
	NotNull(memArena);
	ConfirmDialog_t* newDialog = VarArrayAdd(&pig->confirmDialogs, ConfirmDialog_t);
	NotNull(newDialog);
	ClearPointer(newDialog);
	newDialog->id = pig->nextConfirmDialogId;
	pig->nextConfirmDialogId++;
	newDialog->allocArena = memArena;
	newDialog->messageStr = AllocString(memArena, &message);
	NotNullStr(&newDialog->messageStr);
	newDialog->escapeButtonIndex = -1;
	newDialog->allowClickOutsideToCloseDialog = true;
	newDialog->numButtons = 0;
	return newDialog;
}

ConfirmDialogBtn_t* AddConfirmDialogBtn(ConfirmDialog_t* dialog, MyStr_t buttonDisplayStr, u64 buttonValue)
{
	NotNull(dialog);
	NotNullStr(&buttonDisplayStr);
	NotNull(dialog->allocArena);
	Assert(dialog->numButtons < CONFIRM_DIALOG_MAX_NUM_BUTTONS);
	ConfirmDialogBtn_t* newButton = &dialog->buttons[dialog->numButtons];
	dialog->numButtons++;
	ClearPointer(newButton);
	newButton->index = dialog->numButtons-1;
	newButton->value = buttonValue;
	CreateButton(&newButton->btn, dialog->allocArena, CONFIRM_DIALOG_BTN_STYLE, buttonDisplayStr);
	SetButtonFont(&newButton->btn, &pig->resources.fonts->large, SelectDefaultFontFace());
	ButtonSetMouseCapturePrefix(&newButton->btn, TempPrintStr("ConfirmDialog%lluButton", dialog->id));
	newButton->btn.backColor = ColorTransparent(CONFIRM_DIALOG_BTN_NO_COLOR, 0.7f);
	newButton->btn.textColor = CONFIRM_DIALOG_BTN_TEXT_COLOR;
	newButton->btn.borderColor = CONFIRM_DIALOG_BTN_BORDER_COLOR;
	newButton->btn.cornerRadius = CONFIRM_DIALOG_BTN_CORNER_RADIUS;
	newButton->btn.borderThickness = CONFIRM_DIALOG_BTN_BORDER_THICKNESS;
	return newButton;
}

void ConfirmDialogSetAutoDispose(u64 dialogId, bool autoDispose)
{
	ConfirmDialog_t* dialog = GetConfirmDialogById(dialogId);
	NotNull(dialog);
	dialog->autoDispose = autoDispose;
}
void ConfirmDialogSetButtonColor(u64 dialogId, u64 buttonValue, Color_t backColor, Color_t textColor, Color_t borderColor)
{
	ConfirmDialog_t* dialog = GetConfirmDialogById(dialogId);
	NotNull(dialog);
	ConfirmDialogBtn_t* button = GetConfirmDialogBtn(dialog, buttonValue);
	AssertMsg(button != nullptr, "Tried to set color of button that doesn't exist by that value");
	button->btn.backColor = backColor;
	button->btn.textColor = textColor;
	button->btn.borderColor = borderColor;
}
void ConfirmDialogSetEscapeBtn(u64 dialogId, u64 buttonValue)
{
	ConfirmDialog_t* dialog = GetConfirmDialogById(dialogId);
	NotNull(dialog);
	ConfirmDialogBtn_t* button = GetConfirmDialogBtn(dialog, buttonValue);
	AssertMsg(button != nullptr, "Tried to set confirm dialog EscapeButton to buttonValue that doesn't exist");
	dialog->escapeButtonIndex = (i64)button->index;
}
void ConfirmDialogBegin(u64 dialogId)
{
	ConfirmDialog_t* dialog = GetConfirmDialogById(dialogId);
	NotNull(dialog);
	dialog->isOpen = true;
}

u64 ShowDialog2(MyStr_t message, MyStr_t leftBtnStr, u64 leftBtnValue, MyStr_t rightBtnStr, u64 rightBtnValue, ConfirmDialogCallback_f* callback = nullptr, void* callbackContext = nullptr)
{
	ConfirmDialog_t* result = AllocConfirmDialog(message, mainHeap);
	NotNull(result);
	ConfirmDialogBtn_t* leftBtn = AddConfirmDialogBtn(result, leftBtnStr, leftBtnValue);
	UNUSED(leftBtn);
	ConfirmDialogBtn_t* rightBtn = AddConfirmDialogBtn(result, rightBtnStr, rightBtnValue);
	UNUSED(rightBtn);
	result->callback = callback;
	result->callbackContext = callbackContext;
	return result->id;
}
u64 ShowYesNoDialog(MyStr_t message, bool yesOnRight, ConfirmDialogCallback_f* callback = nullptr, void* callbackContext = nullptr)
{
	u64 dialogId = ShowDialog2(
		message,
		(yesOnRight ? NewStr("No") : NewStr("Yes")), (yesOnRight ? 0 : 1),
		(yesOnRight ? NewStr("Yes") : NewStr("No")), (yesOnRight ? 1 : 0),
		callback, callbackContext
	);
	ConfirmDialogSetButtonColor(dialogId, 0, CONFIRM_DIALOG_BTN_NO_COLOR,  CONFIRM_DIALOG_BTN_TEXT_COLOR, CONFIRM_DIALOG_BTN_BORDER_COLOR);
	ConfirmDialogSetButtonColor(dialogId, 1, CONFIRM_DIALOG_BTN_YES_COLOR, CONFIRM_DIALOG_BTN_TEXT_COLOR, CONFIRM_DIALOG_BTN_BORDER_COLOR);
	ConfirmDialogSetEscapeBtn(dialogId, 0);
	return dialogId;
}
u64 ShowCustomAndCancelDialog(MyStr_t message, MyStr_t confirmBtnStr, u64 confirmBtnValue, bool confirmOnRight, ConfirmDialogCallback_f* callback = nullptr, void* callbackContext = nullptr)
{
	Assert(confirmBtnValue != 0);
	u64 dialogId = ShowDialog2(
		message,
		(confirmOnRight ? NewStr("No") : confirmBtnStr), (confirmOnRight ? 0 : confirmBtnValue),
		(confirmOnRight ? confirmBtnStr : NewStr("No")), (confirmOnRight ? confirmBtnValue : 0),
		callback, callbackContext
	);
	ConfirmDialogSetButtonColor(dialogId, 0,               CONFIRM_DIALOG_BTN_NO_COLOR,  CONFIRM_DIALOG_BTN_TEXT_COLOR, CONFIRM_DIALOG_BTN_BORDER_COLOR);
	ConfirmDialogSetButtonColor(dialogId, confirmBtnValue, CONFIRM_DIALOG_BTN_YES_COLOR, CONFIRM_DIALOG_BTN_TEXT_COLOR, CONFIRM_DIALOG_BTN_BORDER_COLOR);
	ConfirmDialogSetEscapeBtn(dialogId, 0);
	return dialogId;
}

// +--------------------------------------------------------------+
// |                          Initialize                          |
// +--------------------------------------------------------------+
void PigInitConfirmDialogs()
{
	pig->nextConfirmDialogId = 1;
	CreateVarArray(&pig->confirmDialogs, mainHeap, sizeof(ConfirmDialog_t));
}

// +--------------------------------------------------------------+
// |                   Layout and CaptureMouse                    |
// +--------------------------------------------------------------+
void ConfirmDialogLayout(ConfirmDialog_t* dialog)
{
	NotNull(dialog);
	
	dialog->messageMeasure = MeasureTextInFont(dialog->messageStr, &pig->resources.fonts->large, SelectDefaultFontFace());
	dialog->mainRec.width = dialog->messageMeasure.size.width + 2*CONFIRM_DIALOG_TEXT_MARGIN_LEFTRIGHT;
	if (dialog->mainRec.width > CONFIRM_DIALOG_MAX_WIDTH)
	{
		dialog->mainRec.width = CONFIRM_DIALOG_MAX_WIDTH;
		r32 maxWidth = dialog->mainRec.width - 2*CONFIRM_DIALOG_TEXT_MARGIN_LEFTRIGHT;
		dialog->messageMeasure = MeasureTextInFont(dialog->messageStr, &pig->resources.fonts->large, SelectDefaultFontFace(), 1.0f, maxWidth);
	}
	dialog->mainRec.height = dialog->messageMeasure.size.height + CONFIRM_DIALOG_TEXT_MARGIN_TOP + CONFIRM_DIALOG_TEXT_MARGIN_BOTTOM;
	
	dialog->buttonsRec = NewRec(CONFIRM_DIALOG_BTN_MARGIN_LEFTRIGHT, dialog->mainRec.height, dialog->mainRec.width - 2*CONFIRM_DIALOG_BTN_MARGIN_LEFTRIGHT, CONFIRM_DIALOG_BTN_MIN_HEIGHT);
	if (dialog->numButtons > 0)
	{
		rec buttonsBaseRec = dialog->buttonsRec;
		buttonsBaseRec.width -= CONFIRM_DIALOG_BTN_PADDING * (dialog->numButtons-1);
		buttonsBaseRec.width = buttonsBaseRec.width / (r32)dialog->numButtons;
		r32 maxButtonHeight = 0;
		for (u64 bIndex = 0; bIndex < dialog->numButtons; bIndex++)
		{
			ConfirmDialogBtn_t* button = &dialog->buttons[bIndex];
			button->btn.autoSizeHeight = true;
			button->btn.innerMargin = NewVec2(CONFIRM_DIALOG_BTN_INNER_MARGIN, CONFIRM_DIALOG_BTN_INNER_MARGIN);
			ButtonMove(&button->btn, buttonsBaseRec);
			ButtonLayout(&button->btn);
			if (maxButtonHeight < button->btn.mainRec.height) { maxButtonHeight = button->btn.mainRec.height; }
			button->btn.autoSizeHeight = false;
		}
		if (dialog->buttonsRec.height < maxButtonHeight) { dialog->buttonsRec.height = maxButtonHeight; }
	}
	
	dialog->mainRec.height += dialog->buttonsRec.height + CONFIRM_DIALOG_BTN_MARGIN_BOTTOM;
	if (dialog->mainRec.width < CONFIRM_DIALOG_MIN_WIDTH) { dialog->mainRec.width = CONFIRM_DIALOG_MIN_WIDTH; }
	if (dialog->mainRec.height < CONFIRM_DIALOG_MIN_HEIGHT) { dialog->mainRec.height = CONFIRM_DIALOG_MIN_HEIGHT; }
	
	dialog->buttonsRec.y = dialog->mainRec.height - CONFIRM_DIALOG_BTN_MARGIN_BOTTOM - dialog->buttonsRec.height;
	RecAlign(&dialog->buttonsRec);
	
	if (dialog->numButtons > 0)
	{
		rec buttonsBaseRec = dialog->buttonsRec;
		buttonsBaseRec.width -= CONFIRM_DIALOG_BTN_PADDING * (dialog->numButtons-1);
		buttonsBaseRec.width = buttonsBaseRec.width / (r32)dialog->numButtons;
		for (u64 bIndex = 0; bIndex < dialog->numButtons; bIndex++)
		{
			ConfirmDialogBtn_t* button = &dialog->buttons[bIndex];
			rec btnRec = NewRec(
				buttonsBaseRec.x + bIndex * (buttonsBaseRec.width + CONFIRM_DIALOG_BTN_PADDING),
				buttonsBaseRec.y,
				buttonsBaseRec.width,
				buttonsBaseRec.height
			);
			RecAlign(&btnRec);
			ButtonMove(&button->btn, btnRec);
		}
	}
	
	// if (dialog->openAnimProgress < 1.0f)
	// {
	// 	dialog->mainRec.width *= EaseQuadraticOut(dialog->openAnimProgress);
	// }
	dialog->mainRec.topLeft = ScreenSize/2 - dialog->mainRec.size/2;
	RecAlign(&dialog->mainRec);
	
	dialog->contentRec.x = CONFIRM_DIALOG_TEXT_MARGIN_LEFTRIGHT;
	dialog->contentRec.y = CONFIRM_DIALOG_TEXT_MARGIN_TOP;
	dialog->contentRec.width = dialog->mainRec.width - 2*CONFIRM_DIALOG_TEXT_MARGIN_LEFTRIGHT;
	dialog->contentRec.height = dialog->buttonsRec.y - CONFIRM_DIALOG_TEXT_MARGIN_BOTTOM - CONFIRM_DIALOG_TEXT_MARGIN_TOP;
	RecAlign(&dialog->contentRec);
	
	dialog->messageRec.size = dialog->messageMeasure.size;
	dialog->messageRec.topLeft = dialog->contentRec.topLeft + dialog->contentRec.size/2 - dialog->messageRec.size/2;
	RecAlign(&dialog->messageRec);
}

void ConfirmDialogCaptureMouse(ConfirmDialog_t* dialog)
{
	NotNull(dialog);
	if (dialog->openAnimProgress > 0.0f)
	{
		if (dialog->openAnimProgress >= CONFIRM_DIALOG_CONTENT_APPEAR_OPEN_PERCENT)
		{
			for (u64 bIndex = 0; bIndex < dialog->numButtons; bIndex++)
			{
				ConfirmDialogBtn_t* button = &dialog->buttons[bIndex];
				ButtonCaptureMouse(&button->btn, dialog->mainRec.topLeft);
			}
		}
		MouseHitRecPrint(dialog->mainRec, "ConfirmDialog%llu", dialog->id);
		if (dialog->isOpen)
		{
			MouseHitPrint("OutsideConfirmDialog%llu", dialog->id);
		}
	}
}

// +--------------------------------------------------------------+
// |                       Update Function                        |
// +--------------------------------------------------------------+
void PigCaptureMouseConfirmDialogs()
{
	VarArrayLoop(&pig->confirmDialogs, dIndex)
	{
		VarArrayLoopGet(ConfirmDialog_t, dialog, &pig->confirmDialogs, dIndex);
		ConfirmDialogLayout(dialog);
		ConfirmDialogCaptureMouse(dialog);
	}
}
void PigUpdateConfirmDialogs()
{
	//Auto Dispose Dialogs
	for (u64 dIndex = 0; dIndex < pig->confirmDialogs.length; )
	{
		VarArrayLoopGet(ConfirmDialog_t, dialog, &pig->confirmDialogs, dIndex);
		if (!dialog->isOpen && dialog->openAnimProgress == 0.0f && dialog->autoDispose)
		{
			PrintLine_D("Auto-disposing dialog[%llu] which had result %llu", dIndex, dialog->buttonSelectValue);
			DisposeConfirmDialog(dialog);
		}
		else { dIndex++; }
	}
	
	VarArrayLoop(&pig->confirmDialogs, dIndex)
	{
		VarArrayLoopGet(ConfirmDialog_t, dialog, &pig->confirmDialogs, dIndex);
		
		// +===============================+
		// | Handle Escape to Close Dialog |
		// +===============================+
		if (dialog->isOpen && dialog->escapeButtonIndex >= 0)
		{
			if (KeyPressed(Key_Escape))
			{
				HandleKeyExtended(Key_Escape);
				Assert((u64)dialog->escapeButtonIndex < dialog->numButtons);
				dialog->buttons[dialog->escapeButtonIndex].btn.clicked = true;
			}
		}
		
		// +======================================+
		// | Handle Click Outside to Close Dialog |
		// +======================================+
		if (dialog->isOpen && dialog->escapeButtonIndex >= 0 && dialog->openAnimProgress >= 1.0f &&
			!IsMouseOverNamedPartial("ConfirmDialog%llu", dialog->id))
		{
			if (MousePressedAndHandleExtended(MouseBtn_Left))
			{
				Assert((u64)dialog->escapeButtonIndex < dialog->numButtons);
				dialog->buttons[dialog->escapeButtonIndex].btn.clicked = true;
			}
		}
		
		// +==================================+
		// | Handle Mouse Clicked on Buttons  |
		// +==================================+
		if (dialog->isOpen)
		{
			for (u64 bIndex = 0; bIndex < dialog->numButtons; bIndex++)
			{
				ConfirmDialogBtn_t* button = &dialog->buttons[bIndex];
				button->btn.isVisible = (dialog->openAnimProgress >= CONFIRM_DIALOG_CONTENT_APPEAR_OPEN_PERCENT);
				button->btn.opacity = EaseQuadraticOut(SubAnimAmountR32(dialog->openAnimProgress, CONFIRM_DIALOG_CONTENT_APPEAR_OPEN_PERCENT, 1.0f));
				
				UpdateButton(&button->btn, dialog->mainRec.topLeft);
				
				if (button->btn.clicked)
				{
					button->btn.clicked = false;
					dialog->buttonSelected = true;
					dialog->buttonSelectIndex = bIndex;
					dialog->buttonSelectValue = button->value;
					dialog->isOpen = false;
					if (dialog->callback != nullptr)
					{
						dialog->callback(dialog, button, button->value, dialog->callbackContext);
					}
				}
			}
		}
		
		// +==============================+
		// |       Update OpenAnim        |
		// +==============================+
		if (dialog->isOpen && dialog->openAnimProgress < 1.0f)
		{
			UpdateAnimationUp(&dialog->openAnimProgress, CONFIRM_DIALOG_OPEN_ANIM_TIME);
		}
		if (!dialog->isOpen && dialog->openAnimProgress > 0.0f)
		{
			UpdateAnimationDown(&dialog->openAnimProgress, CONFIRM_DIALOG_CLOSE_ANIM_TIME);
		}
	}
}

// +--------------------------------------------------------------+
// |                       Render Function                        |
// +--------------------------------------------------------------+
void PigRenderConfirmDialogs()
{
	RcBindFont(&pig->resources.fonts->large, SelectDefaultFontFace());
	for (u64 dIndex = pig->confirmDialogs.length; dIndex > 0; dIndex--)
	{
		VarArrayLoopGet(ConfirmDialog_t, dialog, &pig->confirmDialogs, dIndex-1);
		if (dialog->openAnimProgress > 0.0f)
		{
			ConfirmDialogLayout(dialog);
			
			rec mainRec = dialog->mainRec;
			mainRec.x += mainRec.width/2;
			mainRec.width *= EaseQuadraticOut(dialog->openAnimProgress);
			mainRec.x -= mainRec.width/2;
			RecAlign(&mainRec);
			Color_t outlineColor = NewColor(0xFFDA8F01);
			Color_t redBtnColor = NewColor(0xFF6D0409);
			RcDrawQuarterCircleFoldedRec(
				mainRec,
				CONFIRM_DIALOG_CORNER_CIRCLE_RADIUS,
				CONFIRM_DIALOG_CORNER_CIRCLE_MARGIN,
				CONFIRM_DIALOG_OUTLINE_THICKNESS,
				CONFIRM_DIALOG_CORNER_CIRCLE_OUTLINE_THICKNESS,
				ColorTransparent(Black, 0.75f), ColorTransparent(Black, 0.75f),
				outlineColor, outlineColor
			);
			
			if (dialog->openAnimProgress > CONFIRM_DIALOG_CONTENT_APPEAR_OPEN_PERCENT)
			{
				// +==============================+
				// |        Render Message        |
				// +==============================+
				r32 messageOpacity = EaseQuadraticOut(SubAnimAmountR32(dialog->openAnimProgress, CONFIRM_DIALOG_CONTENT_APPEAR_OPEN_PERCENT, 1.0f));
				v2 messagePos = dialog->messageRec.topLeft + NewVec2(dialog->messageRec.width/2, dialog->messageMeasure.offset.y) + dialog->mainRec.topLeft;
				Vec2Align(&messagePos);
				r32 messageMaxWidth = dialog->mainRec.width - 2*CONFIRM_DIALOG_TEXT_MARGIN_LEFTRIGHT;
				Color_t messageColor = MonokaiWhite;
				RcDrawText(dialog->messageStr, messagePos, ColorTransparent(messageColor, messageOpacity), TextAlignment_Center, messageMaxWidth);
				
				// +==============================+
				// |        Render Buttons        |
				// +==============================+
				for (u64 bIndex = 0; bIndex < dialog->numButtons; bIndex++)
				{
					ConfirmDialogBtn_t* button = &dialog->buttons[bIndex];
					RenderButton(&button->btn, dialog->mainRec.topLeft);
				}
			}
		}
	}
}
