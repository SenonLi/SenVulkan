#include "Sen_07_Texture.h"

Sen_07_Texture::Sen_07_Texture()
{
	std::cout << "Constructor: Sen_07_Texture()\n\n";
	strWindowName = "Sen Vulkan Texture Tutorial";

	backgroundTextureDiskAddress = "../Images/SunRaise.jpg";
}


Sen_07_Texture::~Sen_07_Texture()
{
	finalizeWidget();

	OutputDebugString("\n\t ~Sen_07_Texture()\n");
}

void Sen_07_Texture::initVulkanApplication()
{
	// Need to be segmented base on pipleStages in this function

	createColorAttachOnlyRenderPass();

	/***************************************/
	createTextureAppDescriptorSetLayout();
	createTextureAppPipeline();
	/***************************************/

	createColorAttachOnlySwapchainFramebuffers();
	createCommandPool();

	/***************************************/
	initBackgroundTextureImage();
	/***************************************/


	createTextureAppVertexBuffer();
	createTriangleIndexBuffer();
	createUniformBuffers();

	createTextureAppDescriptorPool();
	createTextureAppDescriptorSet();

	createTriangleCommandBuffers();

	std::cout << "\n Finish  Sen_07_Texture::initVulkanApplication()\n";
}

void Sen_07_Texture::paintVulkan()
{
	/*******************************************************************************************************************************/
	/*********         Acquire an image from the swap chain                              *******************************************/
	/*******************************************************************************************************************************/
	// Use of a presentable image must occur only after the image is returned by vkAcquireNextImageKHR, and before it is presented by vkQueuePresentKHR.
	// This includes transitioning the image layout and rendering commands.
	uint32_t swapchainImageIndex;
	VkResult result = vkAcquireNextImageKHR(device, swapChain,
		UINT64_MAX,							// timeout for this Image Acquire command, i.e., (std::numeric_limits<uint64_t>::max)(),
		swapchainImageAcquiredSemaphore,	// semaphore to signal
		VK_NULL_HANDLE,						// fence to signal
		&swapchainImageIndex
	);
	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		reCreateRenderTarget();
		return;
	}else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("Failed to acquire swap chain image !!!!");
	}

	/*******************************************************************************************************************************/
	/*********       Execute the command buffer with that image as attachment in the framebuffer           *************************/
	/*******************************************************************************************************************************/
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	std::vector<VkSemaphore> submitInfoWaitSemaphoresVecotr;
	submitInfoWaitSemaphoresVecotr.push_back(swapchainImageAcquiredSemaphore);
	// Commands before this submitInfoWaitDstStageMaskArray stage could be executed before semaphore signaled
	VkPipelineStageFlags submitInfoWaitDstStageMaskArray[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = (uint32_t)submitInfoWaitSemaphoresVecotr.size();
	submitInfo.pWaitSemaphores = submitInfoWaitSemaphoresVecotr.data();
	submitInfo.pWaitDstStageMask = submitInfoWaitDstStageMaskArray;

	submitInfo.commandBufferCount = 1;	// wait for submitInfoCommandBuffersVecotr to be created
	submitInfo.pCommandBuffers = &swapchainCommandBufferVector[swapchainImageIndex];

	std::vector<VkSemaphore> submitInfoSignalSemaphoresVector;
	submitInfoSignalSemaphoresVector.push_back(paintReadyToPresentSemaphore);
	submitInfo.signalSemaphoreCount = (uint32_t)submitInfoSignalSemaphoresVector.size();
	submitInfo.pSignalSemaphores = submitInfoSignalSemaphoresVector.data();

	SenAbstractGLFW::errorCheck(
		vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE),
		std::string("Failed to submit draw command buffer !!!")
	);

	/*******************************************************************************************************************************/
	/*********             Return the image to the swap chain for presentation                **************************************/
	/*******************************************************************************************************************************/
	std::vector<VkSemaphore> presentInfoWaitSemaphoresVector;
	presentInfoWaitSemaphoresVector.push_back(paintReadyToPresentSemaphore);
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = (uint32_t)presentInfoWaitSemaphoresVector.size();
	presentInfo.pWaitSemaphores = presentInfoWaitSemaphoresVector.data();

	VkSwapchainKHR swapChainsArray[] = { swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChainsArray;
	presentInfo.pImageIndices = &swapchainImageIndex;

	result = vkQueuePresentKHR(presentQueue, &presentInfo);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
		reCreateRenderTarget();
	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to present swap chain image !!!");
	}
}

