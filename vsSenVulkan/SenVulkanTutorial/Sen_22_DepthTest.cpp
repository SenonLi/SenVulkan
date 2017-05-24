#include "Sen_22_DepthTest.h"

Sen_22_DepthTest::Sen_22_DepthTest()
{
	std::cout << "Constructor: Sen_22_DepthTest()\n\n";
	strWindowName = "Sen Vulkan Depth Test Tutorial";

	backgroundTextureDiskAddress = "../Images/SunRaise.jpg";
}


Sen_22_DepthTest::~Sen_22_DepthTest()
{
	finalizeWidget();

	OutputDebugString("\n\t ~Sen_22_DepthTest()\n");
}

void Sen_22_DepthTest::initVulkanApplication()
{
	// Need to be segmented base on pipleStages in this function

	createColorAttachOnlyRenderPass();
	createTextureAppDescriptorSetLayout();
	createTextureAppPipeline();
	createColorAttachOnlySwapchainFramebuffers();
	createDefaultCommandPool();

	/***************************************/
	createDepthResources(); // has to be called after createDefaultCommandPool();
	/***************************************/


	initBackgroundTextureImage();
	createDepthTestVertexBuffer();
	createDepthTestIndexBuffer();
	createMvpUniformBuffers();
	createTextureAppDescriptorPool();
	createTextureAppDescriptorSet();

	createTextureAppCommandBuffers();

	std::cout << "\n Finish  Sen_22_DepthTest::initVulkanApplication()\n";
}

void Sen_22_DepthTest::reCreateRenderTarget()
{
	createTextureAppPipeline();
	createColorAttachOnlySwapchainFramebuffers();

	/***************************************/
	createDepthResources();
	/***************************************/

	createTextureAppCommandBuffers();
}

void Sen_22_DepthTest::cleanUpDepthStencil()
{
	if (VK_NULL_HANDLE != depthTestImage) {
		vkDestroyImage(device, depthTestImage, nullptr);
		if (VK_NULL_HANDLE != depthTestImageView)
			vkDestroyImageView(device, depthTestImageView, nullptr);
		if (VK_NULL_HANDLE != depthTestImageDeviceMemory)
			vkFreeMemory(device, depthTestImageDeviceMemory, nullptr); 	// always try to destroy before free

		depthTestImage = VK_NULL_HANDLE;
		depthTestImageView = VK_NULL_HANDLE;
		depthTestImageDeviceMemory = VK_NULL_HANDLE;
	}
}

