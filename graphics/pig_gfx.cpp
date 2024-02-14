/*
File:   pig_gfx.cpp
Author: Taylor Robbins
Date:   02\05\2024
Description: 
	** Include this file once in your project to get PigGfx functions compilied into the project
	** (This file #includes all other .cpp files in this folder)
*/

#include "graphics/pig_gfx_helpers.cpp"

#if PIG_GFX_OPENGL_SUPPORTED
#include "graphics/pig_gfx_opengl.cpp"
#endif
#if PIG_GFX_WEBGL_SUPPORTED
#include "graphics/pig_gfx_webgl.cpp"
#endif
#if PIG_GFX_VULKAN_SUPPORTED
#include "graphics/pig_gfx_vulkan.cpp"
#endif
#if PIG_GFX_D3D11_SUPPORTED
#include "graphics/pig_gfx_d3d11.cpp"
#endif
#if PIG_GFX_D3D12_SUPPORTED
#include "graphics/pig_gfx_d3d12.cpp"
#endif
#if PIG_GFX_METAL_SUPPORTED
#include "graphics/pig_gfx_metal.cpp"
#endif

#include "graphics/pig_gfx_api.cpp"
