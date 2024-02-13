/*
File:   pig_graphics_vulkan.h
Author: Taylor Robbins
Date:   02\05\2024
*/

#ifndef _PIG_GRAPHICS_VULKAN_H
#define _PIG_GRAPHICS_VULKAN_H

#if !PIG_GFX_GLFW_SUPPORTED
#error Vulkan support is (currently) dependent upon GLFW being used. Please set PIG_GFX_GLFW_SUPPORTED to 1
#endif

#define VULKAN_IN_FLIGHT_IMAGE_COUNT 2 //TODO: Remove me?

struct VkTestContent_t
{
	VkDeviceMemory vertBufferMem;
	VkBuffer vertBuffer;
	VkRenderPass renderPass;
	VkShaderModule vertShader;
	VkShaderModule fragShader;
	VkPipelineLayout pipelineLayout;
	VkPipeline pipeline;
	u32 framebufferCount;
	VkFramebuffer* framebuffers;
	VkCommandPool commandPool;
	u32 commandBufferCount;
	VkCommandBuffer* commandBuffers;
	VkSemaphore imageAvailableSemaphores[VULKAN_IN_FLIGHT_IMAGE_COUNT];
	VkSemaphore renderFinishedSemaphores[VULKAN_IN_FLIGHT_IMAGE_COUNT];
	VkFence swapImagesFences[VULKAN_IN_FLIGHT_IMAGE_COUNT];
	VkFence activeFences[VULKAN_IN_FLIGHT_IMAGE_COUNT];
	u32 activeSyncIndex;
};

bool PigGfx_Init_Vulkan();
void PigGfx_SetGlfwWindowHints_Vulkan();
GraphicsContext_t* PigGfx_CreateContext_Vulkan();

void PigGfx_BeginRendering_Vulkan(bool doClearColor, Color_t clearColor, bool doClearDepth, r32 clearDepth, bool doClearStencil, int clearStencilValue);

#endif //  _PIG_GRAPHICS_VULKAN_H
