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