void Sen_07_Texture::reCreateRenderTarget()
{
	createTextureAppPipeline();
	createColorAttachOnlySwapchainFramebuffers();

	createTriangleCommandBuffers();
}

void Sen_07_Texture::finalizeWidget()
{	
	/************************************************************************************************************/
	/******************     Destroy background Memory, ImageView, Image     ***********************************/
	/************************************************************************************************************/
	if (VK_NULL_HANDLE != backgroundTextureImage) {
		vkDestroyImage(device, backgroundTextureImage, nullptr);
		if (VK_NULL_HANDLE != backgroundTextureImageView)  
			vkDestroyImageView(device, backgroundTextureImageView, nullptr);
		if (VK_NULL_HANDLE != texture2DSampler)  
			vkDestroySampler(device, texture2DSampler, nullptr);
		if (VK_NULL_HANDLE != backgroundTextureImageDeviceMemory)
			vkFreeMemory(device, backgroundTextureImageDeviceMemory, nullptr); 	// always try to destroy before free

		backgroundTextureImage				= VK_NULL_HANDLE;
		backgroundTextureImageDeviceMemory	= VK_NULL_HANDLE;
		backgroundTextureImageView			= VK_NULL_HANDLE;
		texture2DSampler					= VK_NULL_HANDLE;
	}
	OutputDebugString("\n\tFinish  Sen_07_Texture::finalizeWidget()\n");
}

