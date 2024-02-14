/*
File:   pig_graphics_vulkan.cpp
Author: Taylor Robbins
Date:   02\06\2024
Description: 
	** None 
*/

bool Win32_ReadFileContents(MyStr_t filePath, PlatFileContents_t* contentsOut); //TODO: Remove me once we are no longer responsible for loading files directly!
void Win32_FreeFileContents(PlatFileContents_t* contents); //TODO: Remove me once we are no longer responsible for loading files directly!
void Win32_SleepForMs(u64 milliseconds); //TODO: Remove me once we are no longer responsible for loading files directly!

#define DeclareVkStruct(type, varName, typeEnum) type varName = {}; varName.sType = typeEnum; varName.pNext = nullptr; varName.flags = 0
#define DeclareVkStructNoFlags(type, varName, typeEnum) type varName = {}; varName.sType = typeEnum; varName.pNext = nullptr

#define DeclareVkBufferCreateInfo(varName)                     DeclareVkStruct(VkBufferCreateInfo,                     varName, VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO)
#define DeclareVkMemoryAllocateInfo(varName)                   DeclareVkStructNoFlags(VkMemoryAllocateInfo,            varName, VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO)
#define DeclareVkMappedMemoryRange(varName)                    DeclareVkStructNoFlags(VkMappedMemoryRange,             varName, VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE)
#define DeclareVkRenderPassCreateInfo(varName)                 DeclareVkStruct(VkRenderPassCreateInfo,                 varName, VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO)
#define DeclareVkShaderModuleCreateInfo(varName)               DeclareVkStruct(VkShaderModuleCreateInfo,               varName, VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO)
#define DeclareVkPipelineLayoutCreateInfo(varName)             DeclareVkStruct(VkPipelineLayoutCreateInfo,             varName, VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO)
#define DeclareVkPipelineShaderStageCreateInfo(varName)        DeclareVkStruct(VkPipelineShaderStageCreateInfo,        varName, VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO)
#define DeclareVkFramebufferCreateInfo(varName)                DeclareVkStruct(VkFramebufferCreateInfo,                varName, VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO)
#define DeclareVkCommandPoolCreateInfo(varName)                DeclareVkStruct(VkCommandPoolCreateInfo,                varName, VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO)
#define DeclareVkCommandBufferAllocateInfo(varName)            DeclareVkStructNoFlags(VkCommandBufferAllocateInfo,     varName, VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO)
#define DeclareVkSemaphoreCreateInfo(varName)                  DeclareVkStruct(VkSemaphoreCreateInfo,                  varName, VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO)
#define DeclareVkFenceCreateInfo(varName)                      DeclareVkStruct(VkFenceCreateInfo,                      varName, VK_STRUCTURE_TYPE_FENCE_CREATE_INFO)
#define DeclareVkApplicationInfo(varName)                      DeclareVkStructNoFlags(VkApplicationInfo,               varName, VK_STRUCTURE_TYPE_APPLICATION_INFO)
#define DeclareVkDeviceQueueCreateInfo(varName)                DeclareVkStruct(VkDeviceQueueCreateInfo,                varName, VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO)
#define DeclareVkSwapchainCreateInfoKHR(varName)               DeclareVkStruct(VkSwapchainCreateInfoKHR,               varName, VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR)
#define DeclareVkImageViewCreateInfo(varName)                  DeclareVkStruct(VkImageViewCreateInfo,                  varName, VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO)
#define DeclareVkSubmitInfo(varName)                           DeclareVkStructNoFlags(VkSubmitInfo,                    varName, VK_STRUCTURE_TYPE_SUBMIT_INFO)
#define DeclareVkCommandBufferBeginInfo(varName)               DeclareVkStruct(VkCommandBufferBeginInfo,               varName, VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO)
#define DeclareVkRenderPassBeginInfo(varName)                  DeclareVkStructNoFlags(VkRenderPassBeginInfo,           varName, VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO)
#define DeclareVkPresentInfoKHR(varName)                       DeclareVkStructNoFlags(VkPresentInfoKHR,                varName, VK_STRUCTURE_TYPE_PRESENT_INFO_KHR)
#define DeclareVkPipelineVertexInputStateCreateInfo(varName)   DeclareVkStruct(VkPipelineVertexInputStateCreateInfo,   varName, VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO)
#define DeclareVkPipelineInputAssemblyStateCreateInfo(varName) DeclareVkStruct(VkPipelineInputAssemblyStateCreateInfo, varName, VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO)
#define DeclareVkPipelineViewportStateCreateInfo(varName)      DeclareVkStruct(VkPipelineViewportStateCreateInfo,      varName, VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO)
#define DeclareVkPipelineRasterizationStateCreateInfo(varName) DeclareVkStruct(VkPipelineRasterizationStateCreateInfo, varName, VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO)
#define DeclareVkPipelineMultisampleStateCreateInfo(varName)   DeclareVkStruct(VkPipelineMultisampleStateCreateInfo,   varName, VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO)
#define DeclareVkPipelineColorBlendStateCreateInfo(varName)    DeclareVkStruct(VkPipelineColorBlendStateCreateInfo,    varName, VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO)
#define DeclareVkGraphicsPipelineCreateInfo(varName)           DeclareVkStruct(VkGraphicsPipelineCreateInfo,           varName, VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO)
#define DeclareVkInstanceCreateInfo(varName)                   DeclareVkStruct(VkInstanceCreateInfo,                   varName, VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO)
#define DeclareVkDeviceCreateInfo(varName)                     DeclareVkStruct(VkDeviceCreateInfo,                     varName, VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO)

