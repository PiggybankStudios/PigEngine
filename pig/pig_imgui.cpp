/*
File:   pig_imgui.cpp
Author: Taylor Robbins
Date:   03\01\2024
Description: 
	** Holds functions that help us interact with Dear Imgui
*/

// +--------------------------------------------------------------+
// |                      Imgui Source Code                       |
// +--------------------------------------------------------------+
//TODO: Idk where this define is coming from, but it caused problems with code inside imgui source
#undef R

#pragma warning(push)
#pragma warning(disable:4459) //there is a local "gl" variable that our global conflicts with
#include "imgui.cpp"
#include "imgui_draw.cpp"
#include "imgui_tables.cpp"
#include "imgui_widgets.cpp"
//TODO: Currently we have a naming conflict with the "CustomConstraints::Square" function and our Square(x) macro.
//      We need to:
//        * include this cpp file before we define our macro
//        * OR rename our macro
//        * OR make this cpp a separate compilation unit
#undef Square
#include "imgui_demo.cpp"
#define Square(value) ((value) * (value))
#pragma warning(pop)

#include "pig_imgui_shaders.cpp"

// +--------------------------------------------------------------+
// |                          Callbacks                           |
// +--------------------------------------------------------------+
static const char* ImguiCallback_GetClipboardText(void* user_data)
{
	UNUSED(user_data);
	NotNull(pig);
	Assert(pig->imgui.initialized);
	NotNull(plat);
	if (!IsEmptyStr(pig->imgui.clipboardStr)) { FreeString(mainHeap, &pig->imgui.clipboardStr); }
	pig->imgui.clipboardStr = plat->PasteTextFromClipboard(mainHeap);
	return pig->imgui.clipboardStr.chars;
}
static void ImguiCallback_SetClipboardText(void* user_data, const char* text)
{
	UNUSED(user_data);
	NotNull(text);
	NotNull(pig);
	Assert(pig->imgui.initialized);
	NotNull(plat);
	bool copySuccess = plat->CopyTextToClipboard(NewStr(text));
	Assert(copySuccess);
}


static void* ImguiCallback_Alloc(size_t numBytes, void* userData)
{
	//TODO: Put this in it's own arena maybe?
	return AllocMem(&pig->imguiHeap, (u64)numBytes);
}

static void ImguiCallback_Free(void* allocPntr, void* userData)
{
	FreeMem(&pig->imguiHeap, allocPntr, 0, true);
}

int ImFormatString(char* bufferPntr, size_t bufferSize, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	int result = MyVaListPrintf(bufferPntr, bufferSize, fmt, args);
	va_end(args);
	if (bufferPntr == nullptr) { return result; }
	if (result == -1 || result >= (int)bufferSize) { result = (int)(bufferSize - 1); }
	bufferPntr[result] = '\0';
	return result;
}
int ImFormatStringV(char* bufferPntr, size_t bufferSize, const char* fmt, va_list args)
{
	int result = MyVaListPrintf(bufferPntr, bufferSize, fmt, args);
	if (bufferPntr == nullptr) { return result; }
	if (result == -1 || result >= (int)bufferSize) { result = (int)(bufferSize - 1); }
	bufferPntr[result] = '\0';
	return result;
}

// +--------------------------------------------------------------+
// |              Default File Function Replacements              |
// +--------------------------------------------------------------+
ImFileHandle ImFileOpen(const char* filename, const char* mode)
{
	NotNull2(filename, mode);
	MemArena_t* scratch = GetScratchArena();
	
	//NOTE: All imgui files go to the AppData folder, not wherever the .exe and Resources are
	MyStr_t appDataFolder = plat->GetSpecialFolderPath(SpecialFolder_SavesAndSettings, NewStr(PROJECT_NAME_SAFE), scratch);
	MyStr_t fullPath = PrintInArenaStr(scratch, "%.*s/%s", StrPrint(appDataFolder), filename);
	
	MyStr_t modeStr = NewStr(mode);
	bool isForRead = FindNextCharInStr(modeStr, 0, "r");
	bool isForWrite = FindNextCharInStr(modeStr, 0, "w");
	bool isForAppend = FindNextCharInStr(modeStr, 0, "a");
	bool convertNewLines = !FindNextCharInStr(modeStr, 0, "b");
	Assert(isForRead || isForWrite || isForAppend);
	
	ImguiFile_t* result = AllocStruct(&pig->imguiHeap, ImguiFile_t);
	NotNull(result);
	result->convertNewLines = convertNewLines;
	if (isForWrite || isForAppend)
	{
		//TODO: How do we account for convertNewLines when using PlatOpenFile_t??
		result->writing = true;
		bool openSuccess = plat->OpenFile(fullPath, OpenFileMode_Write, true, &result->openFile);
		if (!openSuccess) { FreeMem(&pig->imguiHeap, result, sizeof(ImguiFile_t)); FreeScratchArena(scratch); return nullptr; }
	}
	else
	{
		result->writing = false;
		result->stream = plat->OpenFileStream(fullPath, &pig->imguiHeap, convertNewLines);
		if (!StreamIsValid(&result->stream)) { FreeMem(&pig->imguiHeap, result, sizeof(ImguiFile_t)); FreeScratchArena(scratch); return nullptr; }
	}
	
	FreeScratchArena(scratch);
	return result;
}

