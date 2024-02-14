/*
File:   pig_gfx_vulkan_types.h
Author: Taylor Robbins
Date:   02\13\2024
*/

#ifndef _PIG_GFX_VULKAN_TYPES_H
#define _PIG_GFX_VULKAN_TYPES_H

#if !PIG_GFX_GLFW_SUPPORTED
#error Vulkan support is (currently) dependent upon GLFW being used. Please set PIG_GFX_GLFW_SUPPORTED to 1
#endif

#define VULKAN_IN_FLIGHT_IMAGE_COUNT 2 //TODO: Remove me?

struct PigGfxOptions_Vulkan_t
{
	const char* applicationName;
	u32 applicationVersionInt;
	const char* engineName;
	u32 engineVersionInt;
	int requestVersionMajor;
	int requestVersionMinor;
};

struct GraphicsContext_Vulkan_t
{
	VkAllocationCallbacks allocatorStruct;
	VkAllocationCallbacks* allocator;
	VkInstance instance;
	VkSurfaceFormatKHR surfaceFormat;
	VkSurfaceKHR surface;
	VkPhysicalDevice physicalDevice;
	u32 queueFamilyIndex;
	VkExtent2D swapExtent;
	VkDevice device;
	VkQueue queue;
	VkSwapchainKHR swapchain;
	u32 swapImageCount;
	VkImageView* swapImageViews;
};

struct VkTestContent_t
{
	MemArena_t* allocArena;
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

#endif //  _PIG_GFX_VULKAN_TYPES_H