bool _FindQueueFamilyThatSupportsPresent(VkPhysicalDevice* device, VkSurfaceKHR* surface, u32* familyIndexOut)
{
	u32 queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(*device, &queueFamilyCount, nullptr);
	if (queueFamilyCount == 0) { return false; }
	
	//TODO: Move this allocation into scratch arena!
	VkQueueFamilyProperties* queueFamilies = AllocArray(gfx->mainArena, VkQueueFamilyProperties, queueFamilyCount);
	NotNull(queueFamilies);
	vkGetPhysicalDeviceQueueFamilyProperties(*device, &queueFamilyCount, queueFamilies);
	
	for (u32 qIndex = 0; qIndex < queueFamilyCount; qIndex++)
	{
		VkQueueFamilyProperties* queueFamily = &queueFamilies[qIndex];
		if (IsFlagSet(queueFamily->queueFlags, VK_QUEUE_GRAPHICS_BIT))
		{
			VkBool32 supportsPresent = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(*device, qIndex, *surface, &supportsPresent);
			if (supportsPresent)
			{
				SetOptionalOutPntr(familyIndexOut, qIndex);
				FreeMem(gfx->mainArena, queueFamilies);
				return true;
			}
		}
	}
	
	FreeMem(gfx->mainArena, queueFamilies);
	return false;
}

u32 _FindMemoryType(const VkPhysicalDevice physicalDevice, u32 typeFilter, VkMemoryPropertyFlags properties)
{
	//TODO: What does this do exactly??
	VkPhysicalDeviceMemoryProperties memProperties = {};
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
	for (u32 tIndex = 0; tIndex < memProperties.memoryTypeCount; tIndex++)
	{
		if (IsFlagSet(typeFilter, (u32)(1 << tIndex)) &&
			IsFlagSet(memProperties.memoryTypes[tIndex].propertyFlags, properties))
		{
			return tIndex;
		}
	}
	AssertMsg(false, "Couldn't find proper memory type in vertex buffer!");
	return 0;
}

bool PigGfx_Init_Vulkan()
{
	NotNull(gfx);
	bool vulkanSupported = (glfwVulkanSupported() == GLFW_TRUE);
	if (!vulkanSupported) { PigGfx_InitFailure("Vulkan is not supported on this computer and RenderApi_Vulkan was requested as the rendering framework!"); return false; }
	u32 instanceCreationExtensionCount = 0;
	if (glfwGetRequiredInstanceExtensions(&instanceCreationExtensionCount) == nullptr) { PigGfx_InitFailure("Vulkan implementation on this computer does not support rendering instance creation!"); return false; }
	return true;
}

void PigGfx_SetGlfwWindowHints_Vulkan()
{
	NotNull(gfx);
	Assert(gfx->optionsSet);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
}

void _DestroyVkTestContent(GraphicsContext_t* context, VkTestContent_t* content)
{
	vkFreeCommandBuffers(context->vkDevice, content->commandPool, content->commandBufferCount, &content->commandBuffers[0]);
	if (content->commandBuffers != nullptr) { FreeMem(content->allocArena, content->commandBuffers, sizeof(VkCommandBuffer) * content->commandBufferCount); }
	vkDestroyCommandPool(context->vkDevice, content->commandPool, context->vkAllocator);
	if (content->framebuffers != nullptr)
	{
		for (u32 fIndex = 0; fIndex < content->framebufferCount; fIndex++) { vkDestroyFramebuffer(context->vkDevice, content->framebuffers[fIndex], context->vkAllocator); }
		FreeMem(content->allocArena, content->framebuffers, sizeof(VkFramebuffer) * content->framebufferCount);
	}
	vkDestroyPipeline(context->vkDevice, content->pipeline, context->vkAllocator);
	vkDestroyPipelineLayout(context->vkDevice, content->pipelineLayout, context->vkAllocator);
	vkDestroyShaderModule(context->vkDevice, content->fragShader, context->vkAllocator);
	vkDestroyShaderModule(context->vkDevice, content->vertShader, context->vkAllocator);
	vkDestroyRenderPass(context->vkDevice, content->renderPass, context->vkAllocator);
	vkFreeMemory(context->vkDevice, content->vertBufferMem, context->vkAllocator);
	vkDestroyBuffer(context->vkDevice, content->vertBuffer, context->vkAllocator);
	ClearPointer(content);
}

