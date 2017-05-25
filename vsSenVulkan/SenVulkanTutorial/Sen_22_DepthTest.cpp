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
	createTextureAppDescriptorSetLayout();
	createDefaultCommandPool();

	initBackgroundTextureImage();
	createMvpUniformBuffers();
	createTextureAppDescriptorPool();
	createTextureAppDescriptorSet();

	/***************************************/
	createDepthResources();					// has to be called after createDefaultCommandPool();
	createDepthTestRenderPass();			// has to be called after createDepthResources() for depthTestFormat
	createDepthTestPipeline();
	createDepthTestSwapchainFramebuffers(); // has to be called after createDepthResources() for the depthTestImageView
	createDepthTestVertexBuffer();
	createDepthTestIndexBuffer();
	/***************************************/

	createDepthTestCommandBuffers();

	std::cout << "\n Finish  Sen_22_DepthTest::initVulkanApplication()\n";
}

void Sen_22_DepthTest::reCreateRenderTarget()
{
	createDepthResources();
	createDepthTestPipeline();
	createDepthTestSwapchainFramebuffers();
	createDepthTestCommandBuffers();
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
	if (VK_NULL_HANDLE != depthTestPipeline) {
		vkDestroyPipeline(device, depthTestPipeline, nullptr);
		vkDestroyPipelineLayout(device, textureAppPipelineLayout, nullptr);
		vkDestroyRenderPass(device, depthTestRenderPass, nullptr);

		depthTestPipeline			= VK_NULL_HANDLE;
		textureAppPipelineLayout	= VK_NULL_HANDLE;
		depthTestRenderPass	= VK_NULL_HANDLE;
	}
	/************************************************************************************************************/
	/******************     Destroy VertexBuffer, VertexBufferMemory     ****************************************/
	/************************************************************************************************************/
	if (VK_NULL_HANDLE != depthTestVertexBuffer) {
		vkDestroyBuffer(device, depthTestVertexBuffer, nullptr);
		vkFreeMemory(device, depthTestVertexBufferMemory, nullptr);	// always try to destroy before free

		depthTestVertexBuffer			= VK_NULL_HANDLE;
		depthTestVertexBufferMemory	= VK_NULL_HANDLE;
	}

	OutputDebugString("\n\tFinish  Sen_22_DepthTest::finalizeWidget()\n");
}

