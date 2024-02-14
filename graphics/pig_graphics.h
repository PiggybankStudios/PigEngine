/*
File:   pig_graphics.h
Author: Taylor Robbins
Date:   02\05\2024
Description:
	** You probably want to include pig_graphics_early_include.h BEFORE you include this file so you can properly decide the order of includes like glad.h
	** You must define all the [API]_SUPPORTED defines before including this file!
	** This file must be included AFTER gy.h has been included. It depends on many things inside gylib
*/

#ifndef _PIG_GRAPHICS_H
#define _PIG_GRAPHICS_H

#include "graphics/pig_graphics_early_include.h"

#if PIG_GFX_OPENGL_SUPPORTED
#include "graphics/pig_graphics_types_opengl.h"
#endif
#if PIG_GFX_WEBGL_SUPPORTED
#include "graphics/pig_graphics_types_webgl.h"
#endif
#if PIG_GFX_VULKAN_SUPPORTED
#include "graphics/pig_graphics_types_vulkan.h"
#endif
#if PIG_GFX_D3D11_SUPPORTED
#include "graphics/pig_graphics_types_d3d11.h"
#endif
#if PIG_GFX_D3D12_SUPPORTED
#include "graphics/pig_graphics_types_d3d12.h"
#endif
#if PIG_GFX_METAL_SUPPORTED
#include "graphics/pig_graphics_types_metal.h"
#endif

#include "graphics/pig_graphics_types.h"
#include "graphics/pig_graphics_debug.h"

#if PIG_GFX_OPENGL_SUPPORTED
#include "graphics/pig_graphics_opengl.h"
#endif
#if PIG_GFX_WEBGL_SUPPORTED
#include "graphics/pig_graphics_webgl.h"
#endif
#if PIG_GFX_VULKAN_SUPPORTED
#include "graphics/pig_graphics_vulkan.h"
#endif
#if PIG_GFX_D3D11_SUPPORTED
#include "graphics/pig_graphics_d3d11.h"
#endif
#if PIG_GFX_D3D12_SUPPORTED
#include "graphics/pig_graphics_d3d12.h"
#endif
#if PIG_GFX_METAL_SUPPORTED
#include "graphics/pig_graphics_metal.h"
#endif

#include "graphics/pig_graphics_api.h"

#endif //  _PIG_GRAPHICS_H