bool _CreateVkTestContent(GraphicsContext_t* context, VkTestContent_t* content, MemArena_t* memArena)
{
	ClearPointer(content);
	content->allocArena = memArena;
	
	// +==========================================+
	// | Create vkTestBufferMem and vkTestBuffer  |
	// +==========================================+
	{
		float vertices[] = {
			 0.0f, -0.5f,
			 0.5f,  0.5f,
			-0.5f,  0.5f,
		};
		
		DeclareVkBufferCreateInfo(vkCreateBufferParams);
		vkCreateBufferParams.size = sizeof(float) * ArrayCount(vertices);
		vkCreateBufferParams.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		vkCreateBufferParams.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		vkCreateBufferParams.queueFamilyIndexCount = 0;
		vkCreateBufferParams.pQueueFamilyIndices = nullptr;
		if (vkCreateBuffer(context->vkDevice, &vkCreateBufferParams, context->vkAllocator, &content->vertBuffer) != VK_SUCCESS)
		{
			PigGfx_InitFailure("Vulkan buffer creation failed!");
			_DestroyVkTestContent(context, content);
			return false;
		}
		
		VkMemoryRequirements testBufferMemReqs;
		vkGetBufferMemoryRequirements(context->vkDevice, content->vertBuffer, &testBufferMemReqs);
		u32 memoryTypeIndex = _FindMemoryType(context->vkPhysicalDevice, testBufferMemReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		
		DeclareVkMemoryAllocateInfo(vkAllocateMemoryParams);
		vkAllocateMemoryParams.allocationSize = testBufferMemReqs.size;
		vkAllocateMemoryParams.memoryTypeIndex = memoryTypeIndex;
		if (vkAllocateMemory(context->vkDevice, &vkAllocateMemoryParams, context->vkAllocator, &content->vertBufferMem) != VK_SUCCESS)
		{
			PigGfx_InitFailure("Vulkan buffer memory allocation failed!");
			_DestroyVkTestContent(context, content);
			return false;
		}
		
		vkBindBufferMemory(context->vkDevice, content->vertBuffer, content->vertBufferMem, 0);
		
		void* testBufferPntr;
		if (vkMapMemory(context->vkDevice, content->vertBufferMem, 0, VK_WHOLE_SIZE, 0, &testBufferPntr) != VK_SUCCESS)
		{
			PigGfx_InitFailure("Vulkan buffer memory map failed!");
			_DestroyVkTestContent(context, content);
			return false;
		}
		
		MyMemCopy(testBufferPntr, &vertices[0], sizeof(float) * ArrayCount(vertices));
		
		DeclareVkMappedMemoryRange(vkFlushMappedMemoryRangesParams);
		vkFlushMappedMemoryRangesParams.memory = content->vertBufferMem;
		vkFlushMappedMemoryRangesParams.offset = 0;
		vkFlushMappedMemoryRangesParams.size = VK_WHOLE_SIZE;
		vkFlushMappedMemoryRanges(context->vkDevice, 1, &vkFlushMappedMemoryRangesParams);
		
		vkUnmapMemory(context->vkDevice, content->vertBufferMem);
	}
	
	// +==============================+
	// |     Create vkRenderPass      |
	// +==============================+
	{
		VkAttachmentDescription colorAttachment = {};
		colorAttachment.flags = 0;
		colorAttachment.format = context->vkSurfaceFormat.format;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		
		VkAttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		
		VkSubpassDescription subpassDesc = {};
		subpassDesc.flags = 0;
		subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDesc.inputAttachmentCount = 0;
		subpassDesc.pInputAttachments = nullptr;
		subpassDesc.colorAttachmentCount = 1;
		subpassDesc.pColorAttachments = &colorAttachmentRef;
		subpassDesc.pResolveAttachments = nullptr;
		subpassDesc.pDepthStencilAttachment = nullptr;
		subpassDesc.preserveAttachmentCount = 0;
		subpassDesc.pPreserveAttachments = nullptr;
		
		DeclareVkRenderPassCreateInfo(vkCreateRenderPassParams);
		vkCreateRenderPassParams.attachmentCount = 1;
		vkCreateRenderPassParams.pAttachments = &colorAttachment;
		vkCreateRenderPassParams.subpassCount = 1;
		vkCreateRenderPassParams.pSubpasses = &subpassDesc;
		vkCreateRenderPassParams.dependencyCount = 0;
		vkCreateRenderPassParams.pDependencies = nullptr;
		if (vkCreateRenderPass(context->vkDevice, &vkCreateRenderPassParams, context->vkAllocator, &content->renderPass) != VK_SUCCESS)
		{
			PigGfx_InitFailure("Vulkan renderpass creation failed!");
			_DestroyVkTestContent(context, content);
			return false;
		}
	}
	
	// +======================================+
	// | Create vkVertShader and vkFragShader |
	// +======================================+
	{
		PlatFileContents_t vertFile;
		if (!Win32_ReadFileContents(NewStr("Resources/Shaders/vulkan_basic.vert.spv"), &vertFile))
		{
			PigGfx_InitFailure("Failed to open/read basic vert shader SPIR-V code from file!");
			_DestroyVkTestContent(context, content);
			return false;
		}
		PlatFileContents_t fragFile;
		if (!Win32_ReadFileContents(NewStr("Resources/Shaders/vulkan_basic.frag.spv"), &fragFile))
		{
			PigGfx_InitFailure("Failed to open/read basic frag shader SPIR-V code from file!");
			Win32_FreeFileContents(&vertFile);
			_DestroyVkTestContent(context, content);
			return false;
		}
		
		DeclareVkShaderModuleCreateInfo(vkCreateShaderModuleVertParams);
		vkCreateShaderModuleVertParams.codeSize = vertFile.size;
		vkCreateShaderModuleVertParams.pCode = (u32*)vertFile.data;
		if (vkCreateShaderModule(context->vkDevice, &vkCreateShaderModuleVertParams, context->vkAllocator, &content->vertShader) != VK_SUCCESS)
		{
			PigGfx_InitFailure("Vulkan vert shader module creation failed!");
			Win32_FreeFileContents(&vertFile);
			Win32_FreeFileContents(&fragFile);
			_DestroyVkTestContent(context, content);
			return false;
		}
		
		DeclareVkShaderModuleCreateInfo(vkCreateShaderModuleFragParams);
		vkCreateShaderModuleFragParams.codeSize = fragFile.size;
		vkCreateShaderModuleFragParams.pCode = (u32*)fragFile.data;
		if (vkCreateShaderModule(context->vkDevice, &vkCreateShaderModuleFragParams, context->vkAllocator, &content->fragShader) != VK_SUCCESS)
		{
			PigGfx_InitFailure("Vulkan frag shader module creation failed!");
			Win32_FreeFileContents(&vertFile);
			Win32_FreeFileContents(&fragFile);
			_DestroyVkTestContent(context, content);
			return false;
		}
		
		Win32_FreeFileContents(&vertFile);
		Win32_FreeFileContents(&fragFile);
	}
	
	// +==============================+
	// |   Create vkPipelineLayout    |
	// +==============================+
	{
		DeclareVkPipelineLayoutCreateInfo(vkCreatePipelineLayoutParams);
		vkCreatePipelineLayoutParams.setLayoutCount = 0;
		vkCreatePipelineLayoutParams.pSetLayouts = nullptr;
		vkCreatePipelineLayoutParams.pushConstantRangeCount = 0;
		if (vkCreatePipelineLayout(context->vkDevice, &vkCreatePipelineLayoutParams, context->vkAllocator, &content->pipelineLayout) != VK_SUCCESS)
		{
			PigGfx_InitFailure("Vulkan pipeline layout creation failed!");
			_DestroyVkTestContent(context, content);
			return false;
		}
	}
	
	// +==============================+
	// |      Create vkPipeline       |
	// +==============================+
	{
		DeclareVkPipelineShaderStageCreateInfo(vertShaderStageInfo);
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = content->vertShader;
		vertShaderStageInfo.pName = "main";
		vertShaderStageInfo.pSpecializationInfo = nullptr;
		
		DeclareVkPipelineShaderStageCreateInfo(fragShaderStageInfo);
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = content->fragShader;
		fragShaderStageInfo.pName = "main";
		fragShaderStageInfo.pSpecializationInfo = nullptr;
		
		VkPipelineShaderStageCreateInfo shaderStageCreateInfos[] = { vertShaderStageInfo, fragShaderStageInfo };
		
		VkVertexInputBindingDescription v2VertexBinding = {};
		v2VertexBinding.binding = 0;
		v2VertexBinding.stride = sizeof(float) * 2;
		v2VertexBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		
		VkVertexInputAttributeDescription positionVertAttribDesc = {};
		positionVertAttribDesc.binding = 0;
		positionVertAttribDesc.location = 0; //Location in the vertex shader
		positionVertAttribDesc.format = VK_FORMAT_R32G32_SFLOAT;
		positionVertAttribDesc.offset = 0;
		
		DeclareVkPipelineVertexInputStateCreateInfo(vertexInputInfo);
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.pVertexBindingDescriptions = &v2VertexBinding;
		vertexInputInfo.vertexAttributeDescriptionCount = 1;
		vertexInputInfo.pVertexAttributeDescriptions = &positionVertAttribDesc;
		
		DeclareVkPipelineInputAssemblyStateCreateInfo(inputAssemblyInfo);
		inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;
		
		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)context->swapExtent.width;
		viewport.height = (float)context->swapExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		
		VkRect2D scissor = {};
		scissor.offset = { 0, 0 };
		scissor.extent = context->swapExtent;
		
		DeclareVkPipelineViewportStateCreateInfo(viewportCreateInfo);
		viewportCreateInfo.viewportCount = 1;
		viewportCreateInfo.pViewports = &viewport;
		viewportCreateInfo.scissorCount = 1;
		viewportCreateInfo.pScissors = &scissor;
		
		DeclareVkPipelineRasterizationStateCreateInfo(rasterizerCreateInfo);
		rasterizerCreateInfo.depthClampEnable = VK_FALSE;
		rasterizerCreateInfo.rasterizerDiscardEnable = VK_FALSE;
		rasterizerCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizerCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizerCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizerCreateInfo.depthBiasEnable = VK_FALSE;
		rasterizerCreateInfo.depthBiasConstantFactor = 0.0f;
		rasterizerCreateInfo.depthBiasClamp = 0.0f;
		rasterizerCreateInfo.depthBiasSlopeFactor = 0.0f;
		rasterizerCreateInfo.lineWidth = 1.0f;
		
		DeclareVkPipelineMultisampleStateCreateInfo(multisamplingCreateInfo);
		multisamplingCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisamplingCreateInfo.sampleShadingEnable = VK_FALSE;
		multisamplingCreateInfo.minSampleShading = 0.0f;
		multisamplingCreateInfo.pSampleMask = nullptr;
		multisamplingCreateInfo.alphaToCoverageEnable = VK_FALSE;
		multisamplingCreateInfo.alphaToOneEnable = VK_FALSE;
		
		VkPipelineColorBlendAttachmentState colorBlendAttachment;
		colorBlendAttachment.blendEnable = VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		
		DeclareVkPipelineColorBlendStateCreateInfo(colorBlendCreateInfo);
		colorBlendCreateInfo.logicOpEnable = VK_FALSE;
		colorBlendCreateInfo.logicOp = VK_LOGIC_OP_COPY;
		colorBlendCreateInfo.attachmentCount = 1;
		colorBlendCreateInfo.pAttachments = &colorBlendAttachment;
		colorBlendCreateInfo.blendConstants[0] = 0.0f;
		colorBlendCreateInfo.blendConstants[1] = 0.0f;
		colorBlendCreateInfo.blendConstants[2] = 0.0f;
		colorBlendCreateInfo.blendConstants[3] = 0.0f;
		
		DeclareVkGraphicsPipelineCreateInfo(vkCreateGraphicsPipelinesParams);
		vkCreateGraphicsPipelinesParams.stageCount = ArrayCount(shaderStageCreateInfos);
		vkCreateGraphicsPipelinesParams.pStages = &shaderStageCreateInfos[0];
		vkCreateGraphicsPipelinesParams.pVertexInputState = &vertexInputInfo;
		vkCreateGraphicsPipelinesParams.pInputAssemblyState = &inputAssemblyInfo;
		vkCreateGraphicsPipelinesParams.pTessellationState = nullptr;
		vkCreateGraphicsPipelinesParams.pViewportState = &viewportCreateInfo;
		vkCreateGraphicsPipelinesParams.pRasterizationState = &rasterizerCreateInfo;
		vkCreateGraphicsPipelinesParams.pMultisampleState = &multisamplingCreateInfo;
		vkCreateGraphicsPipelinesParams.pDepthStencilState = nullptr;
		vkCreateGraphicsPipelinesParams.pColorBlendState = &colorBlendCreateInfo;
		vkCreateGraphicsPipelinesParams.pDynamicState = nullptr;
		vkCreateGraphicsPipelinesParams.layout = content->pipelineLayout;
		vkCreateGraphicsPipelinesParams.renderPass = content->renderPass;
		vkCreateGraphicsPipelinesParams.subpass = 0;
		vkCreateGraphicsPipelinesParams.basePipelineHandle = VK_NULL_HANDLE;
		vkCreateGraphicsPipelinesParams.basePipelineIndex = 0;
		if (vkCreateGraphicsPipelines(context->vkDevice, VK_NULL_HANDLE, 1, &vkCreateGraphicsPipelinesParams, context->vkAllocator, &content->pipeline) != VK_SUCCESS)
		{
			PigGfx_InitFailure("Vulkan pipeline creation failed!");
			_DestroyVkTestContent(context, content);
			return false;
		}
	}
	
	// +==============================+
	// |    Create vkFramebuffers     |
	// +==============================+
	{
		content->framebufferCount = context->vkSwapImageCount;
		content->framebuffers = AllocArray(content->allocArena, VkFramebuffer, content->framebufferCount);
		NotNull(content->framebuffers);
		MyMemSet(content->framebuffers, 0x00, sizeof(VkFramebuffer) * content->framebufferCount);
		
		DeclareVkFramebufferCreateInfo(vkCreateFramebufferParams);
		vkCreateFramebufferParams.renderPass = content->renderPass;
		vkCreateFramebufferParams.attachmentCount = 1;
		vkCreateFramebufferParams.pAttachments = nullptr; // Set in the loop below
		vkCreateFramebufferParams.width = context->swapExtent.width;
		vkCreateFramebufferParams.height = context->swapExtent.height;
		vkCreateFramebufferParams.layers = 1;
		
		for (u32 fIndex = 0; fIndex < content->framebufferCount; fIndex++)
		{
			vkCreateFramebufferParams.pAttachments = &context->vkSwapImageViews[fIndex];
			if (vkCreateFramebuffer(context->vkDevice, &vkCreateFramebufferParams, context->vkAllocator, &content->framebuffers[fIndex]) != VK_SUCCESS)
			{
				PigGfx_InitFailure("Vulkan framebuffer creation failed!");
				_DestroyVkTestContent(context, content);
				return false;
			}
		}
	}
	
	// +==============================+
	// |     Create vkCommandPool     |
	// +==============================+
	{
		DeclareVkCommandPoolCreateInfo(vkCreateCommandPoolParams);
		vkCreateCommandPoolParams.queueFamilyIndex = context->queueFamilyIndex;
		if (vkCreateCommandPool(context->vkDevice, &vkCreateCommandPoolParams, context->vkAllocator, &content->commandPool) != VK_SUCCESS)
		{
			PigGfx_InitFailure("Vulkan command pool creation failed!");
			_DestroyVkTestContent(context, content);
			return false;
		}
	}
	
	// +==============================+
	// |   Create vkCommandBuffers    |
	// +==============================+
	{
		content->commandBufferCount = context->vkSwapImageCount;
		content->commandBuffers = AllocArray(content->allocArena, VkCommandBuffer, content->commandBufferCount);
		NotNull(content->commandBuffers);
		MyMemSet(content->commandBuffers, 0x00, sizeof(VkCommandBuffer) * content->commandBufferCount);
		
		DeclareVkCommandBufferAllocateInfo(vkAllocateCommandBuffersParams);
		vkAllocateCommandBuffersParams.commandPool = content->commandPool;
		vkAllocateCommandBuffersParams.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		vkAllocateCommandBuffersParams.commandBufferCount = content->commandBufferCount;
		if (vkAllocateCommandBuffers(context->vkDevice, &vkAllocateCommandBuffersParams, &content->commandBuffers[0]) != VK_SUCCESS)
		{
			PigGfx_InitFailure("Vulkan command buffer allocation failed!");
			_DestroyVkTestContent(context, content);
			return false;
		}
	}
	
	// +==============================+
	// | Create Semaphores and Fences |
	// +==============================+
	{
		DeclareVkSemaphoreCreateInfo(vkCreateSemaphoreParams);
		DeclareVkFenceCreateInfo(vkCreateFenceParams);
		vkCreateFenceParams.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		
		for (u32 iIndex = 0; iIndex < VULKAN_IN_FLIGHT_IMAGE_COUNT; iIndex++)
		{
			content->swapImagesFences[iIndex] = VK_NULL_HANDLE;
			if (vkCreateSemaphore(context->vkDevice, &vkCreateSemaphoreParams, context->vkAllocator, &content->imageAvailableSemaphores[iIndex]) != VK_SUCCESS ||
				vkCreateSemaphore(context->vkDevice, &vkCreateSemaphoreParams, context->vkAllocator, &content->renderFinishedSemaphores[iIndex]) != VK_SUCCESS ||
				vkCreateFence(context->vkDevice, &vkCreateFenceParams, context->vkAllocator, &content->activeFences[iIndex]) != VK_SUCCESS)
			{
				PigGfx_InitFailure("Vulkan semaphore/fence creation failed!");
				_DestroyVkTestContent(context, content);
				return false;
			}
		}
		
		content->activeSyncIndex = 0;
	}
	
	return true;
}

