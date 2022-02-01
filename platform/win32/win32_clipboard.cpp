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
	HGLOBAL globalCopy = GlobalAlloc(GMEM_MOVEABLE, (SIZE_T)text.length); 
	if (globalCopy == nullptr)
	{
		WriteLine_E("Couldn't allocate space for clipboard data!");
		return false;
	}
	
	u8* lockPntr = (u8*)GlobalLock(globalCopy);
	MyMemCopy(lockPntr, text.pntr, text.length);
	GlobalUnlock(globalCopy);
	
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
	
	SetClipboardData(CF_TEXT, globalCopy);
	
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
			result = ConvertWideStrToUtf8(memArena, wideStr, wideStrLength);
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

