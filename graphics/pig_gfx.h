/*
File:   pig_gfx.h
Author: Taylor Robbins
Date:   02\05\2024
Description:
	** You probably want to include pig_gfx_early_include.h BEFORE you include this file so you can properly decide the order of includes like glad.h
	** You must define all the [API]_SUPPORTED defines before including this file!
	** This file must be included AFTER gy.h has been included. It depends on many things inside gylib
*/

#ifndef _PIG_GFX_H
#define _PIG_GFX_H

#include "graphics/pig_gfx_early_include.h"

#if PIG_GFX_OPENGL_SUPPORTED
#include "graphics/pig_gfx_opengl_types.h"
#endif
#if PIG_GFX_WEBGL_SUPPORTED
#include "graphics/pig_gfx_webgl_types.h"
#endif
#if PIG_GFX_VULKAN_SUPPORTED
#include "graphics/pig_gfx_vulkan_types.h"
#endif
#if PIG_GFX_D3D11_SUPPORTED
#include "graphics/pig_gfx_d3d11_types.h"
#endif
#if PIG_GFX_D3D12_SUPPORTED
#include "graphics/pig_gfx_d3d12_types.h"
#endif
#if PIG_GFX_METAL_SUPPORTED
#include "graphics/pig_gfx_metal_types.h"
#endif

#include "graphics/pig_gfx_types.h"
#include "graphics/pig_gfx_debug.h"

#if PIG_GFX_OPENGL_SUPPORTED
#include "graphics/pig_gfx_opengl.h"
#endif
#if PIG_GFX_WEBGL_SUPPORTED
#include "graphics/pig_gfx_webgl.h"
#endif
#if PIG_GFX_VULKAN_SUPPORTED
#include "graphics/pig_gfx_vulkan.h"
#endif
#if PIG_GFX_D3D11_SUPPORTED
#include "graphics/pig_gfx_d3d11.h"
#endif
#if PIG_GFX_D3D12_SUPPORTED
#include "graphics/pig_gfx_d3d12.h"
#endif
#if PIG_GFX_METAL_SUPPORTED
#include "graphics/pig_gfx_metal.h"
#endif

#include "graphics/pig_gfx_api.h"

#endif //  _PIG_GFX_H