void PigGfx_DestroyContext_Vulkan(GraphicsContext_t* context)
{
	NotNull(context->allocArena);
	for (u32 iIndex = 0; iIndex < gfx->context.vkSwapImageCount; iIndex++) { vkDestroyImageView(context->vkDevice, context->vkSwapImageViews[iIndex], context->vkAllocator); }
	if (context->vkSwapImageViews != nullptr) { FreeMem(context->allocArena, context->vkSwapImageViews, sizeof(VkImageView) * context->vkSwapImageCount); }
	vkDestroySwapchainKHR(context->vkDevice, context->vkSwapchain, context->vkAllocator);
	vkDestroyDevice(context->vkDevice, context->vkAllocator);
	vkDestroySurfaceKHR(context->vkInstance, context->vkSurface, context->vkAllocator);
	vkDestroyInstance(context->vkInstance, context->vkAllocator);
	ClearPointer(context);
}

GraphicsContext_t* PigGfx_CreateContext_Vulkan(MemArena_t* memArena)
{
	NotNull(gfx);
	Assert(!gfx->contextCreated);
	Assert(gfx->optionsSet);
	NotNull(gfx->currentGlfwWindow);
	NotNull(memArena);
	
	GraphicsContext_t* context = &gfx->context;
	ClearPointer(context);
	context->allocArena = memArena;
	//TODO: Implement this!
	context->vkAllocator = nullptr;
	
	// +==============================+
	// |      Create vkInstance       |
	// +==============================+
	{
		u32 instanceCreationExtensionCount = 0;
		const char** instanceCreationExtensions = glfwGetRequiredInstanceExtensions(&instanceCreationExtensionCount);
		Assert(instanceCreationExtensions != nullptr);
		
		DeclareVkApplicationInfo(applicationInfo);
		applicationInfo.pApplicationName = gfx->options.vulkan_ApplicationName;
		applicationInfo.applicationVersion = gfx->options.vulkan_ApplicationVersionInt;
		applicationInfo.pEngineName = gfx->options.vulkan_EngineName;
		applicationInfo.engineVersion = gfx->options.vulkan_EngineVersionInt;
		applicationInfo.apiVersion = VK_MAKE_API_VERSION(0, gfx->options.vulkan_RequestVersionMajor, gfx->options.vulkan_RequestVersionMinor, 0);
		
		DeclareVkInstanceCreateInfo(vkCreateInstanceParams);
		vkCreateInstanceParams.pApplicationInfo = &applicationInfo;
		const char* vulkanLayers[] = { "VK_LAYER_KHRONOS_validation" };
		vkCreateInstanceParams.enabledLayerCount = ArrayCount(vulkanLayers);
		vkCreateInstanceParams.ppEnabledLayerNames = (ArrayCount(vulkanLayers) > 0) ? &vulkanLayers[0] : nullptr;
		vkCreateInstanceParams.enabledExtensionCount = instanceCreationExtensionCount;
		vkCreateInstanceParams.ppEnabledExtensionNames = instanceCreationExtensions;
		if (vkCreateInstance(&vkCreateInstanceParams, context->vkAllocator, &context->vkInstance) != VK_SUCCESS)
		{
			PigGfx_InitFailure("Vulkan instance creation failed!");
			PigGfx_DestroyContext_Vulkan(context);
			return nullptr;
		}
	}
	
	// +==============================+
	// |       Create vkSurface       |
	// +==============================+
	if (glfwCreateWindowSurface(context->vkInstance, gfx->currentGlfwWindow, context->vkAllocator, &context->vkSurface) != VK_SUCCESS)
	{
		PigGfx_InitFailure("Vulkan window surface creation failed!");
		PigGfx_DestroyContext_Vulkan(context);
		return nullptr;
	}
	
	// +==============================+
	// |   Create vkPhysicalDevice    |
	// +==============================+
	{
		u32 deviceCount;
		vkEnumeratePhysicalDevices(context->vkInstance, &deviceCount, nullptr);
		if (deviceCount == 0)
		{
			PigGfx_InitFailure("Vulkan physical device enumeration failed");
			PigGfx_DestroyContext_Vulkan(context);
			return nullptr;
		}
		
		VkPhysicalDevice* allDevices = AllocArray(memArena, VkPhysicalDevice, deviceCount);
		NotNull(allDevices);
		vkEnumeratePhysicalDevices(context->vkInstance, &deviceCount, allDevices);
		
		bool foundDeviceWithQueue = false;
		for (u32 dIndex = 0; dIndex < deviceCount; dIndex++)
		{
			VkPhysicalDevice* device = &allDevices[dIndex];
			if (_FindQueueFamilyThatSupportsPresent(device, &context->vkSurface, &context->queueFamilyIndex))
			{
				context->vkPhysicalDevice = *device;
				foundDeviceWithQueue = true;
				break;
			}
		}
		
		FreeMem(memArena, allDevices, sizeof(VkPhysicalDevice) * deviceCount);
		
		if (!foundDeviceWithQueue)
		{
			PigGfx_InitFailure("Couldn't find physical device with appropriate queue capabilities (enumerating in Vulkan)");
			PigGfx_DestroyContext_Vulkan(context);
			return nullptr;
		}
	}
	
	// +==============================+
	// | Create vkDevice and vkQueue  |
	// +==============================+
	{
		DeclareVkDeviceQueueCreateInfo(deviceQueueCreateInfo);
		deviceQueueCreateInfo.queueFamilyIndex = context->queueFamilyIndex;
		deviceQueueCreateInfo.queueCount = 1;
		r32 queuePriority = 1.0f;
		deviceQueueCreateInfo.pQueuePriorities = &queuePriority;
		
		DeclareVkDeviceCreateInfo(vkCreateDeviceParams);
		vkCreateDeviceParams.queueCreateInfoCount = 1;
		vkCreateDeviceParams.pQueueCreateInfos = &deviceQueueCreateInfo;
		vkCreateDeviceParams.pEnabledFeatures = nullptr;
		const char* enabledExtensionNames[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		vkCreateDeviceParams.enabledExtensionCount = ArrayCount(enabledExtensionNames);
		vkCreateDeviceParams.ppEnabledExtensionNames = &enabledExtensionNames[0];
		const char* enabledLayerNames[] = { "VK_LAYER_KHRONOS_validation" };
		vkCreateDeviceParams.enabledLayerCount = ArrayCount(enabledLayerNames);
		vkCreateDeviceParams.ppEnabledLayerNames = &enabledLayerNames[0];
		if (vkCreateDevice(context->vkPhysicalDevice, &vkCreateDeviceParams, context->vkAllocator, &context->vkDevice) != VK_SUCCESS)
		{
			PigGfx_InitFailure("Vulkan device creation failed!");
			PigGfx_DestroyContext_Vulkan(context);
			return nullptr;
		}
		
		vkGetDeviceQueue(context->vkDevice, context->queueFamilyIndex, 0, &context->vkQueue);
	}
	
	int windowWidth, windowHeight;
	glfwGetWindowSize(gfx->currentGlfwWindow, &windowWidth, &windowHeight);
	context->swapExtent = { (u32)windowWidth, (u32)windowHeight };
	
	// +========================================+
	// | Create vkSurfaceFormat and vkSwapchain |
	// +========================================+
	{
		VkSurfaceCapabilitiesKHR surfaceCapabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context->vkPhysicalDevice, context->vkSurface, &surfaceCapabilities);
		if (surfaceCapabilities.currentExtent.width != UINT32_MAX)
		{
			context->swapExtent = surfaceCapabilities.currentExtent;
		}
		
		u32 formatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(context->vkPhysicalDevice, context->vkSurface, &formatCount, nullptr);
		Assert(formatCount > 0);
		VkSurfaceFormatKHR* allFormats = AllocArray(memArena, VkSurfaceFormatKHR, formatCount);
		NotNull(allFormats);
		vkGetPhysicalDeviceSurfaceFormatsKHR(context->vkPhysicalDevice, context->vkSurface, &formatCount, &allFormats[0]);
		
		bool foundDesiredFormat = false;
		for (u32 fIndex = 0; fIndex < formatCount; fIndex++)
		{
			VkSurfaceFormatKHR* format = &allFormats[fIndex];
			if (format->format == VK_FORMAT_B8G8R8A8_SRGB && format->colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				context->vkSurfaceFormat = *format;
				foundDesiredFormat = true;
				break;
			}
		}
		
		FreeMem(memArena, allFormats, sizeof(VkSurfaceFormatKHR) * formatCount);
		
		if (!foundDesiredFormat)
		{
			PigGfx_InitFailure("Couldn't find a supported surface format on the (physical device chosen) that matches the format we expect");
			PigGfx_DestroyContext_Vulkan(context);
			return nullptr;
		}
		
		DeclareVkSwapchainCreateInfoKHR(vkCreateSwapchainParams);
		vkCreateSwapchainParams.surface = context->vkSurface;
		vkCreateSwapchainParams.minImageCount = surfaceCapabilities.minImageCount + 1;
		vkCreateSwapchainParams.imageFormat = context->vkSurfaceFormat.format;
		vkCreateSwapchainParams.imageColorSpace = context->vkSurfaceFormat.colorSpace;
		vkCreateSwapchainParams.imageExtent = context->swapExtent;
		vkCreateSwapchainParams.imageArrayLayers = 1;
		vkCreateSwapchainParams.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		vkCreateSwapchainParams.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		vkCreateSwapchainParams.queueFamilyIndexCount = 0;
		vkCreateSwapchainParams.pQueueFamilyIndices = nullptr;
		vkCreateSwapchainParams.preTransform = surfaceCapabilities.currentTransform;
		vkCreateSwapchainParams.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		vkCreateSwapchainParams.presentMode = VK_PRESENT_MODE_FIFO_KHR;
		vkCreateSwapchainParams.clipped = VK_TRUE;
		vkCreateSwapchainParams.oldSwapchain = VK_NULL_HANDLE;
		if (vkCreateSwapchainKHR(context->vkDevice, &vkCreateSwapchainParams, context->vkAllocator, &context->vkSwapchain) != VK_SUCCESS)
		{
			PigGfx_InitFailure("Vulkan swapchain creation failed!");
			PigGfx_DestroyContext_Vulkan(context);
			return nullptr;
		}
	}
	
	// +==============================+
	// |   Create vkSwapImageViews    |
	// +==============================+
	{
		vkGetSwapchainImagesKHR(context->vkDevice, context->vkSwapchain, &context->vkSwapImageCount, nullptr);
		Assert(context->vkSwapImageCount > 0);
		VkImage* vkSwapImages = AllocArray(memArena, VkImage, context->vkSwapImageCount);
		NotNull(vkSwapImages);
		vkGetSwapchainImagesKHR(context->vkDevice, context->vkSwapchain, &context->vkSwapImageCount, vkSwapImages);
		
		context->vkSwapImageViews = AllocArray(memArena, VkImageView, context->vkSwapImageCount);
		NotNull(context->vkSwapImageViews);
		MyMemSet(context->vkSwapImageViews, 0x00, sizeof(VkImageView) * context->vkSwapImageCount);
		
		DeclareVkImageViewCreateInfo(vkCreateImageViewParams);
		vkCreateImageViewParams.viewType = VK_IMAGE_VIEW_TYPE_2D;
		vkCreateImageViewParams.format = context->vkSurfaceFormat.format;
		vkCreateImageViewParams.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		vkCreateImageViewParams.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		vkCreateImageViewParams.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		vkCreateImageViewParams.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		vkCreateImageViewParams.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		vkCreateImageViewParams.subresourceRange.baseMipLevel = 0;
		vkCreateImageViewParams.subresourceRange.levelCount = 1;
		vkCreateImageViewParams.subresourceRange.baseArrayLayer = 0;
		vkCreateImageViewParams.subresourceRange.layerCount = 1;
		
		for (u32 iIndex = 0; iIndex < context->vkSwapImageCount; iIndex++)
		{
			vkCreateImageViewParams.image = vkSwapImages[iIndex];
			if (vkCreateImageView(context->vkDevice, &vkCreateImageViewParams, context->vkAllocator, &context->vkSwapImageViews[iIndex]) != VK_SUCCESS)
			{
				PigGfx_InitFailure("Vulkan swap imageview creation failed!");
				FreeMem(memArena, vkSwapImages, sizeof(VkImage) * context->vkSwapImageCount);
				PigGfx_DestroyContext_Vulkan(context);
				return nullptr;
			}
		}
		
		FreeMem(memArena, vkSwapImages, sizeof(VkImage) * context->vkSwapImageCount);
	}
	
	gfx->contextCreated = true;
	
	//TODO: Get rid of this once the application is fully dictating the rendering behavior
	// +==============================+
	// |        Create vkTest         |
	// +==============================+
	if (!_CreateVkTestContent(&gfx->context, &gfx->vkTest, memArena))
	{
		PigGfx_DestroyContext_Vulkan(context);
		gfx->contextCreated = false;
		return nullptr;
	}
	
	return &gfx->context;
}