bool ImFileClose(ImFileHandle file)
{
	NotNull(file);
	if (file->writing) { plat->CloseFile(&file->openFile); }
	else { FreeStream(&file->stream); }
	FreeMem(&pig->imguiHeap, file, sizeof(ImguiFile_t));
	return true;
}

ImU64 ImFileGetSize(ImFileHandle file)
{
	NotNull(file);
	if (file->writing) { return file->openFile.fileSize; }
	else { return StreamGetSize(&file->stream); }
}

ImU64 ImFileRead(void* data, ImU64 size, ImU64 count, ImFileHandle file)
{
	NotNull(file);
	Assert(!file->writing);
	u64 numBytesRead = StreamReadInto(&file->stream, size * count, data);
	return numBytesRead;
}

ImU64 ImFileWrite(const void* data, ImU64 size, ImU64 count, ImFileHandle file)
{
	NotNull(file);
	Assert(file->writing);
	bool writeSuccess = plat->WriteToFile(&file->openFile, size * count, data, file->convertNewLines);
	return (writeSuccess ? (size * count) : 0);
}

// +--------------------------------------------------------------+
// |                     Imgui Initialization                     |
// +--------------------------------------------------------------+
void PigInitImgui()
{
	NotNull(pig);
	ClearStruct(pig->imgui);
	pig->imgui.frameStarted = false;
	
	InitMemArena_PagedHeapFuncs(&pig->imguiHeap, PIG_IMGUI_ARENA_PAGE_SIZE, PlatAllocFunc, PlatFreeFunc);
	
	PrintLine_I("Initializing ImGui %s", IMGUI_VERSION);
	IMGUI_CHECKVERSION();
	ImGui::SetAllocatorFunctions(ImguiCallback_Alloc, ImguiCallback_Free, nullptr);
	pig->imgui.imguiContext = ImGui::CreateContext();
	NotNull(pig->imgui.imguiContext);
	
	ImGuiIO& imguiIO = ImGui::GetIO();
	
	imguiIO.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	imguiIO.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
	ImGui::StyleColorsDark();
	
	//We will be a custom platform and backend for Imgui
	imguiIO.BackendPlatformName = AllocCharsAndFillNt(&pig->imguiHeap, "Pig Engine");
	imguiIO.BackendRendererName = AllocCharsAndFillNt(&pig->imguiHeap, "Pig Engine");
	imguiIO.BackendPlatformUserData = (void*)&pig->imgui;
	imguiIO.BackendRendererUserData = (void*)&pig->imgui;
	imguiIO.ClipboardUserData = (void*)&pig->imgui;
	imguiIO.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
	imguiIO.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
	imguiIO.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset; //we can do this in OpenGL 3.2+
	imguiIO.SetClipboardTextFn = ImguiCallback_SetClipboardText;
	imguiIO.GetClipboardTextFn = ImguiCallback_GetClipboardText;
	imguiIO.ConfigDebugIsDebuggerPresent = DEBUG_BUILD; //TODO: Can we detect this better?
	imguiIO.IniFilename = AllocCharsAndFillNt(&pig->imguiHeap, IMGUI_INI_FILE_NAME);
	imguiIO.LogFilename = AllocCharsAndFillNt(&pig->imguiHeap, IMGUI_LOG_FILE_NAME);
	
	ImGuiViewport* mainViewport = ImGui::GetMainViewport();
	mainViewport->PlatformHandleRaw = plat->GetNativeWindowPntr(pig->currentWindow);
	
	//TODO: Look for "GL_ARB_clip_control" extension?
	// GLint num_extensions = 0;
	// glGetIntegerv(GL_NUM_EXTENSIONS, &num_extensions);
	// for (GLint i = 0; i < num_extensions; i++)
	// {
	// 	const char* extension = (const char*)glGetStringi(GL_EXTENSIONS, i);
	// }
	
	// +==============================+
	// |     Compile Imgui Shader     |
	// +==============================+
	pig->imgui.mainShaderCode = PrintInArenaStr(mainHeap, "#version 130\n%s\n\n// ====== %s ======\n#version 130\n%s",
		ImguiShaderCode_Vert_GLSL_130,
		FRAGMENT_VERTEX_DIVIDER_STR,
		ImguiShaderCode_Frag_GLSL_130
	);
	NotNullStr(&pig->imgui.mainShaderCode);
	bool imguiShaderCompiledSuccessfully = CreateShader(
		mainHeap,
		&pig->imgui.mainShader,
		pig->imgui.mainShaderCode,
		VertexType_Default2D,
		ShaderUniform_ProjectionMatrix|ShaderUniform_Texture1
	);
	Assert(imguiShaderCompiledSuccessfully);
	
	// +==================================+
	// | Create Imgui Font Atlas Texture  |
	// +==================================+
	u8* fontAtlasPixels;
	int fontAtlasWidth, fontAtlasHeight;
	imguiIO.Fonts->GetTexDataAsRGBA32(&fontAtlasPixels, &fontAtlasWidth, &fontAtlasHeight);
	PlatImageData_t fontAtlasImageData = {};
	fontAtlasImageData.floatChannels = false;
	fontAtlasImageData.pixelSize = sizeof(u32);
	fontAtlasImageData.rowSize = sizeof(u32) * fontAtlasWidth;
	fontAtlasImageData.dataSize = fontAtlasImageData.rowSize * fontAtlasHeight;
	fontAtlasImageData.size = NewVec2i(fontAtlasWidth, fontAtlasHeight);
	fontAtlasImageData.data8 = fontAtlasPixels;
	bool imguiFontTextureCreatedSuccessfully = CreateTexture(
		mainHeap,
		&pig->imgui.fontTexture,
		&fontAtlasImageData,
		false, //pixelated
		false //repeating
	);
Assert(imguiFontTextureCreatedSuccessfully);
	imguiIO.Fonts->SetTexID((ImTextureID)&pig->imgui.fontTexture);
	
	CreateVarArray(&pig->imgui.registeredWindows, mainHeap, sizeof(PigRegisteredImguiWindow_t));
	
	pig->imgui.initialized = true;
}

