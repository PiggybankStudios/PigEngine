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

bool _FindQueueFamilyThatSupportsPresent(VkPhysicalDevice* device, VkSurfaceKHR* surface, u32* familyIndexOut)
{
	u32 queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(*device, &queueFamilyCount, nullptr);
	if (queueFamilyCount == 0) { return false; }
	
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

bool _CreateVkTestContent(GraphicsContext_t* context, VkTestContent_t* content)
{
	// +==================================+
	// | vkTestBufferMem and vkTestBuffer |
	// +==================================+
	{
		float vertices[] = {
			 0.0f, -0.5f,
			 0.5f,  0.5f,
			-0.5f,  0.5f,
		};
		
		VkBufferCreateInfo bufferCreateInfo;
		{
			bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferCreateInfo.pNext = nullptr;
			bufferCreateInfo.flags = 0;
			bufferCreateInfo.size = sizeof(float) * ArrayCount(vertices);
			bufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
			bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			bufferCreateInfo.queueFamilyIndexCount = 0;
			bufferCreateInfo.pQueueFamilyIndices = nullptr;
		}
		
		if (vkCreateBuffer(context->vkDevice, &bufferCreateInfo, context->vkAllocator, &content->vertBuffer) != VK_SUCCESS)
		{
			PigGfx_InitFailure("Vulkan buffer creation failed!");
			return false;
		}
		
		VkMemoryRequirements testBufferMemReqs;
		vkGetBufferMemoryRequirements(context->vkDevice, content->vertBuffer, &testBufferMemReqs);
		u32 memoryTypeIndex = _FindMemoryType(context->vkPhysicalDevice, testBufferMemReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		VkMemoryAllocateInfo vkAllocInfo;
		{
			vkAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			vkAllocInfo.pNext = nullptr;
			vkAllocInfo.allocationSize = testBufferMemReqs.size;
			vkAllocInfo.memoryTypeIndex = memoryTypeIndex;
		}
		
		if (vkAllocateMemory(context->vkDevice, &vkAllocInfo, context->vkAllocator, &content->vertBufferMem) != VK_SUCCESS)
		{
			PigGfx_InitFailure("Vulkan buffer memory allocation failed!");
			vkDestroyBuffer(context->vkDevice, content->vertBuffer, context->vkAllocator);
			return false;
		}
		
		vkBindBufferMemory(context->vkDevice, content->vertBuffer, content->vertBufferMem, 0);
		
		void* testBufferPntr;
		if (vkMapMemory(context->vkDevice, content->vertBufferMem, 0, VK_WHOLE_SIZE, 0, &testBufferPntr) != VK_SUCCESS)
		{
			PigGfx_InitFailure("Vulkan buffer memory map failed!");
			vkFreeMemory(context->vkDevice, content->vertBufferMem, context->vkAllocator);
			vkDestroyBuffer(context->vkDevice, content->vertBuffer, context->vkAllocator);
			return false;
		}
		
		MyMemCopy(testBufferPntr, &vertices[0], sizeof(float) * ArrayCount(vertices));
		
		VkMappedMemoryRange memoryRange;
		{
			memoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
			memoryRange.pNext = nullptr;
			memoryRange.memory = content->vertBufferMem;
			memoryRange.offset = 0;
			memoryRange.size = VK_WHOLE_SIZE;
		}
		vkFlushMappedMemoryRanges(context->vkDevice, 1, &memoryRange);
		
		vkUnmapMemory(context->vkDevice, content->vertBufferMem);
	}
	
	// +==============================+
	// |         vkRenderPass         |
	// +==============================+
	{
		VkAttachmentDescription colorAttachment;
		{
			colorAttachment.flags = 0;
			colorAttachment.format = context->vkSurfaceFormat.format;
			colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		}
		
		VkAttachmentReference colorAttachmentRef;
		{
			colorAttachmentRef.attachment = 0;
			colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}
		
		VkSubpassDescription subpassDesc;
		{
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
		}
		
		VkRenderPassCreateInfo renderPassCreateInfo;
		{
			renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderPassCreateInfo.pNext = nullptr;
			renderPassCreateInfo.flags = 0;
			renderPassCreateInfo.attachmentCount = 1;
			renderPassCreateInfo.pAttachments = &colorAttachment;
			renderPassCreateInfo.subpassCount = 1;
			renderPassCreateInfo.pSubpasses = &subpassDesc;
			renderPassCreateInfo.dependencyCount = 0;
			renderPassCreateInfo.pDependencies = nullptr;
		}
		
		if (vkCreateRenderPass(context->vkDevice, &renderPassCreateInfo, context->vkAllocator, &content->renderPass) != VK_SUCCESS)
		{
			PigGfx_InitFailure("Vulkan renderpass creation failed!");
			vkFreeMemory(context->vkDevice, content->vertBufferMem, context->vkAllocator);
			vkDestroyBuffer(context->vkDevice, content->vertBuffer, context->vkAllocator);
			return false;
		}
	}
	
	// +===============================+
	// | vkVertShader and vkFragShader |
	// +===============================+
	{
		PlatFileContents_t vertFile;
		if (!Win32_ReadFileContents(NewStr("Resources/Shaders/vulkan_basic.vert.spv"), &vertFile))
		{
			PigGfx_InitFailure("Failed to open/read basic vert shader SPIR-V code from file!");
			vkDestroyRenderPass(context->vkDevice, content->renderPass, context->vkAllocator);
			vkFreeMemory(context->vkDevice, content->vertBufferMem, context->vkAllocator);
			vkDestroyBuffer(context->vkDevice, content->vertBuffer, context->vkAllocator);
			return false;
		}
		PlatFileContents_t fragFile;
		if (!Win32_ReadFileContents(NewStr("Resources/Shaders/vulkan_basic.frag.spv"), &fragFile))
		{
			PigGfx_InitFailure("Failed to open/read basic frag shader SPIR-V code from file!");
			Win32_FreeFileContents(&vertFile);
			vkDestroyRenderPass(context->vkDevice, content->renderPass, context->vkAllocator);
			vkFreeMemory(context->vkDevice, content->vertBufferMem, context->vkAllocator);
			vkDestroyBuffer(context->vkDevice, content->vertBuffer, context->vkAllocator);
			return false;
		}
		
		VkShaderModuleCreateInfo vertCreateInfo;
		{
			vertCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			vertCreateInfo.pNext = nullptr;
			vertCreateInfo.flags = 0;
			vertCreateInfo.codeSize = vertFile.size;
			vertCreateInfo.pCode = (u32*)vertFile.data;
		}
		if (vkCreateShaderModule(context->vkDevice, &vertCreateInfo, context->vkAllocator, &content->vertShader) != VK_SUCCESS)
		{
			PigGfx_InitFailure("Vulkan vert shader module creation failed!");
			Win32_FreeFileContents(&vertFile);
			Win32_FreeFileContents(&fragFile);
			vkDestroyRenderPass(context->vkDevice, content->renderPass, context->vkAllocator);
			vkFreeMemory(context->vkDevice, content->vertBufferMem, context->vkAllocator);
			vkDestroyBuffer(context->vkDevice, content->vertBuffer, context->vkAllocator);
			return false;
		}
		
		VkShaderModuleCreateInfo fragCreateInfo;
		{
			fragCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			fragCreateInfo.pNext = nullptr;
			fragCreateInfo.flags = 0;
			fragCreateInfo.codeSize = fragFile.size;
			fragCreateInfo.pCode = (u32*)fragFile.data;
		}
		if (vkCreateShaderModule(context->vkDevice, &fragCreateInfo, context->vkAllocator, &content->fragShader) != VK_SUCCESS)
		{
			PigGfx_InitFailure("Vulkan frag shader module creation failed!");
			vkDestroyShaderModule(context->vkDevice, content->vertShader, context->vkAllocator);
			Win32_FreeFileContents(&vertFile);
			Win32_FreeFileContents(&fragFile);
			vkDestroyRenderPass(context->vkDevice, content->renderPass, context->vkAllocator);
			vkFreeMemory(context->vkDevice, content->vertBufferMem, context->vkAllocator);
			vkDestroyBuffer(context->vkDevice, content->vertBuffer, context->vkAllocator);
			return false;
		}
		
		Win32_FreeFileContents(&vertFile);
		Win32_FreeFileContents(&fragFile);
	}
	
	// +==============================+
	// |       vkPipelineLayout       |
	// +==============================+
	{
		VkPipelineLayoutCreateInfo layoutCreateInfo;
		{
			layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			layoutCreateInfo.pNext = nullptr;
			layoutCreateInfo.flags = 0;
			layoutCreateInfo.setLayoutCount = 0;
			layoutCreateInfo.pSetLayouts = nullptr;
			layoutCreateInfo.pushConstantRangeCount = 0;
		}
		
		if (vkCreatePipelineLayout(context->vkDevice, &layoutCreateInfo, context->vkAllocator, &content->pipelineLayout) != VK_SUCCESS)
		{
			PigGfx_InitFailure("Vulkan pipeline layout creation failed!");
			vkDestroyShaderModule(context->vkDevice, content->fragShader, context->vkAllocator);
			vkDestroyShaderModule(context->vkDevice, content->vertShader, context->vkAllocator);
			vkDestroyRenderPass(context->vkDevice, content->renderPass, context->vkAllocator);
			vkFreeMemory(context->vkDevice, content->vertBufferMem, context->vkAllocator);
			vkDestroyBuffer(context->vkDevice, content->vertBuffer, context->vkAllocator);
			return false;
		}
	}
	
	// +==============================+
	// |          vkPipeline          |
	// +==============================+
	{
		VkPipelineShaderStageCreateInfo vertShaderStageInfo;
		{
			vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			vertShaderStageInfo.pNext = nullptr;
			vertShaderStageInfo.flags = 0;
			vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
			vertShaderStageInfo.module = content->vertShader;
			vertShaderStageInfo.pName = "main";
			vertShaderStageInfo.pSpecializationInfo = nullptr;
		}
		VkPipelineShaderStageCreateInfo fragShaderStageInfo;
		{
			fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			fragShaderStageInfo.pNext = nullptr;
			fragShaderStageInfo.flags = 0;
			fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			fragShaderStageInfo.module = content->fragShader;
			fragShaderStageInfo.pName = "main";
			fragShaderStageInfo.pSpecializationInfo = nullptr;
		}
		VkPipelineShaderStageCreateInfo shaderStageCreateInfos[] = { vertShaderStageInfo, fragShaderStageInfo };
		
		VkVertexInputBindingDescription v2VertexBinding;
		{
			v2VertexBinding.binding = 0;
			v2VertexBinding.stride = sizeof(float) * 2;
			v2VertexBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		}
		VkVertexInputAttributeDescription positionVertAttribDesc;
		{
			positionVertAttribDesc.binding = 0;
			positionVertAttribDesc.location = 0; //Location in the vertex shader
			positionVertAttribDesc.format = VK_FORMAT_R32G32_SFLOAT;
			positionVertAttribDesc.offset = 0;
		}
		VkPipelineVertexInputStateCreateInfo vertexInputInfo;
		{
			vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			vertexInputInfo.pNext = nullptr;
			vertexInputInfo.flags = 0;
			vertexInputInfo.vertexBindingDescriptionCount = 1;
			vertexInputInfo.pVertexBindingDescriptions = &v2VertexBinding;
			vertexInputInfo.vertexAttributeDescriptionCount = 1;
			vertexInputInfo.pVertexAttributeDescriptions = &positionVertAttribDesc;
		}
		
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
		{
			inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			inputAssemblyInfo.pNext = nullptr;
			inputAssemblyInfo.flags = 0;
			inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;
		}
		
		VkViewport viewport;
		{
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = (float)context->swapExtent.width;
			viewport.height = (float)context->swapExtent.height;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
		}
		VkRect2D scissor;
		{
			scissor.offset = { 0, 0 };
			scissor.extent = context->swapExtent;
		}
		VkPipelineViewportStateCreateInfo viewportCreateInfo;
		{
			viewportCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			viewportCreateInfo.pNext = nullptr;
			viewportCreateInfo.flags = 0;
			viewportCreateInfo.viewportCount = 1;
			viewportCreateInfo.pViewports = &viewport;
			viewportCreateInfo.scissorCount = 1;
			viewportCreateInfo.pScissors = &scissor;
		}
		
		VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo;
		{
			rasterizerCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			rasterizerCreateInfo.pNext = nullptr;
			rasterizerCreateInfo.flags = 0;
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
		}
		
		VkPipelineMultisampleStateCreateInfo multisamplingCreateInfo;
		{
			multisamplingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			multisamplingCreateInfo.pNext = nullptr;
			multisamplingCreateInfo.flags = 0;
			multisamplingCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
			multisamplingCreateInfo.sampleShadingEnable = VK_FALSE;
			multisamplingCreateInfo.minSampleShading = 0.0f;
			multisamplingCreateInfo.pSampleMask = nullptr;
			multisamplingCreateInfo.alphaToCoverageEnable = VK_FALSE;
			multisamplingCreateInfo.alphaToOneEnable = VK_FALSE;
		}
		
		VkPipelineColorBlendAttachmentState colorBlendAttachment;
		{
			colorBlendAttachment.blendEnable = VK_FALSE;
			colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
			colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
			colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
			colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
			colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		}
		VkPipelineColorBlendStateCreateInfo colorBlendCreateInfo;
		{
			colorBlendCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			colorBlendCreateInfo.pNext = nullptr;
			colorBlendCreateInfo.flags = 0;
			colorBlendCreateInfo.logicOpEnable = VK_FALSE;
			colorBlendCreateInfo.logicOp = VK_LOGIC_OP_COPY;
			colorBlendCreateInfo.attachmentCount = 1;
			colorBlendCreateInfo.pAttachments = &colorBlendAttachment;
			colorBlendCreateInfo.blendConstants[0] = 0.0f;
			colorBlendCreateInfo.blendConstants[1] = 0.0f;
			colorBlendCreateInfo.blendConstants[2] = 0.0f;
			colorBlendCreateInfo.blendConstants[3] = 0.0f;
		}
		
		VkGraphicsPipelineCreateInfo pipelineCreateInfo;
		{
			pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			pipelineCreateInfo.pNext = nullptr;
			pipelineCreateInfo.flags = 0;
			pipelineCreateInfo.stageCount = ArrayCount(shaderStageCreateInfos);
			pipelineCreateInfo.pStages = &shaderStageCreateInfos[0];
			pipelineCreateInfo.pVertexInputState = &vertexInputInfo;
			pipelineCreateInfo.pInputAssemblyState = &inputAssemblyInfo;
			pipelineCreateInfo.pTessellationState = nullptr;
			pipelineCreateInfo.pViewportState = &viewportCreateInfo;
			pipelineCreateInfo.pRasterizationState = &rasterizerCreateInfo;
			pipelineCreateInfo.pMultisampleState = &multisamplingCreateInfo;
			pipelineCreateInfo.pDepthStencilState = nullptr;
			pipelineCreateInfo.pColorBlendState = &colorBlendCreateInfo;
			pipelineCreateInfo.pDynamicState = nullptr;
			pipelineCreateInfo.layout = content->pipelineLayout;
			pipelineCreateInfo.renderPass = content->renderPass;
			pipelineCreateInfo.subpass = 0;
			pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
			pipelineCreateInfo.basePipelineIndex = 0;
		}
		
		if (vkCreateGraphicsPipelines(context->vkDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, context->vkAllocator, &content->pipeline) != VK_SUCCESS)
		{
			PigGfx_InitFailure("Vulkan pipeline creation failed!");
			vkDestroyPipelineLayout(context->vkDevice, content->pipelineLayout, context->vkAllocator);
			vkDestroyShaderModule(context->vkDevice, content->fragShader, context->vkAllocator);
			vkDestroyShaderModule(context->vkDevice, content->vertShader, context->vkAllocator);
			vkDestroyRenderPass(context->vkDevice, content->renderPass, context->vkAllocator);
			vkFreeMemory(context->vkDevice, content->vertBufferMem, context->vkAllocator);
			vkDestroyBuffer(context->vkDevice, content->vertBuffer, context->vkAllocator);
			return false;
		}
	}
	
	// +==============================+
	// |        vkFramebuffers        |
	// +==============================+
	{
		content->framebufferCount = context->vkSwapImageCount;
		content->framebuffers = AllocArray(gfx->mainArena, VkFramebuffer, content->framebufferCount);
		NotNull(content->framebuffers);
		
		VkFramebufferCreateInfo framebufferCreateInfo;
		{
			framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferCreateInfo.pNext = nullptr;
			framebufferCreateInfo.flags = 0;
			framebufferCreateInfo.renderPass = content->renderPass;
			framebufferCreateInfo.attachmentCount = 1;
			framebufferCreateInfo.pAttachments = nullptr; // Set in the loop below
			framebufferCreateInfo.width = context->swapExtent.width;
			framebufferCreateInfo.height = context->swapExtent.height;
			framebufferCreateInfo.layers = 1;
		}
		
		for (u32 fIndex = 0; fIndex < content->framebufferCount; fIndex++)
		{
			framebufferCreateInfo.pAttachments = &context->vkSwapImageViews[fIndex];
			if (vkCreateFramebuffer(context->vkDevice, &framebufferCreateInfo, context->vkAllocator, &content->framebuffers[fIndex]) != VK_SUCCESS)
			{
				PigGfx_InitFailure("Vulkan framebuffer creation failed!");
				for (u32 fIndex2 = 0; fIndex2 < fIndex; fIndex2++) { vkDestroyFramebuffer(context->vkDevice, content->framebuffers[fIndex2], context->vkAllocator); }
				FreeMem(gfx->mainArena, content->framebuffers, sizeof(VkFramebuffer) * content->framebufferCount);
				vkDestroyPipeline(context->vkDevice, content->pipeline, context->vkAllocator);
				vkDestroyPipelineLayout(context->vkDevice, content->pipelineLayout, context->vkAllocator);
				vkDestroyShaderModule(context->vkDevice, content->fragShader, context->vkAllocator);
				vkDestroyShaderModule(context->vkDevice, content->vertShader, context->vkAllocator);
				vkDestroyRenderPass(context->vkDevice, content->renderPass, context->vkAllocator);
				vkFreeMemory(context->vkDevice, content->vertBufferMem, context->vkAllocator);
				vkDestroyBuffer(context->vkDevice, content->vertBuffer, context->vkAllocator);
				return false;
			}
		}
	}
	
	// +==============================+
	// |        vkCommandPool         |
	// +==============================+
	{
		VkCommandPoolCreateInfo poolCreateInfo;
		{
			poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			poolCreateInfo.pNext = nullptr;
			poolCreateInfo.flags = 0;
			poolCreateInfo.queueFamilyIndex = context->queueFamilyIndex;
		}
		
		if (vkCreateCommandPool(context->vkDevice, &poolCreateInfo, context->vkAllocator, &content->commandPool) != VK_SUCCESS)
		{
			PigGfx_InitFailure("Vulkan command pool creation failed!");
			for (u32 fIndex = 0; fIndex < content->framebufferCount; fIndex++) { vkDestroyFramebuffer(context->vkDevice, content->framebuffers[fIndex], context->vkAllocator); }
			FreeMem(gfx->mainArena, content->framebuffers, sizeof(VkFramebuffer) * content->framebufferCount);
			vkDestroyPipeline(context->vkDevice, content->pipeline, context->vkAllocator);
			vkDestroyPipelineLayout(context->vkDevice, content->pipelineLayout, context->vkAllocator);
			vkDestroyShaderModule(context->vkDevice, content->fragShader, context->vkAllocator);
			vkDestroyShaderModule(context->vkDevice, content->vertShader, context->vkAllocator);
			vkDestroyRenderPass(context->vkDevice, content->renderPass, context->vkAllocator);
			vkFreeMemory(context->vkDevice, content->vertBufferMem, context->vkAllocator);
			vkDestroyBuffer(context->vkDevice, content->vertBuffer, context->vkAllocator);
			return false;
		}
	}
	
	// +==============================+
	// |       vkCommandBuffers       |
	// +==============================+
	{
		content->commandBufferCount = context->vkSwapImageCount;
		content->commandBuffers = AllocArray(gfx->mainArena, VkCommandBuffer, content->commandBufferCount);
		NotNull(content->commandBuffers);
		
		VkCommandBufferAllocateInfo allocInfo;
		{
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.pNext = nullptr;
			// allocInfo.flags = 0; //TODO: there is no flags on this struct???
			allocInfo.commandPool = content->commandPool;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandBufferCount = content->commandBufferCount;
		}
		
		if (vkAllocateCommandBuffers(context->vkDevice, &allocInfo, &content->commandBuffers[0]) != VK_SUCCESS)
		{
			PigGfx_InitFailure("Vulkan command buffer allocation failed!");
			FreeMem(gfx->mainArena, content->commandBuffers, sizeof(VkCommandBuffer) * content->commandBufferCount);
			vkDestroyCommandPool(context->vkDevice, content->commandPool, context->vkAllocator);
			for (u32 fIndex = 0; fIndex < content->framebufferCount; fIndex++) { vkDestroyFramebuffer(context->vkDevice, content->framebuffers[fIndex], context->vkAllocator); }
			FreeMem(gfx->mainArena, content->framebuffers, sizeof(VkFramebuffer) * content->framebufferCount);
			vkDestroyPipeline(context->vkDevice, content->pipeline, context->vkAllocator);
			vkDestroyPipelineLayout(context->vkDevice, content->pipelineLayout, context->vkAllocator);
			vkDestroyShaderModule(context->vkDevice, content->fragShader, context->vkAllocator);
			vkDestroyShaderModule(context->vkDevice, content->vertShader, context->vkAllocator);
			vkDestroyRenderPass(context->vkDevice, content->renderPass, context->vkAllocator);
			vkFreeMemory(context->vkDevice, content->vertBufferMem, context->vkAllocator);
			vkDestroyBuffer(context->vkDevice, content->vertBuffer, context->vkAllocator);
			return false;
		}
	}
	
	// +==============================+
	// |    Begin Command Buffers     |
	// +==============================+
	for (u32 bIndex = 0; bIndex < content->commandBufferCount; bIndex++)
	{
		VkCommandBufferBeginInfo beginInfo;
		{
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.pNext = nullptr;
			beginInfo.flags = 0;
			beginInfo.pInheritanceInfo = nullptr;
		}
		
		if (vkBeginCommandBuffer(content->commandBuffers[bIndex], &beginInfo) != VK_SUCCESS)
		{
			PigGfx_InitFailure("Failed to begin command buffer!");
			vkFreeCommandBuffers(context->vkDevice, content->commandPool, content->commandBufferCount, &content->commandBuffers[0]);
			FreeMem(gfx->mainArena, content->commandBuffers, sizeof(VkCommandBuffer) * content->commandBufferCount);
			vkDestroyCommandPool(context->vkDevice, content->commandPool, context->vkAllocator);
			for (u32 fIndex = 0; fIndex < content->framebufferCount; fIndex++) { vkDestroyFramebuffer(context->vkDevice, content->framebuffers[fIndex], context->vkAllocator); }
			FreeMem(gfx->mainArena, content->framebuffers, sizeof(VkFramebuffer) * content->framebufferCount);
			vkDestroyPipeline(context->vkDevice, content->pipeline, context->vkAllocator);
			vkDestroyPipelineLayout(context->vkDevice, content->pipelineLayout, context->vkAllocator);
			vkDestroyShaderModule(context->vkDevice, content->fragShader, context->vkAllocator);
			vkDestroyShaderModule(context->vkDevice, content->vertShader, context->vkAllocator);
			vkDestroyRenderPass(context->vkDevice, content->renderPass, context->vkAllocator);
			vkFreeMemory(context->vkDevice, content->vertBufferMem, context->vkAllocator);
			vkDestroyBuffer(context->vkDevice, content->vertBuffer, context->vkAllocator);
			return false;
		}
	}
	
	// +==============================+
	// |     Fill Command Buffers     |
	// +==============================+
	for (u32 bIndex = 0; bIndex < content->commandBufferCount; bIndex++)
	{
		VkCommandBuffer cmdBuffer = content->commandBuffers[bIndex];
		
		VkClearValue clearColor = {{{ 0.0f, 0.0f, 0.0f, 1.0f }}};
		VkRenderPassBeginInfo renderPassInfo;
		{
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.pNext = nullptr;
			renderPassInfo.renderPass = content->renderPass;
			renderPassInfo.framebuffer = content->framebuffers[bIndex];
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = { (u32)context->swapExtent.width, (u32)context->swapExtent.height };
			renderPassInfo.clearValueCount = 1;
			renderPassInfo.pClearValues = &clearColor;
		}
		
		vkCmdBeginRenderPass(cmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		
		vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, content->pipeline);
		
		VkDeviceSize bufferOffset = 0;
		vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &content->vertBuffer, &bufferOffset);
		
		vkCmdDraw(cmdBuffer, 3, 1, 0, bIndex);
		
		vkCmdEndRenderPass(cmdBuffer);
	}
	
	// +==============================+
	// |     End Command Buffers      |
	// +==============================+
	for (u32 bIndex = 0; bIndex < content->commandBufferCount; bIndex++)
	{
		if (vkEndCommandBuffer(content->commandBuffers[bIndex]) != VK_SUCCESS)
		{
			PigGfx_InitFailure("Failed to end command buffer!");
			vkFreeCommandBuffers(context->vkDevice, content->commandPool, content->commandBufferCount, &content->commandBuffers[0]);
			FreeMem(gfx->mainArena, content->commandBuffers, sizeof(VkCommandBuffer) * content->commandBufferCount);
			vkDestroyCommandPool(context->vkDevice, content->commandPool, context->vkAllocator);
			for (u32 fIndex = 0; fIndex < content->framebufferCount; fIndex++) { vkDestroyFramebuffer(context->vkDevice, content->framebuffers[fIndex], context->vkAllocator); }
			FreeMem(gfx->mainArena, content->framebuffers, sizeof(VkFramebuffer) * content->framebufferCount);
			vkDestroyPipeline(context->vkDevice, content->pipeline, context->vkAllocator);
			vkDestroyPipelineLayout(context->vkDevice, content->pipelineLayout, context->vkAllocator);
			vkDestroyShaderModule(context->vkDevice, content->fragShader, context->vkAllocator);
			vkDestroyShaderModule(context->vkDevice, content->vertShader, context->vkAllocator);
			vkDestroyRenderPass(context->vkDevice, content->renderPass, context->vkAllocator);
			vkFreeMemory(context->vkDevice, content->vertBufferMem, context->vkAllocator);
			vkDestroyBuffer(context->vkDevice, content->vertBuffer, context->vkAllocator);
			return false;
		}
	}
	
	// +==============================+
	// |    Semaphores and Fences     |
	// +==============================+
	{
		VkSemaphoreCreateInfo semaphoreCreateInfo;
		{
			semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			semaphoreCreateInfo.pNext = nullptr;
			semaphoreCreateInfo.flags = 0;
		}
		
		VkFenceCreateInfo fenceCreateInfo;
		{
			fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceCreateInfo.pNext = nullptr;
			fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		}
		
		for (u32 iIndex = 0; iIndex < VULKAN_IN_FLIGHT_IMAGE_COUNT; iIndex++)
		{
			content->swapImagesFences[iIndex] = VK_NULL_HANDLE;
			if (vkCreateSemaphore(context->vkDevice, &semaphoreCreateInfo, context->vkAllocator, &content->imageAvailableSemaphores[iIndex]) != VK_SUCCESS ||
				vkCreateSemaphore(context->vkDevice, &semaphoreCreateInfo, context->vkAllocator, &content->renderFinishedSemaphores[iIndex]) != VK_SUCCESS ||
				vkCreateFence(context->vkDevice, &fenceCreateInfo, context->vkAllocator, &content->activeFences[iIndex]) != VK_SUCCESS)
			{
				PigGfx_InitFailure("Vulkan semaphore/fence creation failed!");
				vkFreeCommandBuffers(context->vkDevice, content->commandPool, content->commandBufferCount, &content->commandBuffers[0]);
				FreeMem(gfx->mainArena, content->commandBuffers, sizeof(VkCommandBuffer) * content->commandBufferCount);
				vkDestroyCommandPool(context->vkDevice, content->commandPool, context->vkAllocator);
				for (u32 fIndex = 0; fIndex < content->framebufferCount; fIndex++) { vkDestroyFramebuffer(context->vkDevice, content->framebuffers[fIndex], context->vkAllocator); }
				FreeMem(gfx->mainArena, content->framebuffers, sizeof(VkFramebuffer) * content->framebufferCount);
				vkDestroyPipeline(context->vkDevice, content->pipeline, context->vkAllocator);
				vkDestroyPipelineLayout(context->vkDevice, content->pipelineLayout, context->vkAllocator);
				vkDestroyShaderModule(context->vkDevice, content->fragShader, context->vkAllocator);
				vkDestroyShaderModule(context->vkDevice, content->vertShader, context->vkAllocator);
				vkDestroyRenderPass(context->vkDevice, content->renderPass, context->vkAllocator);
				vkFreeMemory(context->vkDevice, content->vertBufferMem, context->vkAllocator);
				vkDestroyBuffer(context->vkDevice, content->vertBuffer, context->vkAllocator);
				return false;
			}
		}
		
		content->activeSyncIndex = 0;
	}
	
	return true;
}

GraphicsContext_t* PigGfx_CreateContext_Vulkan()
{
	NotNull(gfx);
	Assert(!gfx->contextCreated);
	Assert(gfx->optionsSet);
	NotNull(gfx->currentGlfwWindow);
	NotNull(gfx->mainArena);
	
	//TODO: Implement this!
	VkAllocationCallbacks* vkAllocator = nullptr;
	
	VkInstance vkInstance;
	{
		u32 instanceCreationExtensionCount = 0;
		const char** instanceCreationExtensions = glfwGetRequiredInstanceExtensions(&instanceCreationExtensionCount);
		Assert(instanceCreationExtensions != nullptr);
		
		VkApplicationInfo applicationInfo = {};
		applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		applicationInfo.pNext = nullptr;
		applicationInfo.pApplicationName = gfx->options.vulkan_ApplicationName;
		applicationInfo.applicationVersion = gfx->options.vulkan_ApplicationVersionInt;
		applicationInfo.pEngineName = gfx->options.vulkan_EngineName;
		applicationInfo.engineVersion = gfx->options.vulkan_EngineVersionInt;
		applicationInfo.apiVersion = VK_MAKE_API_VERSION(0, gfx->options.vulkan_RequestVersionMajor, gfx->options.vulkan_RequestVersionMinor, 0);
		
		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.pApplicationInfo = &applicationInfo;
		const char* vulkanLayers[] = { "VK_LAYER_KHRONOS_validation" };
		createInfo.enabledLayerCount = ArrayCount(vulkanLayers);
		createInfo.ppEnabledLayerNames = (ArrayCount(vulkanLayers) > 0) ? &vulkanLayers[0] : nullptr;
		createInfo.enabledExtensionCount = instanceCreationExtensionCount;
		createInfo.ppEnabledExtensionNames = instanceCreationExtensions;
		
		if (vkCreateInstance(&createInfo, vkAllocator, &vkInstance) != VK_SUCCESS) { PigGfx_InitFailure("Vulkan instance creation failed!"); return nullptr; }
	}
	
	VkSurfaceKHR vkSurface = {};
	if (glfwCreateWindowSurface(vkInstance, gfx->currentGlfwWindow, vkAllocator, &vkSurface) != VK_SUCCESS)
	{
		PigGfx_InitFailure("Vulkan window surface creation failed!");
		vkDestroyInstance(vkInstance, vkAllocator);
		return nullptr;
	}
	
	VkPhysicalDevice vkPhysicalDevice = {};
	u32 queueFamilyIndex = 0;
	{
		u32 deviceCount;
		vkEnumeratePhysicalDevices(vkInstance, &deviceCount, nullptr);
		if (deviceCount == 0)
		{
			PigGfx_InitFailure("Vulkan physical device enumeration failed");
			vkDestroySurfaceKHR(vkInstance, vkSurface, vkAllocator);
			vkDestroyInstance(vkInstance, vkAllocator);
			return nullptr;
		}
		
		VkPhysicalDevice* allDevices = AllocArray(gfx->mainArena, VkPhysicalDevice, deviceCount);
		NotNull(allDevices);
		vkEnumeratePhysicalDevices(vkInstance, &deviceCount, allDevices);
		
		bool foundDeviceWithQueue = false;
		for (u32 dIndex = 0; dIndex < deviceCount; dIndex++)
		{
			VkPhysicalDevice* device = &allDevices[dIndex];
			if (_FindQueueFamilyThatSupportsPresent(device, &vkSurface, &queueFamilyIndex))
			{
				vkPhysicalDevice = *device;
				foundDeviceWithQueue = true;
				break;
			}
		}
		
		FreeMem(gfx->mainArena, allDevices, sizeof(VkPhysicalDevice) * deviceCount);
		
		if (!foundDeviceWithQueue)
		{
			PigGfx_InitFailure("Couldn't find physical device with appropriate queue capabilities (enumerating in Vulkan)");
			vkDestroySurfaceKHR(vkInstance, vkSurface, vkAllocator);
			vkDestroyInstance(vkInstance, vkAllocator);
			return nullptr;
		}
	}
	
	VkDevice vkDevice;
	VkQueue vkQueue;
	{
		r32 queuePriority = 1.0f;
		VkDeviceQueueCreateInfo queueCreateInfo;
		{
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.pNext = nullptr;
			queueCreateInfo.flags = 0;
			queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
		}
		
		VkDeviceCreateInfo deviceCreateInfo;
		{
			deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			deviceCreateInfo.pNext = nullptr;
			deviceCreateInfo.flags = 0;
			deviceCreateInfo.queueCreateInfoCount = 1;
			deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
			deviceCreateInfo.pEnabledFeatures = nullptr;
			const char* enabledExtensionNames[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
			deviceCreateInfo.enabledExtensionCount = ArrayCount(enabledExtensionNames);
			deviceCreateInfo.ppEnabledExtensionNames = &enabledExtensionNames[0];
			const char* enabledLayerNames[] = { "VK_LAYER_KHRONOS_validation" };
			deviceCreateInfo.enabledLayerCount = ArrayCount(enabledLayerNames);
			deviceCreateInfo.ppEnabledLayerNames = &enabledLayerNames[0];
		}
		
		if (vkCreateDevice(vkPhysicalDevice, &deviceCreateInfo, vkAllocator, &vkDevice) != VK_SUCCESS)
		{
			PigGfx_InitFailure("Vulkan device creation failed!");
			vkDestroySurfaceKHR(vkInstance, vkSurface, vkAllocator);
			vkDestroyInstance(vkInstance, vkAllocator);
			return nullptr;
		}
		
		vkGetDeviceQueue(vkDevice, queueFamilyIndex, 0, &vkQueue);
	}
	
	int windowWidth, windowHeight;
	glfwGetWindowSize(gfx->currentGlfwWindow, &windowWidth, &windowHeight);
	VkExtent2D swapExtent = { (u32)windowWidth, (u32)windowHeight };
	
	VkSurfaceFormatKHR vkSurfaceFormat = {};
	VkSwapchainKHR vkSwapchain;
	{
		VkSurfaceCapabilitiesKHR surfaceCapabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkPhysicalDevice, vkSurface, &surfaceCapabilities);
		if (surfaceCapabilities.currentExtent.width != UINT32_MAX)
		{
			swapExtent = surfaceCapabilities.currentExtent;
		}
		
		u32 formatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice, vkSurface, &formatCount, nullptr);
		Assert(formatCount > 0);
		VkSurfaceFormatKHR* allFormats = AllocArray(gfx->mainArena, VkSurfaceFormatKHR, formatCount);
		NotNull(allFormats);
		vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice, vkSurface, &formatCount, &allFormats[0]);
		
		bool foundDesiredFormat = false;
		for (u32 fIndex = 0; fIndex < formatCount; fIndex++)
		{
			VkSurfaceFormatKHR* format = &allFormats[fIndex];
			if (format->format == VK_FORMAT_B8G8R8A8_SRGB && format->colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				vkSurfaceFormat = *format;
				foundDesiredFormat = true;
				break;
			}
		}
		
		FreeMem(gfx->mainArena, allFormats, sizeof(VkSurfaceFormatKHR) * formatCount);
		
		if (!foundDesiredFormat)
		{
			PigGfx_InitFailure("Couldn't find a supported surface format on the (physical device chosen) that matches the format we expect");
			vkDestroyDevice(vkDevice, vkAllocator);
			vkDestroySurfaceKHR(vkInstance, vkSurface, vkAllocator);
			vkDestroyInstance(vkInstance, vkAllocator);
			return nullptr;
		}
		
		VkSwapchainCreateInfoKHR swapchainCreateInfo;
		{
			swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
			swapchainCreateInfo.pNext = nullptr;
			swapchainCreateInfo.flags = 0;
			swapchainCreateInfo.surface = vkSurface;
			swapchainCreateInfo.minImageCount = surfaceCapabilities.minImageCount + 1;
			swapchainCreateInfo.imageFormat = vkSurfaceFormat.format;
			swapchainCreateInfo.imageColorSpace = vkSurfaceFormat.colorSpace;
			swapchainCreateInfo.imageExtent = swapExtent;
			swapchainCreateInfo.imageArrayLayers = 1;
			swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
			swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			swapchainCreateInfo.queueFamilyIndexCount = 0;
			swapchainCreateInfo.pQueueFamilyIndices = nullptr;
			swapchainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
			swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
			swapchainCreateInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
			swapchainCreateInfo.clipped = VK_TRUE;
			swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;
		}
		
		if (vkCreateSwapchainKHR(vkDevice, &swapchainCreateInfo, vkAllocator, &vkSwapchain) != VK_SUCCESS)
		{
			PigGfx_InitFailure("Vulkan swapchain creation failed!");
			vkDestroyDevice(vkDevice, vkAllocator);
			vkDestroySurfaceKHR(vkInstance, vkSurface, vkAllocator);
			vkDestroyInstance(vkInstance, vkAllocator);
			return nullptr;
		}
	}
	
	u32 vkSwapImageCount = 0;
	VkImage* vkSwapImages = nullptr;
	VkImageView* vkSwapImageViews = nullptr;
	{
		vkGetSwapchainImagesKHR(vkDevice, vkSwapchain, &vkSwapImageCount, nullptr);
		Assert(vkSwapImageCount > 0);
		vkSwapImages = AllocArray(gfx->mainArena, VkImage, vkSwapImageCount);
		NotNull(vkSwapImages);
		vkGetSwapchainImagesKHR(vkDevice, vkSwapchain, &vkSwapImageCount, vkSwapImages);
		
		vkSwapImageViews = AllocArray(gfx->mainArena, VkImageView, vkSwapImageCount);
		NotNull(vkSwapImageViews);
		
		VkImageViewCreateInfo imageViewCreateInfo;
		{
			imageViewCreateInfo.sType= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			imageViewCreateInfo.pNext = nullptr;
			imageViewCreateInfo.flags = 0;
			imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			imageViewCreateInfo.format = vkSurfaceFormat.format;
			imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
			imageViewCreateInfo.subresourceRange.levelCount = 1;
			imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
			imageViewCreateInfo.subresourceRange.layerCount = 1;
		}
		
		for (u32 iIndex = 0; iIndex < vkSwapImageCount; iIndex++)
		{
			imageViewCreateInfo.image = vkSwapImages[iIndex];
			if (vkCreateImageView(vkDevice, &imageViewCreateInfo, vkAllocator, &vkSwapImageViews[iIndex]) != VK_SUCCESS)
			{
				PigGfx_InitFailure("Vulkan swap imageview creation failed!");
				FreeMem(gfx->mainArena, vkSwapImageViews, sizeof(VkImageView) * vkSwapImageCount);
				FreeMem(gfx->mainArena, vkSwapImages, sizeof(VkImage) * vkSwapImageCount);
				vkDestroySwapchainKHR(vkDevice, vkSwapchain, vkAllocator);
				vkDestroyDevice(vkDevice, vkAllocator);
				vkDestroySurfaceKHR(vkInstance, vkSurface, vkAllocator);
				vkDestroyInstance(vkInstance, vkAllocator);
				return nullptr;
			}
		}
		
		FreeMem(gfx->mainArena, vkSwapImages, sizeof(VkImage) * vkSwapImageCount);
	}
	
	ClearStruct(gfx->context);
	gfx->context.renderApi = RenderApi_Vulkan;
	gfx->context.vkInstance = vkInstance;
	gfx->context.vkSurfaceFormat = vkSurfaceFormat;
	gfx->context.vkSurface = vkSurface;
	gfx->context.vkPhysicalDevice = vkPhysicalDevice;
	gfx->context.queueFamilyIndex = queueFamilyIndex;
	gfx->context.swapExtent = swapExtent;
	gfx->context.vkDevice = vkDevice;
	gfx->context.vkQueue = vkQueue;
	gfx->context.vkSwapchain = vkSwapchain;
	gfx->context.vkSwapImageCount = vkSwapImageCount;
	gfx->context.vkSwapImages = vkSwapImages;
	gfx->context.vkSwapImageViews = vkSwapImageViews;
	gfx->contextCreated = true;
	
	//TODO: Get rid of this:
	if (!_CreateVkTestContent(&gfx->context, &gfx->vkTest))
	{
		for (u32 iIndex = 0; iIndex < gfx->context.vkSwapImageCount; iIndex++) { vkDestroyImageView(gfx->context.vkDevice, gfx->context.vkSwapImageViews[iIndex], gfx->context.vkAllocator); }
		FreeMem(gfx->mainArena, gfx->context.vkSwapImageViews, sizeof(VkImageView) * gfx->context.vkSwapImageCount);
		vkDestroySwapchainKHR(gfx->context.vkDevice, gfx->context.vkSwapchain, gfx->context.vkAllocator);
		vkDestroyDevice(gfx->context.vkDevice, gfx->context.vkAllocator);
		vkDestroySurfaceKHR(gfx->context.vkInstance, gfx->context.vkSurface, gfx->context.vkAllocator);
		vkDestroyInstance(gfx->context.vkInstance, gfx->context.vkAllocator);
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
	
	VkSubmitInfo submitInfo;
	{
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext = nullptr;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &waitSemaphore;
		submitInfo.pWaitDstStageMask = &waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &content->commandBuffers[imageIndex];
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &signalSemaphore;
	}
	
	vkResetFences(context->vkDevice, 1, &content->activeFences[content->activeSyncIndex]);
	
	if (vkQueueSubmit(context->vkQueue, 1, &submitInfo, content->activeFences[content->activeSyncIndex]) != VK_SUCCESS)
	{
		AssertMsg(false, "Failed to submit Vulkan queue!");
	}
	
	VkPresentInfoKHR presentInfo;
	{
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pNext = nullptr;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &signalSemaphore;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &context->vkSwapchain;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr;
	}
	
	vkQueuePresentKHR(context->vkQueue, &presentInfo);
	
	content->activeSyncIndex = (content->activeSyncIndex + 1) % VULKAN_IN_FLIGHT_IMAGE_COUNT;
	
	Win32_SleepForMs(15);
}