void Sen_07_Texture::createTextureAppPipeline()
{
	if (VK_NULL_HANDLE != trianglePipeline) {
		vkDestroyPipeline(device, trianglePipeline, nullptr);
		vkDestroyPipelineLayout(device, trianglePipelineLayout, nullptr);

		trianglePipeline = VK_NULL_HANDLE;
		trianglePipelineLayout = VK_NULL_HANDLE;
	}

	/****************************************************************************************************************************/
	/**********                Reserve pipeline ShaderStage CreateInfos Array           *****************************************/
	/****************************************************************************************************************************/
	VkShaderModule vertShaderModule, fragShaderModule;

	createVulkanShaderModule(device, "SenVulkanTutorial/Shaders/textureVert.spv", vertShaderModule);
	createVulkanShaderModule(device, "SenVulkanTutorial/Shaders/textureVert.spv", fragShaderModule);

	VkPipelineShaderStageCreateInfo vertPipelineShaderStageCreateInfo{};
	vertPipelineShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertPipelineShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertPipelineShaderStageCreateInfo.module = vertShaderModule;
	vertPipelineShaderStageCreateInfo.pName = "main"; // shader's entry point name

	VkPipelineShaderStageCreateInfo fragPipelineShaderStageCreateInfo{};
	fragPipelineShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragPipelineShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragPipelineShaderStageCreateInfo.module = fragShaderModule;
	fragPipelineShaderStageCreateInfo.pName = "main"; // shader's entry point name

	std::vector<VkPipelineShaderStageCreateInfo> pipelineShaderStagesCreateInfoVector;
	pipelineShaderStagesCreateInfoVector.push_back(vertPipelineShaderStageCreateInfo);
	pipelineShaderStagesCreateInfoVector.push_back(fragPipelineShaderStageCreateInfo);

	/****************************************************************************************************************************/
	/**********                Reserve pipeline Fixed-Function Stages CreateInfos           *************************************/
	/****************************************************************************************************************************/
	VkVertexInputBindingDescription vertexInputBindingDescription{};
	vertexInputBindingDescription.binding = 0;
	vertexInputBindingDescription.stride = 7 * sizeof(float);
	vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	std::vector<VkVertexInputBindingDescription> vertexInputBindingDescriptionVector;
	vertexInputBindingDescriptionVector.push_back(vertexInputBindingDescription);


	std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescriptionVector;

	VkVertexInputAttributeDescription positionVertexInputAttributeDescription;
	positionVertexInputAttributeDescription.location	= 0;
	positionVertexInputAttributeDescription.binding		= 0;
	positionVertexInputAttributeDescription.format		= VK_FORMAT_R32G32_SFLOAT;
	positionVertexInputAttributeDescription.offset		= 0;
	vertexInputAttributeDescriptionVector.push_back(positionVertexInputAttributeDescription);

	VkVertexInputAttributeDescription colorVertexInputAttributeDescription;
	colorVertexInputAttributeDescription.location		= 1;
	colorVertexInputAttributeDescription.binding		= 0;
	colorVertexInputAttributeDescription.format			= VK_FORMAT_R32G32B32_SFLOAT;
	colorVertexInputAttributeDescription.offset			= 2 * sizeof(float);
	vertexInputAttributeDescriptionVector.push_back(colorVertexInputAttributeDescription);

	VkVertexInputAttributeDescription texCoordVertexInputAttributeDescription;
	texCoordVertexInputAttributeDescription.location	= 2;
	texCoordVertexInputAttributeDescription.binding		= 0;
	texCoordVertexInputAttributeDescription.format		= VK_FORMAT_R32G32_SFLOAT;
	texCoordVertexInputAttributeDescription.offset		= 5 * sizeof(float);
	vertexInputAttributeDescriptionVector.push_back(texCoordVertexInputAttributeDescription);

	VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo{};
	pipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	pipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount	= vertexInputBindingDescriptionVector.size();// spacing between data && whether the data is per-vertex or per-instance (geometry instancing)
	pipelineVertexInputStateCreateInfo.pVertexBindingDescriptions		= vertexInputBindingDescriptionVector.data();
	pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount	= vertexInputAttributeDescriptionVector.size();
	pipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions		= vertexInputAttributeDescriptionVector.data();


	VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo{};
	pipelineInputAssemblyStateCreateInfo.sType					= VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	pipelineInputAssemblyStateCreateInfo.topology				= VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	pipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

	/*********************************************************************************************/
	/*********************************************************************************************/
	VkViewport viewport{};
	viewport.x			= 0.0f;									viewport.y			= 0.0f;
	viewport.width		= static_cast<float>(widgetWidth);		viewport.height		= static_cast<float>(widgetHeight);
	viewport.minDepth	= 0.0f;									viewport.maxDepth	= 1.0f;
	VkRect2D scissorRect2D{};
	scissorRect2D.offset		= { 0, 0 };
	scissorRect2D.extent.width	= static_cast<uint32_t>(widgetWidth);
	scissorRect2D.extent.height = static_cast<uint32_t>(widgetHeight);

	VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo{};
	pipelineViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	pipelineViewportStateCreateInfo.viewportCount = 1;
	pipelineViewportStateCreateInfo.pViewports = &viewport;
	pipelineViewportStateCreateInfo.scissorCount = 1;
	pipelineViewportStateCreateInfo.pScissors = &scissorRect2D;

	/*********************************************************************************************/
	/*********************************************************************************************/
	VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo{};
	pipelineRasterizationStateCreateInfo.sType						= VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	pipelineRasterizationStateCreateInfo.depthClampEnable			= VK_FALSE;
	pipelineRasterizationStateCreateInfo.rasterizerDiscardEnable	= VK_FALSE;
	pipelineRasterizationStateCreateInfo.polygonMode				= VK_POLYGON_MODE_FILL;
	pipelineRasterizationStateCreateInfo.cullMode					= VK_CULL_MODE_NONE;// VK_CULL_MODE_BACK_BIT;
	pipelineRasterizationStateCreateInfo.frontFace					= VK_FRONT_FACE_COUNTER_CLOCKWISE;
	pipelineRasterizationStateCreateInfo.depthBiasEnable			= VK_FALSE;
	pipelineRasterizationStateCreateInfo.lineWidth					= 1.0f;

	/*********************************************************************************************/
	/*********************************************************************************************/
	VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo{}; // for anti-aliasing
	pipelineMultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	pipelineMultisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
	pipelineMultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	/*********************************************************************************************/
	/*********************************************************************************************/
	std::vector<VkPipelineColorBlendAttachmentState> pipelineColorBlendAttachmentStateVector; // for multi-framebuffer rendering
	VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState{};
	pipelineColorBlendAttachmentState.colorWriteMask	= VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
															| VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	pipelineColorBlendAttachmentState.blendEnable		= VK_FALSE;
	pipelineColorBlendAttachmentStateVector.push_back(pipelineColorBlendAttachmentState);

	VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo{};
	pipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	pipelineColorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
	//pipelineColorBlendStateCreateInfo.logicOp						= VK_LOGIC_OP_COPY;
	pipelineColorBlendStateCreateInfo.attachmentCount = (uint32_t)pipelineColorBlendAttachmentStateVector.size();
	pipelineColorBlendStateCreateInfo.pAttachments = pipelineColorBlendAttachmentStateVector.data();
	pipelineColorBlendStateCreateInfo.blendConstants[0] = 0.0f;
	pipelineColorBlendStateCreateInfo.blendConstants[1] = 0.0f;
	pipelineColorBlendStateCreateInfo.blendConstants[2] = 0.0f;
	pipelineColorBlendStateCreateInfo.blendConstants[3] = 0.0f;

	/****************************************************************************************************************************/
	/**********   Reserve pipeline Layout, which help access to descriptor sets from a pipeline       ***************************/
	/****************************************************************************************************************************/
	std::vector<VkDescriptorSetLayout> descriptorSetLayoutVector;
	descriptorSetLayoutVector.push_back(perspectiveProjection_DSL);

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
	pipelineLayoutCreateInfo.sType			= VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = descriptorSetLayoutVector.size();
	pipelineLayoutCreateInfo.pSetLayouts	= descriptorSetLayoutVector.data();

	SenAbstractGLFW::errorCheck(
		vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &trianglePipelineLayout),
		std::string("Failed to to create pipeline layout !!!")
	);

	/****************************************************************************************************************************/
	/**********                Create   Pipeline            *********************************************************************/
	/****************************************************************************************************************************/
	std::vector<VkGraphicsPipelineCreateInfo> graphicsPipelineCreateInfoVector;
	VkGraphicsPipelineCreateInfo textureAppPipelineCreateInfo{};
	textureAppPipelineCreateInfo.sType					= VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	textureAppPipelineCreateInfo.stageCount				= (uint32_t)pipelineShaderStagesCreateInfoVector.size();
	textureAppPipelineCreateInfo.pStages				= pipelineShaderStagesCreateInfoVector.data();
	textureAppPipelineCreateInfo.pVertexInputState		= &pipelineVertexInputStateCreateInfo;
	textureAppPipelineCreateInfo.pInputAssemblyState	= &pipelineInputAssemblyStateCreateInfo;
	textureAppPipelineCreateInfo.pViewportState			= &pipelineViewportStateCreateInfo;
	textureAppPipelineCreateInfo.pRasterizationState	= &pipelineRasterizationStateCreateInfo;
	textureAppPipelineCreateInfo.pMultisampleState		= &pipelineMultisampleStateCreateInfo;
	textureAppPipelineCreateInfo.pColorBlendState		= &pipelineColorBlendStateCreateInfo;
	textureAppPipelineCreateInfo.layout					= trianglePipelineLayout;
	textureAppPipelineCreateInfo.renderPass				= colorAttachOnlyRenderPass;
	textureAppPipelineCreateInfo.subpass				= 0; // index of this trianglePipeline's subpass of the colorAttachOnlyRenderPass
															//textureAppPipelineCreateInfo.basePipelineHandle	= VK_NULL_HANDLE;

	graphicsPipelineCreateInfoVector.push_back(textureAppPipelineCreateInfo);

	SenAbstractGLFW::errorCheck(
		vkCreateGraphicsPipelines(
			device, VK_NULL_HANDLE,
			(uint32_t)graphicsPipelineCreateInfoVector.size(),
			graphicsPipelineCreateInfoVector.data(),
			nullptr,
			&trianglePipeline), // could be a pipelineArray
		std::string("Failed to create graphics pipeline !!!")
	);

	vkDestroyShaderModule(device, vertShaderModule, nullptr);
	vkDestroyShaderModule(device, fragShaderModule, nullptr);
}