void PigImguiHandleReload()
{
	IMGUI_CHECKVERSION();
	ImGui::SetAllocatorFunctions(ImguiCallback_Alloc, ImguiCallback_Free, nullptr);
	NotNull(pig->imgui.imguiContext);
	
	// ImGuiSettingsHandler type holds some function pointers which are not safe to keep after the reload
	// Fortunately, they are relatively easy to fixup to new function pointers, since the type information is stored in the TypeHash
	// (We also fixup the TypeName const char* because it was not allocated, so it also went away with the old dll)
	for (ImGuiSettingsHandler& handler : pig->imgui.imguiContext->SettingsHandlers)
	{
		if (handler.TypeHash == ImHashStr("Window"))
		{
			handler.TypeName = "Window";
			handler.ClearAllFn = WindowSettingsHandler_ClearAll;
			handler.ReadOpenFn = WindowSettingsHandler_ReadOpen;
			handler.ReadLineFn = WindowSettingsHandler_ReadLine;
			handler.ApplyAllFn = WindowSettingsHandler_ApplyAll;
			handler.WriteAllFn = WindowSettingsHandler_WriteAll;
		}
		else if (handler.TypeHash == ImHashStr("Table"))
		{
			handler.TypeName = "Table";
			handler.ClearAllFn = TableSettingsHandler_ClearAll;
			handler.ReadOpenFn = TableSettingsHandler_ReadOpen;
			handler.ReadLineFn = TableSettingsHandler_ReadLine;
			handler.ApplyAllFn = TableSettingsHandler_ApplyAll;
			handler.WriteAllFn = TableSettingsHandler_WriteAll;
		}
		else
		{
			AssertMsg(false, "An ImGuiSettingsHandler was registered for an unknown type. We need to perform proper function pointer fixup for this type on reload, but we don't know how!");
		}
	}
	
	ImGui::SetCurrentContext(pig->imgui.imguiContext);
}

