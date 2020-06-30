
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
	createDepthTestAttachment();					// has to be called after createDefaultCommandPool();
	createDepthTestRenderPass();			// has to be called after createDepthTestAttachment() for depthTestFormat
	createDepthTestPipeline();
	createDepthTestSwapchainFramebuffers(); // has to be called after createDepthTestAttachment() for the depthTestImageView
	createDepthTestVertexBuffer();
	createDepthTestIndexBuffer();
	/***************************************/

	createDepthTestCommandBuffers();

	std::cout << "\n Finish  Sen_22_DepthTest::initVulkanApplication()\n";
}

void Sen_22_DepthTest::reCreateRenderTarget()
{
	createDepthTestAttachment();
	createDepthTestSwapchainFramebuffers();
	createDepthTestCommandBuffers();
}

void Sen_22_DepthTest::cleanUpDepthStencil()
{
	if (VK_NULL_HANDLE != depthTestImage) {
		vkDestroyImage(m_LogicalDevice, depthTestImage, nullptr);
		if (VK_NULL_HANDLE != depthTestImageView)
			vkDestroyImageView(m_LogicalDevice, depthTestImageView, nullptr);
		if (VK_NULL_HANDLE != depthTestImageDeviceMemory)
			vkFreeMemory(m_LogicalDevice, depthTestImageDeviceMemory, nullptr); 	// always try to destroy before free

		depthTestImage = VK_NULL_HANDLE;
		depthTestImageView = VK_NULL_HANDLE;
		depthTestImageDeviceMemory = VK_NULL_HANDLE;
	}
}

