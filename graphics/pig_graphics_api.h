/*
File:   pig_graphics_api.h
Author: Taylor Robbins
Date:   02\06\2024
*/

#ifndef _PIG_GRAPHICS_API_H
#define _PIG_GRAPHICS_API_H

#define VULKAN_IN_FLIGHT_IMAGE_COUNT 2 //TODO: Remove me?

struct PigGfxState_t
{
	bool initialized;
	RenderApi_t renderApi;
	MemArena_t* mainArena;
	
	bool ctxSet;
	PigGfxContext_t ctx;
	bool optionsSet;
	PigGfxOptions_t options;
	
	#if PIG_GFX_GLFW_SUPPORTED
	GLFWwindow* currentGlfwWindow;
	#endif
	
	bool contextCreated;
	GraphicsContext_t context;
	
	#if PIG_GFX_VULKAN_SUPPORTED
	VkDeviceMemory vkTestBufferMem;
	VkBuffer vkTestBuffer;
	VkRenderPass vkRenderPass;
	VkShaderModule vkVertShader;
	VkShaderModule vkFragShader;
	VkPipelineLayout vkPipelineLayout;
	VkPipeline vkPipeline;
	u32 vkNumFramebuffers;
	VkFramebuffer* vkFramebuffers;
	VkCommandPool vkCommandPool;
	u32 vkNumCommandBuffers;
	VkCommandBuffer* vkCommandBuffers;
	VkSemaphore vkImageAvailableSemaphores[VULKAN_IN_FLIGHT_IMAGE_COUNT];
	VkSemaphore vkRenderFinishedSemaphores[VULKAN_IN_FLIGHT_IMAGE_COUNT];
	VkFence vkSwapImagesFences[VULKAN_IN_FLIGHT_IMAGE_COUNT];
	VkFence vkActiveFences[VULKAN_IN_FLIGHT_IMAGE_COUNT];
	u32 activeSyncIndex;
	#endif
};

extern PigGfxState_t* gfx;

u64 PigGfx_GetSupportedRenderApis(MemArena_t* memArena, RenderApi_t** apisOut);
bool PigGfx_Init(const PigGfxContext_t* context, MemArena_t* stateAllocArena, MemArena_t* mainAllocArena, RenderApi_t renderApi);
void PigGfx_FillDefaultOptions(PigGfxOptions_t* optionsOut);
void PigGfx_SetOptions(const PigGfxOptions_t* options);
#if PIG_GFX_GLFW_SUPPORTED
void PigGfx_SetGlfwWindowHints();
void PigGfx_SwitchToGlfwWindow(GLFWwindow* glfwWindowPntr);
#endif
GraphicsContext_t* PigGfx_CreateContext();

// +==============================+
// |       Render Functions       |
// +==============================+
void PigGfx_BeginRendering(bool doClearColor, Color_t clearColor, bool doClearDepth, r32 clearDepth, bool doClearStencil, int clearStencilValue);

#endif //  _PIG_GRAPHICS_API_H