// +--------------------------------------------------------------+
// |                     Window Registration                      |
// +--------------------------------------------------------------+
PigRegisteredImguiWindow_t* PigRegisterImguiWindow(MyStr_t name, Func_t renderFunc, void* contextPntr = nullptr)
{
	NotNullStr(&name);
	PigRegisteredImguiWindow_t* newWindow = VarArrayAdd(&pig->imgui.registeredWindows, PigRegisteredImguiWindow_t);
	NotNull(newWindow);
	ClearPointer(newWindow);
	newWindow->name = AllocString(mainHeap, &name);
	newWindow->renderFunc = renderFunc;
	newWindow->contextPntr = contextPntr;
	return newWindow;
}
PigRegisteredImguiWindow_t* PigRegisterImguiWindow(const char* nameNullTerm, Func_t renderFunc, void* contextPntr = nullptr)
{
	return PigRegisterImguiWindow(NewStr(nameNullTerm), renderFunc, contextPntr);
}

// +--------------------------------------------------------------+
// |                         Imgui Update                         |
// +--------------------------------------------------------------+
void PigUpdateImguiBefore()
{
	ImGuiIO& imguiIO = ImGui::GetIO();
	imguiIO.DisplaySize = ImVec2(ScreenSize.width, ScreenSize.height);
	imguiIO.DisplayFramebufferScale = ImVec2(1.0f, 1.0f); //TODO: Fill this with real info! Compare glfwGetFramebufferSize and glfwGetWindowSize?
	imguiIO.DeltaTime = (r32)(ElapsedMs / NUM_MS_PER_SECOND);
	
	if (imguiIO.WantSetMousePos)
	{
		Unimplemented(); //TODO: Implement me!
	}
	
	pig->imgui.isMouseOverImgui = imguiIO.WantCaptureMouse;
	//TODO: Handle imguiIO.WantCaptureKeyboard and imguiIO.WantTextInput
	ImGui::NewFrame();
	pig->imgui.frameStarted = true;
}

void PigImguiHandleInputEventsAndCaptureMouse()
{
	if (pig->imgui.isMouseOverImgui)
	{
		MouseHitNamed("ImGui");
		if (MouseDown(MouseBtn_Left) || MouseReleased(MouseBtn_Left)) { HandleMouse(MouseBtn_Left); }
		if (MouseDown(MouseBtn_Right) || MouseReleased(MouseBtn_Right)) { HandleMouse(MouseBtn_Right); }
		if (MouseDown(MouseBtn_Middle) || MouseReleased(MouseBtn_Middle)) { HandleMouse(MouseBtn_Middle); }
		if (MouseScrolled()) { HandleMouseScroll(); }
	}
	
	if (pig->imgui.launcherIsOpen)
	{
		if (ImGui::Begin("Launcher", &pig->imgui.launcherIsOpen))
		{
			ImGui::Text("Windows (%llu)", pig->imgui.registeredWindows.length);
			VarArrayLoop(&pig->imgui.registeredWindows, wIndex)
			{
				VarArrayLoopGet(PigRegisteredImguiWindow_t, registeredWindow, &pig->imgui.registeredWindows, wIndex);
				if (ImGui::Button(registeredWindow->name.chars))
				{
					registeredWindow->isOpen = !registeredWindow->isOpen;
				}
			}
		}
		ImGui::End();
	}
	
	VarArrayLoop(&pig->imgui.registeredWindows, wIndex)
	{
		VarArrayLoopGet(PigRegisteredImguiWindow_t, registeredWindow, &pig->imgui.registeredWindows, wIndex);
		if (registeredWindow->isOpen)
		{
			CallFunc(ImguiWindowRenderFunc_f, registeredWindow->renderFunc, registeredWindow);
		}
	}
}

