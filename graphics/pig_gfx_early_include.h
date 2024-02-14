/*
File:   pig_gfx_early_include.h
Author: Taylor Robbins
Date:   02\06\2024
*/

#ifndef _PIG_GFX_EARLY_INCLUDE_H
#define _PIG_GFX_EARLY_INCLUDE_H

#if !defined(PIG_GFX_OPENGL_SUPPORTED)
#error You must define PIG_GFX_OPENGL_SUPPORTED to 0/1 before including pig_gfx_early_include.h
#endif
#if !defined(PIG_GFX_WEBGL_SUPPORTED)
#error You must define PIG_GFX_WEBGL_SUPPORTED to 0/1 before including pig_gfx_early_include.h
#endif
#if !defined(PIG_GFX_VULKAN_SUPPORTED)
#error You must define PIG_GFX_VULKAN_SUPPORTED to 0/1 before including pig_gfx_early_include.h
#endif
#if !defined(PIG_GFX_D3D11_SUPPORTED)
#error You must define PIG_GFX_D3D11_SUPPORTED to 0/1 before including pig_gfx_early_include.h
#endif
#if !defined(PIG_GFX_D3D12_SUPPORTED)
#error You must define PIG_GFX_D3D12_SUPPORTED to 0/1 before including pig_gfx_early_include.h
#endif
#if !defined(PIG_GFX_METAL_SUPPORTED)
#error You must define PIG_GFX_METAL_SUPPORTED to 0/1 before including pig_gfx_early_include.h
#endif

#if !defined(PIG_GFX_GLFW_SUPPORTED)
#error You must define PIG_GFX_GLFW_SUPPORTED to 0/1 before including pig_gfx_early_include.h
#endif

#if PIG_GFX_OPENGL_SUPPORTED
#include "glad/glad.h"
#endif
#if PIG_GFX_WEBGL_SUPPORTED
//TODO: Implement me!
#endif
#if PIG_GFX_VULKAN_SUPPORTED
//TODO: Implement me!
#endif
#if PIG_GFX_D3D11_SUPPORTED
//TODO: Implement me!
#endif
#if PIG_GFX_D3D12_SUPPORTED
//TODO: Implement me!
#endif
#if PIG_GFX_METAL_SUPPORTED
//TODO: Implement me!
#endif

#endif //  _PIG_GFX_EARLY_INCLUDE_H