void Sen_22_DepthTest::updateUniformBuffer() {
	static auto startTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();
	int duration = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count() / 200.0f;

	MvpUniformBufferObject mvpUbo{};
	mvpUbo.model = glm::rotate(glm::mat4(1.0f), -duration * glm::radians(15.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	mvpUbo.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	mvpUbo.projection = glm::perspective(glm::radians(45.0f), m_WidgetWidth / (float)m_WidgetHeight, 0.1f, 100.0f);
	mvpUbo.projection[1][1] *= -1;

	void* data;
	vkMapMemory(m_LogicalDevice, mvpUniformStagingBufferDeviceMemory, 0, sizeof(mvpUbo), 0, &data);
	memcpy(data, &mvpUbo, sizeof(mvpUbo));
	vkUnmapMemory(m_LogicalDevice, mvpUniformStagingBufferDeviceMemory);

	SLVK_AbstractGLFW::transferResourceBuffer(m_DefaultThreadCommandPool, m_LogicalDevice, m_GraphicsQueue, mvpUniformStagingBuffer,
		mvpOptimalUniformBuffer, sizeof(mvpUbo));
}

void Sen_22_DepthTest::finalizeWidget()
{	
	cleanUpDepthStencil();

	/************************************************************************************************************/
	/*********************           Destroy Pipeline, PipelineLayout, and RenderPass         *******************/
	/************************************************************************************************************/
	if (VK_NULL_HANDLE != depthTestPipeline) {
		vkDestroyPipeline(m_LogicalDevice, depthTestPipeline, nullptr);
		vkDestroyPipelineLayout(m_LogicalDevice, textureAppPipelineLayout, nullptr);
		vkDestroyRenderPass(m_LogicalDevice, depthTestRenderPass, nullptr);

		depthTestPipeline			= VK_NULL_HANDLE;
		textureAppPipelineLayout	= VK_NULL_HANDLE;
		depthTestRenderPass			= VK_NULL_HANDLE;
	}
	/************************************************************************************************************/
	/*************      Destroy m_DescriptorPool,  m_Default_DSL,  m_Default_DS      ****************************/
	/************************************************************************************************************/
	if (VK_NULL_HANDLE != m_DescriptorPool) {
		vkDestroyDescriptorPool(m_LogicalDevice, m_DescriptorPool, nullptr);
		// When a DescriptorPool is destroyed, all descriptor sets allocated from the pool are implicitly freed and become invalid
		vkDestroyDescriptorSetLayout(m_LogicalDevice, m_Default_DSL, nullptr);

		m_Default_DSL		= VK_NULL_HANDLE;
		m_DescriptorPool	= VK_NULL_HANDLE;
		m_Default_DS		= VK_NULL_HANDLE;
	}
	/************************************************************************************************************/
	/******************           Destroy Memory, ImageView, Image          *************************************/
	/************************************************************************************************************/
	if (VK_NULL_HANDLE != backgroundTextureImage) {
		vkDestroyImage(m_LogicalDevice, backgroundTextureImage, nullptr);
		if (VK_NULL_HANDLE != backgroundTextureImageView)  
			vkDestroyImageView(m_LogicalDevice, backgroundTextureImageView, nullptr);
		if (VK_NULL_HANDLE != texture2DSampler)  
			vkDestroySampler(m_LogicalDevice, texture2DSampler, nullptr);
		if (VK_NULL_HANDLE != backgroundTextureImageDeviceMemory)
			vkFreeMemory(m_LogicalDevice, backgroundTextureImageDeviceMemory, nullptr); 	// always try to destroy before free

		backgroundTextureImage				= VK_NULL_HANDLE;
		backgroundTextureImageDeviceMemory	= VK_NULL_HANDLE;
		backgroundTextureImageView			= VK_NULL_HANDLE;
		texture2DSampler					= VK_NULL_HANDLE;
	}
	/************************************************************************************************************/
	/******************     Destroy VertexBuffer, VertexBufferMemory     ****************************************/
	/************************************************************************************************************/
	if (VK_NULL_HANDLE != depthTestVertexBuffer) {
		vkDestroyBuffer(m_LogicalDevice, depthTestVertexBuffer, nullptr);
		vkFreeMemory(m_LogicalDevice, depthTestVertexBufferMemory, nullptr);	// always try to destroy before free

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
		vkDestroyPipeline(m_LogicalDevice, depthTestPipeline, nullptr);
		vkDestroyPipelineLayout(m_LogicalDevice, textureAppPipelineLayout, nullptr);

		depthTestPipeline			= VK_NULL_HANDLE;
		textureAppPipelineLayout	= VK_NULL_HANDLE;
	}

	/****************************************************************************************************************************/
	/**********                Reserve pipeline ShaderStage CreateInfos Array           *****************************************/
	/********     Different shader or vertex layout    ==>>   entirely Recreate the graphics pipeline.    ***********************/
	/*--------------------------------------------------------------------------------------------------------------------------*/
	VkShaderModule vertShaderModule, fragShaderModule;

	createVulkanShaderModule(m_LogicalDevice, "SenVulkanTutorial/Shaders/depthTest.vert", vertShaderModule);
	createVulkanShaderModule(m_LogicalDevice, "SenVulkanTutorial/Shaders/depthTest.frag", fragShaderModule);

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
	// Viewport			define HOW,		in which region of the Framebuffer/Widget/GLFW_Window to render; Squeeze your view in your Widget
	m_SwapchainResize_Viewport.x		= 0.0f;									m_SwapchainResize_Viewport.y		= 0.0f;
	m_SwapchainResize_Viewport.width	= static_cast<float>(m_WidgetWidth);	m_SwapchainResize_Viewport.height	= static_cast<float>(m_WidgetHeight);
	m_SwapchainResize_Viewport.minDepth	= 0.0f;									m_SwapchainResize_Viewport.maxDepth	= 1.0f;
	// ScissorRect2D	define WHERE,	which	pixels    of	the Framebuffer  could be rendered;
	//									any		pixels	outside the ScissorRect2D will be discarded by the rasterizer.
	m_SwapchainResize_ScissorRect2D.offset			= { 0, 0 };
	m_SwapchainResize_ScissorRect2D.extent.width	= static_cast<uint32_t>(m_WidgetWidth);
	m_SwapchainResize_ScissorRect2D.extent.height	= static_cast<uint32_t>(m_WidgetHeight);

	VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo{};
	pipelineViewportStateCreateInfo.sType			= VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	pipelineViewportStateCreateInfo.viewportCount	= 1;
	pipelineViewportStateCreateInfo.pViewports		= &m_SwapchainResize_Viewport;
	pipelineViewportStateCreateInfo.scissorCount	= 1;
	pipelineViewportStateCreateInfo.pScissors		= &m_SwapchainResize_ScissorRect2D;

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

	/*********************************************************************************************/
	/*********************************************************************************************/
	std::vector<VkDynamicState> dynamicStateEnablesVector;
	dynamicStateEnablesVector.push_back(VK_DYNAMIC_STATE_VIEWPORT);
	dynamicStateEnablesVector.push_back(VK_DYNAMIC_STATE_SCISSOR);

	VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo{};
	pipelineDynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	pipelineDynamicStateCreateInfo.dynamicStateCount = dynamicStateEnablesVector.size();
	pipelineDynamicStateCreateInfo.pDynamicStates = dynamicStateEnablesVector.data();

	/****************************************************************************************************************************/
	/**********   Reserve pipeline Layout, which help access to descriptor sets from a pipeline       ***************************/
	/****************************************************************************************************************************/
	std::vector<VkDescriptorSetLayout> descriptorSetLayoutVector;
	descriptorSetLayoutVector.push_back(m_Default_DSL);

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
	pipelineLayoutCreateInfo.sType			= VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = descriptorSetLayoutVector.size();
	pipelineLayoutCreateInfo.pSetLayouts	= descriptorSetLayoutVector.data();

	SLVK_AbstractGLFW::errorCheck(
		vkCreatePipelineLayout(m_LogicalDevice, &pipelineLayoutCreateInfo, nullptr, &textureAppPipelineLayout),
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
	depthTestPipelineCreateInfo.pDynamicState		= &pipelineDynamicStateCreateInfo;
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

	SLVK_AbstractGLFW::errorCheck(
		vkCreateGraphicsPipelines(
			m_LogicalDevice, VK_NULL_HANDLE,
			(uint32_t)depthTestGraphicsPipelineCreateInfoVector.size(),
			depthTestGraphicsPipelineCreateInfoVector.data(),
			nullptr,
			&depthTestPipeline), // could be a pipelineArray
		std::string("Failed to create graphics pipeline !!!")
	);

	vkDestroyShaderModule(m_LogicalDevice, vertShaderModule, nullptr);
	vkDestroyShaderModule(m_LogicalDevice, fragShaderModule, nullptr);
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
	SLVK_AbstractGLFW::createResourceBuffer(m_LogicalDevice, indicesBufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE, m_PhysicalDeviceMemoryProperties,
		stagingBuffer, stagingBufferDeviceMemory, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	void* data;
	vkMapMemory(m_LogicalDevice, stagingBufferDeviceMemory, 0, indicesBufferSize, 0, &data);
	memcpy(data, indices, indicesBufferSize);
	//// The driver may not immediately copy the data into the buffer memory, for example because of caching. 
	//// There are two ways to deal with that problem, and what we use is the first one below:
	////  1. Use a memory heap that is host coherent, indicated with VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	////  2. Call vkFlushMappedMemoryRanges to after writing to the mapped memory, and call vkInvalidateMappedMemoryRanges before reading from the mapped memory
	vkUnmapMemory(m_LogicalDevice, stagingBufferDeviceMemory);

	/****************************************************************************************************************************************************/
	/***************   Transfer from stagingBuffer to Optimal triangleVertexBuffer   ********************************************************************/
	SLVK_AbstractGLFW::createResourceBuffer(m_LogicalDevice, indicesBufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE, m_PhysicalDeviceMemoryProperties,
		singleRectIndexBuffer, singleRectIndexBufferMemory, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	SLVK_AbstractGLFW::transferResourceBuffer(m_DefaultThreadCommandPool, m_LogicalDevice, m_GraphicsQueue, stagingBuffer,
		singleRectIndexBuffer, indicesBufferSize);

	vkDestroyBuffer(m_LogicalDevice, stagingBuffer, nullptr);
	vkFreeMemory(m_LogicalDevice, stagingBufferDeviceMemory, nullptr);	// always try to destroy before free
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
	SLVK_AbstractGLFW::createResourceBuffer(m_LogicalDevice, verticesBufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE, m_PhysicalDeviceMemoryProperties,
		stagingBuffer, stagingBufferDeviceMemory, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	void* data;
	vkMapMemory(m_LogicalDevice, stagingBufferDeviceMemory, 0, verticesBufferSize, 0, &data);
	memcpy(data, vertices, verticesBufferSize);
	// The driver may not immediately copy the data into the buffer memory, for example because of caching. 
	// There are two ways to deal with that problem, and what we use is the first one below:
	//  1. Use a memory heap that is host coherent, indicated with VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	//  2. Call vkFlushMappedMemoryRanges to after writing to the mapped memory, and call vkInvalidateMappedMemoryRanges before reading from the mapped memory
	vkUnmapMemory(m_LogicalDevice, stagingBufferDeviceMemory);

	/****************************************************************************************************************************************************/
	/***************   Transfer from stagingBuffer to Optimal depthTestVertexBuffer   ********************************************************************/
	SLVK_AbstractGLFW::createResourceBuffer(m_LogicalDevice, verticesBufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE, m_PhysicalDeviceMemoryProperties,
		depthTestVertexBuffer, depthTestVertexBufferMemory, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	SLVK_AbstractGLFW::transferResourceBuffer(m_DefaultThreadCommandPool, m_LogicalDevice, m_GraphicsQueue, stagingBuffer,
		depthTestVertexBuffer, verticesBufferSize);

	vkDestroyBuffer(m_LogicalDevice, stagingBuffer, nullptr);
	vkFreeMemory(m_LogicalDevice, stagingBufferDeviceMemory, nullptr);	// always try to destroy before free
}

void Sen_22_DepthTest::initBackgroundTextureImage()
{
	SLVK_AbstractGLFW::createDeviceLocalTexture(m_LogicalDevice, m_PhysicalDeviceMemoryProperties
		, backgroundTextureDiskAddress, VK_IMAGE_TYPE_2D, backgroundTextureWidth, backgroundTextureHeight
		, backgroundTextureImage, backgroundTextureImageDeviceMemory, backgroundTextureImageView
		, VK_SHARING_MODE_EXCLUSIVE, m_DefaultThreadCommandPool, m_GraphicsQueue);

	SLVK_AbstractGLFW::createTextureSampler(m_LogicalDevice, texture2DSampler);
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

	SLVK_AbstractGLFW::errorCheck(
		vkCreateDescriptorPool(m_LogicalDevice, &descriptorPoolCreateInfo, nullptr, &m_DescriptorPool),
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
	
	SLVK_AbstractGLFW::errorCheck(
		vkCreateDescriptorSetLayout(m_LogicalDevice, &perspectiveProjectionDSL_CreateInfo, nullptr, &m_Default_DSL),
		std::string("Fail to Create m_Default_DSL !")
	);
}

void Sen_22_DepthTest::createTextureAppDescriptorSet()
{
	std::vector<VkDescriptorSetLayout> descriptorSetLayoutVector;
	descriptorSetLayoutVector.push_back(m_Default_DSL);
	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
	descriptorSetAllocateInfo.sType					= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.descriptorPool		= m_DescriptorPool;
	descriptorSetAllocateInfo.descriptorSetCount	= descriptorSetLayoutVector.size();
	descriptorSetAllocateInfo.pSetLayouts			= descriptorSetLayoutVector.data();

	SLVK_AbstractGLFW::errorCheck(
		vkAllocateDescriptorSets(m_LogicalDevice, &descriptorSetAllocateInfo, &m_Default_DS),
		std::string("Fail to Allocate m_Default_DS !")
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
	uniformBuffer_DS_Write.dstSet			= m_Default_DS;
	uniformBuffer_DS_Write.dstBinding		= m_UniformBuffer_DS_Index;	// binding number, same with the binding index  in shader
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
	combinedImageSampler_DS_Write.dstSet			= m_Default_DS;
	combinedImageSampler_DS_Write.dstBinding		= m_COMBINED_IMAGE_SAMPLER_DS_Index; // binding number, same with the binding index  in shader
	combinedImageSampler_DS_Write.dstArrayElement	= 0;	// start from the index dstArrayElement of pBufferInfo (descriptorBufferInfoVector)
	combinedImageSampler_DS_Write.descriptorCount	= descriptorImageInfoVector.size();// the total number of descriptors to update in pBufferInfo
	combinedImageSampler_DS_Write.pImageInfo		= descriptorImageInfoVector.data();

	std::vector<VkWriteDescriptorSet> DS_Write_Vector;
	DS_Write_Vector.push_back(uniformBuffer_DS_Write);
	DS_Write_Vector.push_back(combinedImageSampler_DS_Write);

	vkUpdateDescriptorSets(m_LogicalDevice, DS_Write_Vector.size(), DS_Write_Vector.data(), 0, nullptr);
}

void Sen_22_DepthTest::createDepthTestCommandBuffers()
{
	/************************************************************************************************************/
	/*********     Destroy old m_SwapchainCommandBufferVector first for widgetRezie, if there are      ************/
	/************************************************************************************************************/
	if (m_SwapchainCommandBufferVector.size() > 0) {
		vkFreeCommandBuffers(m_LogicalDevice, m_DefaultThreadCommandPool, (uint32_t)m_SwapchainCommandBufferVector.size(), m_SwapchainCommandBufferVector.data());
	}
	/****************************************************************************************************************************/
	/**********           Allocate Swapchain CommandBuffers         *************************************************************/
	/****************************************************************************************************************************/
	m_SwapchainCommandBufferVector.resize(m_SwapChain_ImagesCount);

	VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
	commandBufferAllocateInfo.sType			= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandPool	= m_DefaultThreadCommandPool;
	commandBufferAllocateInfo.level			= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount = static_cast<uint32_t>(m_SwapchainCommandBufferVector.size());

	SLVK_AbstractGLFW::errorCheck(
		vkAllocateCommandBuffers(m_LogicalDevice, &commandBufferAllocateInfo, m_SwapchainCommandBufferVector.data()),
		std::string("Failed to allocate Swapchain commandBuffers !!!")
	);

	/****************************************************************************************************************************/
	/**********           Record Triangle Swapchain CommandBuffers        *******************************************************/
	/****************************************************************************************************************************/
	for (size_t i = 0; i < m_SwapchainCommandBufferVector.size(); i++) {
		//======================================================================================
		//======================================================================================
		VkCommandBufferBeginInfo commandBufferBeginInfo{};
		commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT; // In case we may already be scheduling the drawing commands for the next frame while the last frame hass not finished yet.
		vkBeginCommandBuffer(m_SwapchainCommandBufferVector[i], &commandBufferBeginInfo);

		//======================================================================================
		//======================================================================================
		VkRenderPassBeginInfo renderPassBeginInfo{};
		renderPassBeginInfo.sType				= VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass			= depthTestRenderPass;
		renderPassBeginInfo.framebuffer			= m_SwapchainFramebufferVector[i];
		renderPassBeginInfo.renderArea.offset	= { 0, 0 };
		renderPassBeginInfo.renderArea.extent.width		= m_WidgetWidth;
		renderPassBeginInfo.renderArea.extent.height	= m_WidgetHeight;

		// Because we now have both color & depth attachments with VK_ATTACHMENT_LOAD_OP_CLEAR, we also need to specify multiple clear values. 
		std::array<VkClearValue, 2> clearValueArray{};
		clearValueArray[0].color		= { 0.2f, 0.3f, 0.3f, 1.0f };
		clearValueArray[1].depthStencil = { 1.0f, 0 };
		renderPassBeginInfo.clearValueCount = (uint32_t)clearValueArray.size();
		renderPassBeginInfo.pClearValues	= clearValueArray.data();

		vkCmdBeginRenderPass(m_SwapchainCommandBufferVector[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		//======================================================================================
		//======================================================================================
		vkCmdBindPipeline(m_SwapchainCommandBufferVector[i], VK_PIPELINE_BIND_POINT_GRAPHICS, depthTestPipeline);
		VkDeviceSize offsetDeviceSize = 0;
		vkCmdBindVertexBuffers(m_SwapchainCommandBufferVector[i], 0, 1, &depthTestVertexBuffer, &offsetDeviceSize);
		vkCmdBindIndexBuffer(m_SwapchainCommandBufferVector[i], singleRectIndexBuffer, 0, VK_INDEX_TYPE_UINT16);
		vkCmdBindDescriptorSets(m_SwapchainCommandBufferVector[i], VK_PIPELINE_BIND_POINT_GRAPHICS, textureAppPipelineLayout, 0, 1, &m_Default_DS, 0, nullptr);

		//vkCmdDraw(
		//	m_SwapchainCommandBufferVector[i],
		//	3, // vertexCount
		//	1, // instanceCount
		//	0, // firstVertex
		//	0  // firstInstance
		//);
		vkCmdSetViewport(m_SwapchainCommandBufferVector[i], 0, 1, &m_SwapchainResize_Viewport);
		vkCmdSetScissor(m_SwapchainCommandBufferVector[i], 0, 1, &m_SwapchainResize_ScissorRect2D);

		vkCmdDrawIndexed(m_SwapchainCommandBufferVector[i], 12, 1, 0, 0, 0);

		vkCmdEndRenderPass(m_SwapchainCommandBufferVector[i]);

		SLVK_AbstractGLFW::errorCheck(
			vkEndCommandBuffer(m_SwapchainCommandBufferVector[i]),
			std::string("Failed to end record of Triangle Swapchain commandBuffers !!!")
		);
	}
}