void Sen_22_DepthTest::updateUniformBuffer() {
	static auto startTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();
	int duration = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count() / 1000.0f;

	MvpUniformBufferObject mvpUbo{};
	mvpUbo.model = glm::rotate(glm::mat4(), -duration * glm::radians(15.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	mvpUbo.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	mvpUbo.projection = glm::perspective(glm::radians(45.0f), widgetWidth / (float)widgetHeight, 0.1f, 100.0f);
	mvpUbo.projection[1][1] *= -1;

	void* data;
	vkMapMemory(device, mvpUniformStagingBufferDeviceMemory, 0, sizeof(mvpUbo), 0, &data);
	memcpy(data, &mvpUbo, sizeof(mvpUbo));
	vkUnmapMemory(device, mvpUniformStagingBufferDeviceMemory);

	SenAbstractGLFW::transferResourceBuffer(defaultThreadCommandPool, device, graphicsQueue, mvpUniformStagingBuffer,
		mvpOptimalUniformBuffer, sizeof(mvpUbo));
}

void Sen_22_DepthTest::finalizeWidget()
{	
	cleanUpDepthStencil();

	/************************************************************************************************************/
	/******************           Destroy Memory, ImageView, Image          *************************************/
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
	/************************************************************************************************************/
	/*********************           Destroy Pipeline, PipelineLayout, and RenderPass         *******************/
	/************************************************************************************************************/
	if (VK_NULL_HANDLE != textureAppPipeline) {
		vkDestroyPipeline(device, textureAppPipeline, nullptr);
		vkDestroyPipelineLayout(device, textureAppPipelineLayout, nullptr);
		vkDestroyRenderPass(device, colorAttachOnlyRenderPass, nullptr);

		textureAppPipeline			= VK_NULL_HANDLE;
		textureAppPipelineLayout	= VK_NULL_HANDLE;
		colorAttachOnlyRenderPass	= VK_NULL_HANDLE;
	}
	/************************************************************************************************************/
	/******************     Destroy VertexBuffer, VertexBufferMemory     ****************************************/
	/************************************************************************************************************/
	if (VK_NULL_HANDLE != textureAppVertexBuffer) {
		vkDestroyBuffer(device, textureAppVertexBuffer, nullptr);
		vkFreeMemory(device, textureAppVertexBufferMemory, nullptr);	// always try to destroy before free

		textureAppVertexBuffer			= VK_NULL_HANDLE;
		textureAppVertexBufferMemory	= VK_NULL_HANDLE;
	}

	OutputDebugString("\n\tFinish  Sen_22_DepthTest::finalizeWidget()\n");
}

void Sen_22_DepthTest::createTextureAppPipeline()
{
	if (VK_NULL_HANDLE != textureAppPipeline) {
		vkDestroyPipeline(device, textureAppPipeline, nullptr);
		vkDestroyPipelineLayout(device, textureAppPipelineLayout, nullptr);

		textureAppPipeline = VK_NULL_HANDLE;
		textureAppPipelineLayout = VK_NULL_HANDLE;
	}

	/****************************************************************************************************************************/
	/**********                Reserve pipeline ShaderStage CreateInfos Array           *****************************************/
	/****************************************************************************************************************************/
	VkShaderModule vertShaderModule, fragShaderModule;

	createVulkanShaderModule(device, "SenVulkanTutorial/Shaders/depthTest.vert", vertShaderModule);
	createVulkanShaderModule(device, "SenVulkanTutorial/Shaders/depthTest.frag", fragShaderModule);

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
	vertexInputBindingDescription.binding	= 0;
	vertexInputBindingDescription.stride	= 8 * sizeof(float);
	vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	std::vector<VkVertexInputBindingDescription> vertexInputBindingDescriptionVector;
	vertexInputBindingDescriptionVector.push_back(vertexInputBindingDescription);


	std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescriptionVector;

	VkVertexInputAttributeDescription positionVertexInputAttributeDescription;
	positionVertexInputAttributeDescription.location	= 0;
	positionVertexInputAttributeDescription.binding		= 0;
	positionVertexInputAttributeDescription.format		= VK_FORMAT_R32G32B32_SFLOAT;
	positionVertexInputAttributeDescription.offset		= 0;
	vertexInputAttributeDescriptionVector.push_back(positionVertexInputAttributeDescription);

	VkVertexInputAttributeDescription colorVertexInputAttributeDescription;
	colorVertexInputAttributeDescription.location		= 1;
	colorVertexInputAttributeDescription.binding		= 0;
	colorVertexInputAttributeDescription.format			= VK_FORMAT_R32G32B32_SFLOAT;
	colorVertexInputAttributeDescription.offset			= 3 * sizeof(float);
	vertexInputAttributeDescriptionVector.push_back(colorVertexInputAttributeDescription);

	VkVertexInputAttributeDescription texCoordVertexInputAttributeDescription;
	texCoordVertexInputAttributeDescription.location	= 2;
	texCoordVertexInputAttributeDescription.binding		= 0;
	texCoordVertexInputAttributeDescription.format		= VK_FORMAT_R32G32_SFLOAT;
	texCoordVertexInputAttributeDescription.offset		= 6 * sizeof(float);
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
		vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &textureAppPipelineLayout),
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
	textureAppPipelineCreateInfo.layout					= textureAppPipelineLayout;
	textureAppPipelineCreateInfo.renderPass				= colorAttachOnlyRenderPass;
	textureAppPipelineCreateInfo.subpass				= 0; // index of this textureAppPipeline's subpass of the colorAttachOnlyRenderPass
															//textureAppPipelineCreateInfo.basePipelineHandle	= VK_NULL_HANDLE;

	graphicsPipelineCreateInfoVector.push_back(textureAppPipelineCreateInfo);

	SenAbstractGLFW::errorCheck(
		vkCreateGraphicsPipelines(
			device, VK_NULL_HANDLE,
			(uint32_t)graphicsPipelineCreateInfoVector.size(),
			graphicsPipelineCreateInfoVector.data(),
			nullptr,
			&textureAppPipeline), // could be a pipelineArray
		std::string("Failed to create graphics pipeline !!!")
	);

	vkDestroyShaderModule(device, vertShaderModule, nullptr);
	vkDestroyShaderModule(device, fragShaderModule, nullptr);
}

void Sen_22_DepthTest::createDepthTestIndexBuffer()
{
	uint16_t indices[] = {	0, 1, 2, 1, 2, 3, 
							4, 5, 6, 5, 6, 7 };
	size_t indicesBufferSize = sizeof(indices);

	/****************************************************************************************************************************************************/
	/***************   Create temporary stagingBuffer to transfer from to get Optimal Buffer Resource   *************************************************/
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferDeviceMemory;
	SenAbstractGLFW::createResourceBuffer(device, indicesBufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE, physicalDeviceMemoryProperties,
		stagingBuffer, stagingBufferDeviceMemory, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	void* data;
	vkMapMemory(device, stagingBufferDeviceMemory, 0, indicesBufferSize, 0, &data);
	memcpy(data, indices, indicesBufferSize);
	//// The driver may not immediately copy the data into the buffer memory, for example because of caching. 
	//// There are two ways to deal with that problem, and what we use is the first one below:
	////  1. Use a memory heap that is host coherent, indicated with VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	////  2. Call vkFlushMappedMemoryRanges to after writing to the mapped memory, and call vkInvalidateMappedMemoryRanges before reading from the mapped memory
	vkUnmapMemory(device, stagingBufferDeviceMemory);

	/****************************************************************************************************************************************************/
	/***************   Transfer from stagingBuffer to Optimal triangleVertexBuffer   ********************************************************************/
	SenAbstractGLFW::createResourceBuffer(device, indicesBufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE, physicalDeviceMemoryProperties,
		singleRectIndexBuffer, singleRectIndexBufferMemory, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	SenAbstractGLFW::transferResourceBuffer(defaultThreadCommandPool, device, graphicsQueue, stagingBuffer,
		singleRectIndexBuffer, indicesBufferSize);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferDeviceMemory, nullptr);	// always try to destroy before free
}

void Sen_22_DepthTest::createDepthTestVertexBuffer()
{
	float vertices[] = {
		// Positions			// Colors				// TexCoord
		-0.5f,	-0.5f,	0.5f,	1.0f,	0.0f,	0.0f,	0.0f,	0.0f,	// Bottom Right
		0.5f,	-0.5f,	0.5f,	0.0f,	0.0f,	1.0f,	1.0f,	0.0f,	// Bottom Left
		-0.5f,	0.5f,	0.5f,	1.0f,	1.0f,	1.0f,   0.0f,	1.0f,	// Top Right
		0.5f,	0.5f,	0.5f,	0.0f,	1.0f,	0.0f,	1.0f,	1.0f,   // Top Left

		-0.5f,	-0.5f,	-0.5f,	1.0f,	0.0f,	0.0f,	0.0f,	0.0f,	// Bottom Right
		0.5f,	-0.5f,	-0.5f,	0.0f,	0.0f,	1.0f,	1.0f,	0.0f,	// Bottom Left
		-0.5f,	0.5f,	-0.5f,	1.0f,	1.0f,	1.0f,   0.0f,	1.0f,	// Top Right
		0.5f,	0.5f,	-0.5f,	0.0f,	1.0f,	0.0f,	1.0f,	1.0f   // Top Left
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
	/***************   Transfer from stagingBuffer to Optimal textureAppVertexBuffer   ********************************************************************/
	SenAbstractGLFW::createResourceBuffer(device, verticesBufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE, physicalDeviceMemoryProperties,
		textureAppVertexBuffer, textureAppVertexBufferMemory, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	SenAbstractGLFW::transferResourceBuffer(defaultThreadCommandPool, device, graphicsQueue, stagingBuffer,
		textureAppVertexBuffer, verticesBufferSize);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferDeviceMemory, nullptr);	// always try to destroy before free
}

void Sen_22_DepthTest::createDepthResources()
{
	/********************************************************************************************************************/
	/******************************  Check Image Format *****************************************************************/
	bool hasStencil = false;
	VkFormatProperties formatProperties{};
	vkGetPhysicalDeviceFormatProperties(physicalDevice, VK_FORMAT_D32_SFLOAT, &formatProperties);
	if (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
		depthTestFormat = VK_FORMAT_D32_SFLOAT; // VK_FORMAT_D32_SFLOAT is extremely common for depthTest
	else{
		for (auto f : SenAbstractGLFW::depthStencilSupportCheckFormatsVector) {
			VkFormatProperties formatProperties{};
			vkGetPhysicalDeviceFormatProperties(physicalDevice, f, &formatProperties);
			if (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
				depthTestFormat = f;
				break;
			}
		}
		if (depthTestFormat == VK_FORMAT_UNDEFINED) {
			throw std::runtime_error("Depth stencil format not selected.");
			std::exit(-1);
		}
		hasStencil = SenAbstractGLFW::hasStencilComponent(depthTestFormat);
	}
	/********************************************************************************************************************/
	/***************************     Create depthTest Image     *********************************************************/
	SenAbstractGLFW::createResourceImage(device, widgetWidth, widgetHeight, VK_IMAGE_TYPE_2D,  // depthTestImage is also a 2D image
		depthTestFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, depthTestImage
		, depthTestImageDeviceMemory, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_SHARING_MODE_EXCLUSIVE, physicalDeviceMemoryProperties);

	/********************************************************************************************************************/
	/******************************     Create depthTest Image View    **************************************************/
	VkImageSubresourceRange depthTestImageSubresourceRange{};
	depthTestImageSubresourceRange.aspectMask		= VK_IMAGE_ASPECT_DEPTH_BIT | (hasStencil ? VK_IMAGE_ASPECT_STENCIL_BIT : 0);
	depthTestImageSubresourceRange.baseMipLevel		= 0;	// first mipMap level to start
	depthTestImageSubresourceRange.levelCount		= 1;
	depthTestImageSubresourceRange.baseArrayLayer	= 0;	// first arrayLayer to start
	depthTestImageSubresourceRange.layerCount		= 1;
	
	VkImageViewCreateInfo depthTestImageViewCreateInfo{};
	depthTestImageViewCreateInfo.sType				= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	depthTestImageViewCreateInfo.image				= depthTestImage;
	depthTestImageViewCreateInfo.viewType			= VK_IMAGE_VIEW_TYPE_2D;
	depthTestImageViewCreateInfo.format				= depthTestFormat;
	depthTestImageViewCreateInfo.subresourceRange	= depthTestImageSubresourceRange;

	vkCreateImageView(device, &depthTestImageViewCreateInfo, nullptr, &depthTestImageView);
	/********************************************************************************************************************/
	/******************************     Transition depthTest ImageLayout     ********************************************/
	SenAbstractGLFW::transitionResourceImageLayout(depthTestImage, depthTestImageSubresourceRange, VK_IMAGE_LAYOUT_PREINITIALIZED,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, depthTestFormat, device, defaultThreadCommandPool, graphicsQueue);


}

void Sen_22_DepthTest::initBackgroundTextureImage()
{
	SenAbstractGLFW::createDeviceLocalTexture(device, physicalDeviceMemoryProperties
		, backgroundTextureDiskAddress, VK_IMAGE_TYPE_2D, backgroundTextureWidth, backgroundTextureHeight
		, backgroundTextureImage, backgroundTextureImageDeviceMemory, backgroundTextureImageView
		, VK_SHARING_MODE_EXCLUSIVE, defaultThreadCommandPool, graphicsQueue);

	SenAbstractGLFW::createTextureSampler(device, texture2DSampler);
}

void Sen_22_DepthTest::createTextureAppDescriptorPool()
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

void Sen_22_DepthTest::createTextureAppDescriptorSetLayout()
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

void Sen_22_DepthTest::createTextureAppDescriptorSet()
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
	/**********************************************************************************************************************/
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

void Sen_22_DepthTest::createTextureAppCommandBuffers()
{
	if (swapchainCommandBufferVector.size() > 0) {
		vkFreeCommandBuffers(device, defaultThreadCommandPool, (uint32_t)swapchainCommandBufferVector.size(), swapchainCommandBufferVector.data());
	}
	/****************************************************************************************************************************/
	/**********           Allocate Swapchain CommandBuffers         *************************************************************/
	/****************************************************************************************************************************/
	swapchainCommandBufferVector.resize(swapchainImagesCount);

	VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandPool = defaultThreadCommandPool;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount = static_cast<uint32_t>(swapchainCommandBufferVector.size());

	SenAbstractGLFW::errorCheck(
		vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, swapchainCommandBufferVector.data()),
		std::string("Failed to allocate Swapchain commandBuffers !!!")
	);

	/****************************************************************************************************************************/
	/**********           Record Triangle Swapchain CommandBuffers        *******************************************************/
	/****************************************************************************************************************************/
	for (size_t i = 0; i < swapchainCommandBufferVector.size(); i++) {
		//======================================================================================
		//======================================================================================
		VkCommandBufferBeginInfo commandBufferBeginInfo{};
		commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT; // In case we may already be scheduling the drawing commands for the next frame while the last frame hass not finished yet.
		vkBeginCommandBuffer(swapchainCommandBufferVector[i], &commandBufferBeginInfo);

		//======================================================================================
		//======================================================================================
		VkRenderPassBeginInfo renderPassBeginInfo{};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = colorAttachOnlyRenderPass;
		renderPassBeginInfo.framebuffer = swapchainFramebufferVector[i];
		renderPassBeginInfo.renderArea.offset = { 0, 0 };
		renderPassBeginInfo.renderArea.extent.width = widgetWidth;
		renderPassBeginInfo.renderArea.extent.height = widgetHeight;

		std::vector<VkClearValue> clearValueVector;
		clearValueVector.push_back(VkClearValue{ 0.2f, 0.3f, 0.3f, 1.0f });
		renderPassBeginInfo.clearValueCount = (uint32_t)clearValueVector.size();
		renderPassBeginInfo.pClearValues = clearValueVector.data();

		vkCmdBeginRenderPass(swapchainCommandBufferVector[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		//======================================================================================
		//======================================================================================
		vkCmdBindPipeline(swapchainCommandBufferVector[i], VK_PIPELINE_BIND_POINT_GRAPHICS, textureAppPipeline);
		VkDeviceSize offsetDeviceSize = 0;
		vkCmdBindVertexBuffers(swapchainCommandBufferVector[i], 0, 1, &textureAppVertexBuffer, &offsetDeviceSize);

		//vkCmdDraw(
		//	swapchainCommandBufferVector[i],
		//	3, // vertexCount
		//	1, // instanceCount
		//	0, // firstVertex
		//	0  // firstInstance
		//);

		vkCmdBindIndexBuffer(swapchainCommandBufferVector[i], singleRectIndexBuffer, 0, VK_INDEX_TYPE_UINT16);

		vkCmdBindDescriptorSets(swapchainCommandBufferVector[i], VK_PIPELINE_BIND_POINT_GRAPHICS, textureAppPipelineLayout, 0, 1, &perspectiveProjection_DS, 0, nullptr);


		vkCmdDrawIndexed(swapchainCommandBufferVector[i], 12, 1, 0, 0, 0);

		vkCmdEndRenderPass(swapchainCommandBufferVector[i]);

		SenAbstractGLFW::errorCheck(
			vkEndCommandBuffer(swapchainCommandBufferVector[i]),
			std::string("Failed to end record of Triangle Swapchain commandBuffers !!!")
		);
	}
}