void PigGfx_BeginRendering_Vulkan(bool doClearColor, Color_t clearColor, bool doClearDepth, r32 clearDepth, bool doClearStencil, int clearStencilValue)
{
	NotNull(gfx);
	Assert(gfx->contextCreated);
	GraphicsContext_t* context = &gfx->context;
	VkTestContent_t* content = &gfx->vkTest;
	
	vkWaitForFences(context->vkDevice, 1, &content->activeFences[content->activeSyncIndex], VK_TRUE, UINT64_MAX);
	
	u32 imageIndex = 0;
	vkAcquireNextImageKHR(context->vkDevice, context->vkSwapchain, UINT64_MAX, content->imageAvailableSemaphores[content->activeSyncIndex], VK_NULL_HANDLE, &imageIndex);
	
	if (content->swapImagesFences[imageIndex] != VK_NULL_HANDLE)
	{
		vkWaitForFences(context->vkDevice, 1, &content->swapImagesFences[imageIndex], VK_TRUE, UINT64_MAX);
	}
	content->swapImagesFences[imageIndex] = content->activeFences[content->activeSyncIndex];
	
	VkSemaphore waitSemaphore = content->imageAvailableSemaphores[content->activeSyncIndex];
	VkPipelineStageFlags waitStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	VkSemaphore signalSemaphore = content->renderFinishedSemaphores[content->activeSyncIndex];
	vkResetFences(context->vkDevice, 1, &content->activeFences[content->activeSyncIndex]);
	
	// +==============================+
	// |     Test Render Triangle     |
	// +==============================+
	{
		VkCommandBuffer cmdBuffer = content->commandBuffers[imageIndex];
		
		DeclareVkCommandBufferBeginInfo(vkBeginCommandBufferParams);
		vkBeginCommandBufferParams.pInheritanceInfo = nullptr;
		if (vkBeginCommandBuffer(cmdBuffer, &vkBeginCommandBufferParams) == VK_SUCCESS)
		{
			DeclareVkRenderPassBeginInfo(vkCmdBeginRenderPassParams);
			vkCmdBeginRenderPassParams.renderPass = content->renderPass;
			vkCmdBeginRenderPassParams.framebuffer = content->framebuffers[imageIndex];
			vkCmdBeginRenderPassParams.renderArea.offset = { 0, 0 };
			vkCmdBeginRenderPassParams.renderArea.extent = { (u32)context->swapExtent.width, (u32)context->swapExtent.height };
			vkCmdBeginRenderPassParams.clearValueCount = 1;
			v4 colorVec = ToVec4(clearColor);
			VkClearValue vkClearColor = {{{ colorVec.r, colorVec.g, colorVec.b, colorVec.a }}};
			vkCmdBeginRenderPassParams.pClearValues = &vkClearColor;
			vkCmdBeginRenderPass(cmdBuffer, &vkCmdBeginRenderPassParams, VK_SUBPASS_CONTENTS_INLINE);
			
			vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, content->pipeline);
			
			VkDeviceSize bufferOffset = 0;
			vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &content->vertBuffer, &bufferOffset);
			
			vkCmdDraw(cmdBuffer, 3, 1, 0, imageIndex);
			
			vkCmdEndRenderPass(cmdBuffer);
			
			int endSuccess = vkEndCommandBuffer(cmdBuffer);
			Assert(endSuccess == VK_SUCCESS);
		}
		else
		{
			DebugAssertMsg(false, "vkBeginCommandBuffer failed!");
		}
	}
	
	DeclareVkSubmitInfo(vkQueueSubmitParams);
	vkQueueSubmitParams.waitSemaphoreCount = 1;
	vkQueueSubmitParams.pWaitSemaphores = &waitSemaphore;
	vkQueueSubmitParams.pWaitDstStageMask = &waitStages;
	vkQueueSubmitParams.commandBufferCount = 1;
	vkQueueSubmitParams.pCommandBuffers = &content->commandBuffers[imageIndex];
	vkQueueSubmitParams.signalSemaphoreCount = 1;
	vkQueueSubmitParams.pSignalSemaphores = &signalSemaphore;
	if (vkQueueSubmit(context->vkQueue, 1, &vkQueueSubmitParams, content->activeFences[content->activeSyncIndex]) != VK_SUCCESS)
	{
		AssertMsg(false, "Failed to submit Vulkan queue!");
	}
	
	DeclareVkPresentInfoKHR(vkQueuePresentParams);
	vkQueuePresentParams.waitSemaphoreCount = 1;
	vkQueuePresentParams.pWaitSemaphores = &signalSemaphore;
	vkQueuePresentParams.swapchainCount = 1;
	vkQueuePresentParams.pSwapchains = &context->vkSwapchain;
	vkQueuePresentParams.pImageIndices = &imageIndex;
	vkQueuePresentParams.pResults = nullptr;
	vkQueuePresentKHR(context->vkQueue, &vkQueuePresentParams);
	
	content->activeSyncIndex = (content->activeSyncIndex + 1) % VULKAN_IN_FLIGHT_IMAGE_COUNT;
	
	Win32_SleepForMs(15);
}