void Sen_07_Texture::createTextureAppVertexBuffer()
{
	float vertices[] = {
		// Positions	// Colors
		-0.5f,	-0.5f,	1.0f,	0.0f,	0.0f,	0.0f,	0.0f,	// Bottom Right
		0.5f,	-0.5f,	0.0f,	0.0f,	1.0f,	1.0f,	0.0f,	// Bottom Left
		-0.5f,	0.5f,	1.0f,	1.0f,	1.0f,   0.0f,	1.0f,	// Top Right
		0.5f,	0.5f,	0.0f,	1.0f,	0.0f,	1.0f,	1.0f   // Top Left
	};
	size_t verticesBufferSize = sizeof(vertices);

	/****************************************************************************************************************************************************/
	/***************   Create temporary stagingBuffer to transfer from to get Optimal Buffer Resource   *************************************************/
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferDeviceMemory;
	SenAbstractGLFW::createResourceBuffer(device, verticesBufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE, physicalDeviceMemoryProperties,
		stagingBuffer, stagingBufferDeviceMemory, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	void* data;
	vkMapMemory(device, stagingBufferDeviceMemory, 0, verticesBufferSize, 0, &data);
	memcpy(data, vertices, verticesBufferSize);
	// The driver may not immediately copy the data into the buffer memory, for example because of caching. 
	// There are two ways to deal with that problem, and what we use is the first one below:
	//  1. Use a memory heap that is host coherent, indicated with VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	//  2. Call vkFlushMappedMemoryRanges to after writing to the mapped memory, and call vkInvalidateMappedMemoryRanges before reading from the mapped memory
	vkUnmapMemory(device, stagingBufferDeviceMemory);

	/****************************************************************************************************************************************************/
	/***************   Transfer from stagingBuffer to Optimal triangleVertexBuffer   ********************************************************************/
	SenAbstractGLFW::createResourceBuffer(device, verticesBufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE, physicalDeviceMemoryProperties,
		triangleVertexBuffer, triangleVertexBufferMemory, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	SenAbstractGLFW::transferResourceBuffer(defaultThreadCommandPool, device, graphicsQueue, stagingBuffer,
		triangleVertexBuffer, verticesBufferSize);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferDeviceMemory, nullptr);	// always try to destroy before free
}

void Sen_07_Texture::initBackgroundTextureImage()
{
	SenAbstractGLFW::createDeviceLocalTexture(device, physicalDeviceMemoryProperties
		, backgroundTextureDiskAddress, VK_IMAGE_TYPE_2D, backgroundTextureWidth, backgroundTextureHeight
		, backgroundTextureImage, backgroundTextureImageDeviceMemory, backgroundTextureImageView
		, VK_SHARING_MODE_EXCLUSIVE, defaultThreadCommandPool, graphicsQueue);

	SenAbstractGLFW::createTextureSampler(device, texture2DSampler);
}

void Sen_07_Texture::createTextureAppDescriptorPool()
{
	std::vector<VkDescriptorPoolSize> descriptorPoolSizeVector;

	VkDescriptorPoolSize uniformBufferDescriptorPoolSize{};
	uniformBufferDescriptorPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uniformBufferDescriptorPoolSize.descriptorCount = 1;
	descriptorPoolSizeVector.push_back(uniformBufferDescriptorPoolSize);

	VkDescriptorPoolSize combinedImageSamplerDescriptorPoolSize{};
	combinedImageSamplerDescriptorPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	combinedImageSamplerDescriptorPoolSize.descriptorCount = 1;
	descriptorPoolSizeVector.push_back(combinedImageSamplerDescriptorPoolSize);

	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{};
	descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCreateInfo.poolSizeCount = descriptorPoolSizeVector.size();
	descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizeVector.data();
	descriptorPoolCreateInfo.maxSets = 1; // Need a new descriptorSetVector

	SenAbstractGLFW::errorCheck(
		vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, nullptr, &descriptorPool),
		std::string("Fail to Create descriptorPool !")
	);
}

