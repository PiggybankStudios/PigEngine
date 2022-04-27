/*
File:   web_main.h
Author: Taylor Robbins
Date:   03\18\2022
*/

#ifndef _WEB_MAIN_H
#define _WEB_MAIN_H

struct Line_t
{
	v2 start;
	v2 end;
};

struct Web_PlatformState_t
{
	bool initialized;
	MemArena_t mainHeap;
	MemArena_t tempArena;
	
	StartupInfo_t startupInfo;
	StartupOptions_t startupOptions;
	PlatformInfo_t info;
	PlatformApi_t api;
	EngineMemory_t engineMemory;
	EngineInput_t enginePreviousInput;
	EngineInput_t engineInput;
	EngineInput_t engineActiveInput;
	EngineOutput_t engineOutput;
	
	r32 loadingPercent;
	
	PlatCursor_t currentCursorType;
	
	//Debug Output
	LinkedList_t debugReadoutLines;
	
	Shader_t testShader;
	VertBuffer_t squareBuffer;
	VertexArrayObject_t testVao;
	
	// TODO: Remove the stuff below this
	v2 testPos;
	VarArray_t drawnLines; //Line_t
	
	v2 mousePos;
	v2 prevMousePos;
	bool mouseMoved;
	
	bool mouseLeftBtnDown;
	bool mouseMiddleBtnDown;
	bool mouseRightBtnDown;
	
	bool mouseLeftBtnPressed;
	bool mouseMiddleBtnPressed;
	bool mouseRightBtnPressed;
	
	bool mouseLeftBtnReleased;
	bool mouseMiddleBtnReleased;
	bool mouseRightBtnReleased;
};

#endif //  _WEB_MAIN_H