void Sen_22_DepthTest::createDepthTestPipeline()
{
	/************************************************************************************************************/
	/*********     Destroy old depthTestPipeline first for widgetRezie, if there are      ***********************/
	/************************************************************************************************************/
	if (VK_NULL_HANDLE != depthTestPipeline) {
		vkDestroyPipeline(device, depthTestPipeline, nullptr);
		vkDestroyPipelineLayout(device, textureAppPipelineLayout, nullptr);

		depthTestPipeline			= VK_NULL_HANDLE;
		textureAppPipelineLayout	= VK_NULL_HANDLE;
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
	pipelineColorBlendStateCreateInfo.attachmentCount	= (uint32_t)pipelineColorBlendAttachmentStateVector.size();
	pipelineColorBlendStateCreateInfo.pAttachments		= pipelineColorBlendAttachmentStateVector.data();
	pipelineColorBlendStateCreateInfo.blendConstants[0] = 0.0f;
	pipelineColorBlendStateCreateInfo.blendConstants[1] = 0.0f;
	pipelineColorBlendStateCreateInfo.blendConstants[2] = 0.0f;
	pipelineColorBlendStateCreateInfo.blendConstants[3] = 0.0f;

	/*********************************************************************************************/
	/*********************************************************************************************/
	VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo{};
	pipelineDepthStencilStateCreateInfo.sType					= VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	pipelineDepthStencilStateCreateInfo.depthTestEnable			= VK_TRUE; // Enabled depth testing in the graphics pipeline
	pipelineDepthStencilStateCreateInfo.depthWriteEnable		= VK_TRUE; // useful for drawing transparent objects
	pipelineDepthStencilStateCreateInfo.depthCompareOp			= VK_COMPARE_OP_LESS;
	pipelineDepthStencilStateCreateInfo.depthBoundsTestEnable	= VK_FALSE;
	pipelineDepthStencilStateCreateInfo.stencilTestEnable		= VK_FALSE;

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
	std::vector<VkGraphicsPipelineCreateInfo> depthTestGraphicsPipelineCreateInfoVector;
	VkGraphicsPipelineCreateInfo depthTestPipelineCreateInfo{};
	depthTestPipelineCreateInfo.sType				= VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	depthTestPipelineCreateInfo.stageCount			= (uint32_t)pipelineShaderStagesCreateInfoVector.size();
	depthTestPipelineCreateInfo.pStages				= pipelineShaderStagesCreateInfoVector.data();
	depthTestPipelineCreateInfo.pVertexInputState	= &pipelineVertexInputStateCreateInfo;
	depthTestPipelineCreateInfo.pInputAssemblyState	= &pipelineInputAssemblyStateCreateInfo;
	depthTestPipelineCreateInfo.pViewportState		= &pipelineViewportStateCreateInfo;
	depthTestPipelineCreateInfo.pRasterizationState	= &pipelineRasterizationStateCreateInfo;
	depthTestPipelineCreateInfo.pMultisampleState	= &pipelineMultisampleStateCreateInfo;
	depthTestPipelineCreateInfo.pColorBlendState	= &pipelineColorBlendStateCreateInfo;
	depthTestPipelineCreateInfo.pDepthStencilState	= &pipelineDepthStencilStateCreateInfo;
	depthTestPipelineCreateInfo.layout				= textureAppPipelineLayout;
	depthTestPipelineCreateInfo.renderPass			= depthTestRenderPass;
	depthTestPipelineCreateInfo.subpass				= 0;	// index of this depthTestPipeline's subpass of the depthTestRenderPass
															//depthTestPipelineCreateInfo.basePipelineHandle	= VK_NULL_HANDLE;

	depthTestGraphicsPipelineCreateInfoVector.push_back(depthTestPipelineCreateInfo);

	SenAbstractGLFW::errorCheck(
		vkCreateGraphicsPipelines(
			device, VK_NULL_HANDLE,
			(uint32_t)depthTestGraphicsPipelineCreateInfoVector.size(),
			depthTestGraphicsPipelineCreateInfoVector.data(),
			nullptr,
			&depthTestPipeline), // could be a pipelineArray
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
	/***************   Transfer from stagingBuffer to Optimal depthTestVertexBuffer   ********************************************************************/
	SenAbstractGLFW::createResourceBuffer(device, verticesBufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE, physicalDeviceMemoryProperties,
		depthTestVertexBuffer, depthTestVertexBufferMemory, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	SenAbstractGLFW::transferResourceBuffer(defaultThreadCommandPool, device, graphicsQueue, stagingBuffer,
		depthTestVertexBuffer, verticesBufferSize);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferDeviceMemory, nullptr);	// always try to destroy before free
}

void Sen_22_DepthTest::createDepthResources()
{
	/********************************************************************************************************************/
	/******    If first time (not resize):  Check depthTestImage Format,  Initial depthTestImageSubresourceRange     ****/
	if (depthTestFormat == VK_FORMAT_UNDEFINED) {
		VkFormatProperties formatProperties{};
		vkGetPhysicalDeviceFormatProperties(physicalDevice, VK_FORMAT_D32_SFLOAT, &formatProperties);
		if (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
			depthTestFormat = VK_FORMAT_D32_SFLOAT; // VK_FORMAT_D32_SFLOAT is extremely common for depthTest
		else {
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

		depthTestImageSubresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | (hasStencil ? VK_IMAGE_ASPECT_STENCIL_BIT : 0);
		depthTestImageSubresourceRange.baseMipLevel = 0;	// first mipMap level to start
		depthTestImageSubresourceRange.levelCount = 1;
		depthTestImageSubresourceRange.baseArrayLayer = 0;	// first arrayLayer to start
		depthTestImageSubresourceRange.layerCount = 1;
	}
	/********************************************************************************************************************/
	/***************************     Create depthTest Image     *********************************************************/
	SenAbstractGLFW::createResourceImage(device, widgetWidth, widgetHeight, VK_IMAGE_TYPE_2D,  // depthTestImage is also a 2D image
		depthTestFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, depthTestImage
		, depthTestImageDeviceMemory, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_SHARING_MODE_EXCLUSIVE, physicalDeviceMemoryProperties);

	/********************************************************************************************************************/
	/******************************     Create depthTest Image View    **************************************************/
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

void Sen_22_DepthTest::createDepthTestCommandBuffers()
{
	/************************************************************************************************************/
	/*********     Destroy old swapchainCommandBufferVector first for widgetRezie, if there are      ************/
	/************************************************************************************************************/
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
		renderPassBeginInfo.sType				= VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass			= depthTestRenderPass;
		renderPassBeginInfo.framebuffer			= swapchainFramebufferVector[i];
		renderPassBeginInfo.renderArea.offset	= { 0, 0 };
		renderPassBeginInfo.renderArea.extent.width		= widgetWidth;
		renderPassBeginInfo.renderArea.extent.height	= widgetHeight;

		// Because we now have both color & depth attachments with VK_ATTACHMENT_LOAD_OP_CLEAR, we also need to specify multiple clear values. 
		std::array<VkClearValue, 2> clearValueArray{};
		clearValueArray[0].color		= { 0.2f, 0.3f, 0.3f, 1.0f };
		clearValueArray[1].depthStencil = { 1.0f, 0 };
		renderPassBeginInfo.clearValueCount = (uint32_t)clearValueArray.size();
		renderPassBeginInfo.pClearValues	= clearValueArray.data();

		vkCmdBeginRenderPass(swapchainCommandBufferVector[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		//======================================================================================
		//======================================================================================
		vkCmdBindPipeline(swapchainCommandBufferVector[i], VK_PIPELINE_BIND_POINT_GRAPHICS, depthTestPipeline);
		VkDeviceSize offsetDeviceSize = 0;
		vkCmdBindVertexBuffers(swapchainCommandBufferVector[i], 0, 1, &depthTestVertexBuffer, &offsetDeviceSize);

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

void Sen_22_DepthTest::createDepthTestRenderPass()
{
	/********************************************************************************************************************/
	/************    Setting AttachmentDescription:  colorAttachment + depthTestAttachment      *************************/
	/********************************************************************************************************************/
	VkAttachmentDescription colorAttachmentDescription{};
	colorAttachmentDescription.format			= surfaceFormat.format;// swapChainImageFormat;
	colorAttachmentDescription.samples			= VK_SAMPLE_COUNT_1_BIT; // Not using multi-sampling
	colorAttachmentDescription.loadOp			= VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachmentDescription.storeOp			= VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachmentDescription.stencilLoadOp	= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentDescription.stencilStoreOp	= VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachmentDescription.initialLayout	= VK_IMAGE_LAYOUT_UNDEFINED;       // layout before renderPass
	colorAttachmentDescription.finalLayout		= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // auto transition after renderPass

	VkAttachmentDescription depthTestAttachmentDescription{};
	depthTestAttachmentDescription.format			= depthTestFormat;
	depthTestAttachmentDescription.samples			= VK_SAMPLE_COUNT_1_BIT; // Not using multi-sampling
	depthTestAttachmentDescription.loadOp			= VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthTestAttachmentDescription.storeOp			= VK_ATTACHMENT_STORE_OP_DONT_CARE; // don't care storing because depth data will not be used after drawing has finished. 
	depthTestAttachmentDescription.stencilLoadOp	= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthTestAttachmentDescription.stencilStoreOp	= VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthTestAttachmentDescription.initialLayout	= VK_IMAGE_LAYOUT_UNDEFINED; // bug happen if preinitialized, not sure why
	depthTestAttachmentDescription.finalLayout		= VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	std::vector<VkAttachmentDescription> attachmentDescriptionVector;
	attachmentDescriptionVector.push_back(colorAttachmentDescription);		// The colorAttachment index is 0
	attachmentDescriptionVector.push_back(depthTestAttachmentDescription);	// The depthTestAttachment index is 1
	/********************************************************************************************************************/
	/********    Setting Subpasses with Dependencies: One subpass is enough to paint the triangle     *******************/
	/********************************************************************************************************************/
	std::array<VkAttachmentReference, 1> colorAttachmentReferenceArray{};
	colorAttachmentReferenceArray[0].attachment = 0;						// The colorAttachment index is 0
	colorAttachmentReferenceArray[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // auto transition during renderPass

	VkAttachmentReference depthTestAttachmentReferenceArray{};
	depthTestAttachmentReferenceArray.attachment = 1;						// The depthTestAttachment index is 1
	depthTestAttachmentReferenceArray.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	std::array<VkSubpassDescription, 1> subpassDescriptionArray{};
	subpassDescriptionArray[0].pipelineBindPoint		= VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescriptionArray[0].colorAttachmentCount		= (uint32_t)colorAttachmentReferenceArray.size();	// Every subpass references one or more attachments
	subpassDescriptionArray[0].pColorAttachments		= colorAttachmentReferenceArray.data();
	subpassDescriptionArray[0].pDepthStencilAttachment	= &depthTestAttachmentReferenceArray;

	/******* There are two built-in dependencies that take care of the transition at the start of the render pass and at the end of the render pass,
	/////       but the former does not occur at the right time.
	/////       It assumes that the transition occurs at the start of the pipeline, but we haven't acquired the image yet at that point! ****/
	/******* There are two ways to deal with this problem.
	/////       We could change the waitStages for the imageAvailableSemaphore to VK_PIPELINE_STAGE_TOP_OF_PIPELINE_BIT
	/////        to ensure that the render passes don't begin until the image is available,
	/////       or we can make the render pass wait for the VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT stage. ****************************/
	std::vector<VkSubpassDependency> subpassDependencyVector;
	VkSubpassDependency headSubpassDependency{};
	headSubpassDependency.srcSubpass	= VK_SUBPASS_EXTERNAL;		// subpassIndex, from external
	headSubpassDependency.dstSubpass	= 0;						// subpassIndex, to the first subpass, which is also the only one
	headSubpassDependency.srcStageMask	= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // specify the operations to wait on and the stages in which these operations occur.
	headSubpassDependency.dstStageMask	= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	headSubpassDependency.srcAccessMask = 0;											 // specify the operations to wait on and the stages in which these operations occur.
	headSubpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT
										| VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	subpassDependencyVector.push_back(headSubpassDependency);

	/********************************************************************************************************************/
	/*********************    Create RenderPass for rendering triangle      *********************************************/
	/********************************************************************************************************************/
	VkRenderPassCreateInfo depthTestRenderPassCreateInfo{};
	depthTestRenderPassCreateInfo.sType				= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	depthTestRenderPassCreateInfo.attachmentCount	= (uint32_t)attachmentDescriptionVector.size();
	depthTestRenderPassCreateInfo.pAttachments		= attachmentDescriptionVector.data();
	depthTestRenderPassCreateInfo.subpassCount		= (uint32_t)subpassDescriptionArray.size();
	depthTestRenderPassCreateInfo.pSubpasses		= subpassDescriptionArray.data();
	depthTestRenderPassCreateInfo.dependencyCount	= (uint32_t)subpassDependencyVector.size();
	depthTestRenderPassCreateInfo.pDependencies		= subpassDependencyVector.data();

	SenAbstractGLFW::errorCheck(
		vkCreateRenderPass(device, &depthTestRenderPassCreateInfo, nullptr, &depthTestRenderPass),
		std::string("Failed to create render pass !!")
	);
}

void Sen_22_DepthTest::createDepthTestSwapchainFramebuffers()
{
	/************************************************************************************************************/
	/*********     Destroy old swapchainFramebuffers first for widgetRezie, if there are      *******************/
	/************************************************************************************************************/
	for (auto swapchainFramebuffer : swapchainFramebufferVector) {
		vkDestroyFramebuffer(device, swapchainFramebuffer, nullptr);
	}
	swapchainFramebufferVector.clear();
	swapchainFramebufferVector.resize(swapchainImagesCount);

	for (size_t i = 0; i < swapchainImagesCount; i++) {
		std::array<VkImageView, 2> imageViewAttachmentArray = {
			swapchainImageViewsVector[i],
			// The same depth image can be used by all of them,
			// because only a single subpass is running at the same time in this example due to the semaphores.
			depthTestImageView
		};

		VkFramebufferCreateInfo framebufferCreateInfo{};
		framebufferCreateInfo.sType				= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.renderPass		= depthTestRenderPass;
		framebufferCreateInfo.attachmentCount	= (uint32_t)imageViewAttachmentArray.size();
		framebufferCreateInfo.pAttachments		= imageViewAttachmentArray.data();
		framebufferCreateInfo.width				= widgetWidth;
		framebufferCreateInfo.height			= widgetHeight;
		framebufferCreateInfo.layers			= 1;

		SenAbstractGLFW::errorCheck(
			vkCreateFramebuffer(device, &framebufferCreateInfo, nullptr, &swapchainFramebufferVector[i]),
			std::string("Failed to create framebuffer !!!")
		);
	}
}