void Sen_07_Texture::createTextureAppDescriptorSetLayout()
{
	std::vector<VkDescriptorSetLayoutBinding> perspectiveProjectionDSL_BindingVector;

	VkDescriptorSetLayoutBinding mvpUboDSL_Binding{};
	mvpUboDSL_Binding.binding				= 0;
	mvpUboDSL_Binding.descriptorCount		= 1;
	mvpUboDSL_Binding.descriptorType		= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	mvpUboDSL_Binding.pImmutableSamplers	= nullptr;
	mvpUboDSL_Binding.stageFlags			= VK_SHADER_STAGE_VERTEX_BIT;
	perspectiveProjectionDSL_BindingVector.push_back(mvpUboDSL_Binding);

	VkDescriptorSetLayoutBinding combinedImageSamplerDSL_Binding{};
	combinedImageSamplerDSL_Binding.binding				= 1;
	combinedImageSamplerDSL_Binding.descriptorCount		= 1;
	combinedImageSamplerDSL_Binding.descriptorType		= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	combinedImageSamplerDSL_Binding.pImmutableSamplers	= nullptr;
	combinedImageSamplerDSL_Binding.stageFlags			= VK_SHADER_STAGE_FRAGMENT_BIT;
	perspectiveProjectionDSL_BindingVector.push_back(combinedImageSamplerDSL_Binding);

	VkDescriptorSetLayoutCreateInfo perspectiveProjectionDSL_CreateInfo{};
	perspectiveProjectionDSL_CreateInfo.sType			= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	perspectiveProjectionDSL_CreateInfo.bindingCount	= perspectiveProjectionDSL_BindingVector.size();
	perspectiveProjectionDSL_CreateInfo.pBindings		= perspectiveProjectionDSL_BindingVector.data();
	
	SenAbstractGLFW::errorCheck(
		vkCreateDescriptorSetLayout(device, &perspectiveProjectionDSL_CreateInfo, nullptr, &perspectiveProjection_DSL),
		std::string("Fail to Create perspectiveProjection_DSL !")
	);
}