void PigUpdateImguiAfter()
{
	ImGuiIO& imguiIO = ImGui::GetIO();
	
	if (pig->currentWindow->input.isFocusedChanged)
	{
		imguiIO.AddFocusEvent(pig->currentWindow->input.isFocused);
	}
	
	imguiIO.AddMousePosEvent(MousePos.x, MousePos.y);
	if (!IsMouseOverAnything() || IsMouseOverNamed("ImGui"))
	{
		if (MousePressedRaw(MouseBtn_Left))    { imguiIO.AddMouseButtonEvent(0, true);  }
		if (MouseReleasedRaw(MouseBtn_Left))   { imguiIO.AddMouseButtonEvent(0, false); }
		if (MousePressedRaw(MouseBtn_Right))   { imguiIO.AddMouseButtonEvent(1, true);  }
		if (MouseReleasedRaw(MouseBtn_Right))  { imguiIO.AddMouseButtonEvent(1, false); }
		if (MousePressedRaw(MouseBtn_Middle))  { imguiIO.AddMouseButtonEvent(2, true);  }
		if (MouseReleasedRaw(MouseBtn_Middle)) { imguiIO.AddMouseButtonEvent(2, false); }
		if (MouseScrolledRaw()) { imguiIO.AddMouseWheelEvent(pigIn->scrollDelta.x, pigIn->scrollDelta.y); }
		
		//TODO: Somehow we need to know if the mouse is over an imgui window. If so, we should handle the mouse buttons, and capture the mouse
	}
	
	if (!IsFocusedItemTyping())
	{
		for (u64 keyIndex = 0; keyIndex < Key_NumKeys; keyIndex++)
		{
			Key_t key = (Key_t)keyIndex;
			ImGuiKey imguiKey = GetImGuiKey(key);
			if (imguiKey != ImGuiKey_None)
			{
				if (KeyPressed(key)) { imguiIO.AddKeyEvent(imguiKey, true); }
				if (KeyReleased(key)) { imguiIO.AddKeyEvent(imguiKey, false); }
			}
		}
		
		VarArrayLoop(&pigIn->inputEvents, eIndex)
		{
			VarArrayLoopGet(InputEvent_t, inputEvent, &pigIn->inputEvents, eIndex);
			if (inputEvent->type == InputEventType_Character)
			{
				imguiIO.AddInputCharacter(inputEvent->character.codepoint);
			}
		}
		
		//TODO: Somehow we need to know if an imgui EditText is in focus, if so we should handle keys
	}
	
	for (i32 cIndex = 0; cIndex < MAX_NUM_CONTROLLERS; cIndex++)
	{
		const PlatControllerState_t* controller = &pigIn->controllerStates[cIndex];
		if (controller->connected)
		{
			for (u64 btnIndex = 0; btnIndex < ControllerBtn_NumBtns; btnIndex++)
			{
				ControllerBtn_t btn = (ControllerBtn_t)btnIndex;
				ImGuiKey imguiKey = GetImGuiKey(btn);
				if (imguiKey != ImGuiKey_None)
				{
					if (ControllerBtnPressed(cIndex, btn)) { imguiIO.AddKeyEvent(imguiKey, true); }
					if (ControllerBtnReleased(cIndex, btn)) { imguiIO.AddKeyEvent(imguiKey, false); }
				}
			}
		}
		
		//TODO: We should probably route analog data from the controller to Imgui with imguiIO.AddKeyAnalogEvent
	}
	
	ImGuiMouseCursor imguiCursor = ImGui::GetMouseCursor();
	if (imguiCursor == ImGuiMouseCursor_Hand) { pigOut->cursorType = PlatCursor_Pointer; }
	else if (imguiCursor == ImGuiMouseCursor_TextInput) { pigOut->cursorType = PlatCursor_TextIBeam; }
	else if (imguiCursor == ImGuiMouseCursor_ResizeNS) { pigOut->cursorType = PlatCursor_ResizeVertical; }
	else if (imguiCursor == ImGuiMouseCursor_ResizeEW) { pigOut->cursorType = PlatCursor_ResizeHorizontal; }
	else if (imguiCursor == ImGuiMouseCursor_NotAllowed) { } //TODO: Implement me!
	else if (imguiCursor == ImGuiMouseCursor_ResizeNESW || imguiCursor == ImGuiMouseCursor_ResizeNWSE || imguiCursor == ImGuiMouseCursor_ResizeAll)
	{
		//TODO: Make a PlatCursor option for this!
		pigOut->cursorType = PlatCursor_ResizeHorizontal;
	}
}

// +--------------------------------------------------------------+
// |                         Imgui Render                         |
// +--------------------------------------------------------------+
void PigRenderImgui()
{
	ImGui::Render();
	pig->imgui.frameStarted = false;
	ImDrawData* imDrawData = ImGui::GetDrawData();
	RcRenderImDrawData(imDrawData);
}
