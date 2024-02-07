/*
File:   pig_graphics_helpers.cpp
Author: Taylor Robbins
Date:   02\06\2024
Description: 
	** Holds all the implementations for functions that are common to all graphics platforms
*/

const char* GetRenderApiStr(RenderApi_t enumValue)
{
	switch (enumValue)
	{
		case RenderApi_None:   return "None";
		case RenderApi_OpenGL: return "OpenGL";
		case RenderApi_WebGL:  return "WebGL";
		case RenderApi_Vulkan: return "Vulkan";
		case RenderApi_D3D11:  return "D3D11";
		case RenderApi_D3D12:  return "D3D12";
		case RenderApi_Metal:  return "Metal";
		default: return "Unknown";
	}
}

const char* GetOpenGlProfileStr(OpenGlProfile_t enumValue)
{
	switch (enumValue)
	{
		case OpenGlProfile_None:   return "None";
		case OpenGlProfile_Any:    return "Any";
		case OpenGlProfile_Core:   return "Core";
		case OpenGlProfile_Compat: return "Compat";
		default: return "Unknown";
	}
}