void Sen_07_Texture::createTextureAppDescriptorSet()
{
	std::vector<VkDescriptorSetLayout> descriptorSetLayoutVector;
	descriptorSetLayoutVector.push_back(perspectiveProjection_DSL);
	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
	descriptorSetAllocateInfo.sType					= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.descriptorPool		= descriptorPool;
	descriptorSetAllocateInfo.descriptorSetCount	= descriptorSetLayoutVector.size();
	descriptorSetAllocateInfo.pSetLayouts			= descriptorSetLayoutVector.data();

	SenAbstractGLFW::errorCheck(
		vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, &perspectiveProjection_DS),
		std::string("Fail to Allocate perspectiveProjection_DS !")
	);
	/**********************************************************************************************************************/
	/**********************************************************************************************************************/
	VkDescriptorBufferInfo mvpDescriptorBufferInfo{};
	mvpDescriptorBufferInfo.buffer	= mvpOptimalUniformBuffer;
	mvpDescriptorBufferInfo.offset	= 0;
	mvpDescriptorBufferInfo.range	= sizeof(MvpUniformBufferObject);
	std::vector<VkDescriptorBufferInfo> descriptorBufferInfoVector;
	descriptorBufferInfoVector.push_back(mvpDescriptorBufferInfo);
	VkWriteDescriptorSet uniformBuffer_DS_Write{};
	uniformBuffer_DS_Write.sType			= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	uniformBuffer_DS_Write.descriptorType	= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uniformBuffer_DS_Write.dstSet			= perspectiveProjection_DS;
	uniformBuffer_DS_Write.dstBinding		= 0;	// binding number, same as the binding index specified in shader for a given shader stage
	uniformBuffer_DS_Write.dstArrayElement	= 0;	// start from the index dstArrayElement of pBufferInfo (descriptorBufferInfoVector)
	uniformBuffer_DS_Write.descriptorCount	= descriptorBufferInfoVector.size();// the total number of descriptors to update in pBufferInfo
	uniformBuffer_DS_Write.pBufferInfo		= descriptorBufferInfoVector.data();

	VkDescriptorImageInfo backgroundTextureDescriptorImageInfo{};
	backgroundTextureDescriptorImageInfo.imageLayout	= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	backgroundTextureDescriptorImageInfo.imageView		= backgroundTextureImageView;
	backgroundTextureDescriptorImageInfo.sampler		= texture2DSampler;
	std::vector<VkDescriptorImageInfo> descriptorImageInfoVector;
	descriptorImageInfoVector.push_back(backgroundTextureDescriptorImageInfo);
	VkWriteDescriptorSet combinedImageSampler_DS_Write{};
	combinedImageSampler_DS_Write.sType				= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	combinedImageSampler_DS_Write.descriptorType	= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	combinedImageSampler_DS_Write.dstSet			= perspectiveProjection_DS;
	combinedImageSampler_DS_Write.dstBinding		= 1;	// binding number, same as the binding index specified in shader for a given shader stage
	combinedImageSampler_DS_Write.dstArrayElement	= 0;	// start from the index dstArrayElement of pBufferInfo (descriptorBufferInfoVector)
	combinedImageSampler_DS_Write.descriptorCount	= descriptorImageInfoVector.size();// the total number of descriptors to update in pBufferInfo
	combinedImageSampler_DS_Write.pImageInfo		= descriptorImageInfoVector.data();

	std::vector<VkWriteDescriptorSet> DS_Write_Vector;
	DS_Write_Vector.push_back(uniformBuffer_DS_Write);
	DS_Write_Vector.push_back(combinedImageSampler_DS_Write);

	vkUpdateDescriptorSets(device, DS_Write_Vector.size(), DS_Write_Vector.data(), 0, nullptr);
}
