/*
File:   win32_clipboard.cpp
Author: Taylor Robbins
Date:   01\04\2022
Description: 
	** Holds functions that allow the application (and platform layer) to copy information
	** to and from the clipboard of the operating system
*/

// +==============================+
// |  Win32_CopyTextToClipboard   |
// +==============================+
// bool CopyTextToClipboard(MyStr_t text)
COPY_TEXT_TO_CLIPBOARD_DEFINITION(Win32_CopyTextToClipboard)
{
	NotNullStr(&text);
	
	u64 dataSize = text.length+1;
	void* dataPntr = text.chars;
	bool isDataWide = false;
	if (DoesStrContainMultibyteUtf8Characters(text))
	{
		PushMemMark(GetTempArena());
		MyWideStr_t wideStr = ConvertUtf8StrToUcs2(GetTempArena(), text);
		dataPntr = wideStr.chars;
		dataSize = wideStr.length * sizeof(wchar_t);
		isDataWide = true;
	}
	
	HGLOBAL globalCopy = GlobalAlloc(GMEM_MOVEABLE, (SIZE_T)dataSize + (isDataWide ? sizeof(wchar_t) : sizeof(char))); 
	if (globalCopy == nullptr)
	{
		WriteLine_E("Couldn't allocate space for clipboard data!");
		if (isDataWide) { PopMemMark(GetTempArena()); }
		return false;
	}
	
	u8* lockPntr = (u8*)GlobalLock(globalCopy);
	MyMemCopy(lockPntr, dataPntr, dataSize);
	MyMemSet(&lockPntr[dataSize], 0x00, (isDataWide ? sizeof(wchar_t) : sizeof(char)));
	GlobalUnlock(globalCopy);
	if (isDataWide) { PopMemMark(GetTempArena()); }
	
	if (OpenClipboard(Platform->windowHandle) == false)
	{
		WriteLine_E("Couldn't open clipboard!");
		HGLOBAL freeResult = GlobalFree(globalCopy);
		Assert(freeResult != NULL);
		return false;
	}
	if (EmptyClipboard() == false)
	{
		WriteLine_E("Could not empty current clipboard contents!");
		CloseClipboard();
		return false;
	}
	
	SetClipboardData((isDataWide ? CF_UNICODETEXT : CF_TEXT), globalCopy);
	
	CloseClipboard();
	return true;
}

// +==============================+
// | Win32_PasteTextFromClipboard |
// +==============================+
// MyStr_t PasteTextFromClipboard(MemArena_t* memArena)
PASTE_TEXT_FROM_CLIPBOARD_DEFINITION(Win32_PasteTextFromClipboard)
{
	NotNull(memArena);
	MyStr_t result = MyStr_Empty;
	
	if (OpenClipboard(Platform->windowHandle))
	{
		HANDLE dataHandle = GetClipboardData(CF_UNICODETEXT);
		if (dataHandle != nullptr)
		{
			wchar_t* wideStr = (wchar_t*)GlobalLock(dataHandle);
			NotNull(wideStr);
			u64 wideStrLength = MyWideStrLength(wideStr);
			result = ConvertUcs2StrToUtf8(memArena, wideStr, wideStrLength);
			NotNullStr(&result);
			GlobalUnlock(dataHandle);
		}
		else
		{
			WriteLine_E("Clipboard did not contain CF_UNICODETEXT data");
		}
		
		CloseClipboard();
	}
	else
	{
		WriteLine_E("Couldn't open clipboard");
	}
	
	return result;
}

