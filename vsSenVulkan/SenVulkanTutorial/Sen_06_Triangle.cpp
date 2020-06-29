
#include "Sen_06_Triangle.h"

Sen_06_Triangle::Sen_06_Triangle()
{
	std::cout << "Constructor: Sen_06_Triangle()\n\n";
	
	strWindowName = "Sen Vulkan Triangle Tutorial";
}

Sen_06_Triangle::~Sen_06_Triangle()
{
	finalizeWidget();

	OutputDebugString("\n\t ~Sen_06_Triangle()\n");
}
/*********************************************************************************************************************/
/*********************************************************************************************************************/
void Sen_06_Triangle::initVulkanApplication()
{
	createColorAttachOnlyRenderPass();

	createTriangleDescriptorSetLayout();
	createTrianglePipeline();
	createColorAttachOnlySwapchainFramebuffers();
	createDefaultCommandPool();

	createTriangleVertexBuffer();
	createSingleRectIndexBuffer();
	createMvpUniformBuffers();
	createTriangleDescriptorPool();
	createTriangleDescriptorSet();
	createTriangleCommandBuffers();

	std::cout << "\n Finish  Sen_06_Triangle::initVulkanApplication()\n";
}

void Sen_06_Triangle::reCreateRenderTarget()
{
	createColorAttachOnlySwapchainFramebuffers();
	createTriangleCommandBuffers();
}

void Sen_06_Triangle::finalizeWidget()
{
	/************************************************************************************************************/
	/*********************           Destroy Pipeline, PipelineLayout, and RenderPass         *******************/
	/************************************************************************************************************/
	if (VK_NULL_HANDLE != trianglePipeline) {
		vkDestroyPipeline(m_LogicalDevice, trianglePipeline, nullptr);
		vkDestroyPipelineLayout(m_LogicalDevice, trianglePipelineLayout, nullptr);
		vkDestroyRenderPass(m_LogicalDevice, m_ColorAttachOnlyRenderPass, nullptr);

		trianglePipeline = VK_NULL_HANDLE;
		trianglePipelineLayout = VK_NULL_HANDLE;
		m_ColorAttachOnlyRenderPass = VK_NULL_HANDLE;
	}
	/************************************************************************************************************/
	/******************     Destroy VertexBuffer, VertexBufferMemory     ****************************************/
	/************************************************************************************************************/
	if (VK_NULL_HANDLE != triangleVertexBuffer) {
		vkDestroyBuffer(m_LogicalDevice, triangleVertexBuffer, nullptr);
		vkFreeMemory(m_LogicalDevice, triangleVertexBufferMemory, nullptr);	// always try to destroy before free

		triangleVertexBuffer = VK_NULL_HANDLE;
		triangleVertexBufferMemory = VK_NULL_HANDLE;
	}
}

void Sen_06_Triangle::updateUniformBuffer()
{
	static auto startTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();
	int duration = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count() / 1000.0f;

	MvpUniformBufferObject mvpUbo{};
	mvpUbo.model = glm::rotate(glm::mat4(1.0f), -duration * glm::radians(15.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	//mvpUbo.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	//mvpUbo.projection = glm::perspective(glm::radians(45.0f), m_WidgetWidth / (float)m_WidgetHeight, 0.1f, 100.0f);
	mvpUbo.projection[1][1] *= -1;

	void* data;
	vkMapMemory(m_LogicalDevice, mvpUniformStagingBufferDeviceMemory, 0, sizeof(mvpUbo), 0, &data);
	memcpy(data, &mvpUbo, sizeof(mvpUbo));
	vkUnmapMemory(m_LogicalDevice, mvpUniformStagingBufferDeviceMemory);

	SLVK_AbstractGLFW::transferResourceBuffer(m_DefaultThreadCommandPool, m_LogicalDevice, graphicsQueue, mvpUniformStagingBuffer,
		mvpOptimalUniformBuffer, sizeof(mvpUbo));
}

// createDescriptorSetLayout() need to be called before createPipeline for the pipelineLayout
void Sen_06_Triangle::createTriangleDescriptorSetLayout() {
	VkDescriptorSetLayoutBinding mvpUboDSL_Binding{};
	mvpUboDSL_Binding.binding = 0;
	mvpUboDSL_Binding.descriptorCount = 1;
	mvpUboDSL_Binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	mvpUboDSL_Binding.pImmutableSamplers = nullptr;
	mvpUboDSL_Binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutCreateInfo mvpUboDescriptorSetLayoutCreateInfo{};
	mvpUboDescriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	mvpUboDescriptorSetLayoutCreateInfo.bindingCount = 1;
	mvpUboDescriptorSetLayoutCreateInfo.pBindings = &mvpUboDSL_Binding;

	SLVK_AbstractGLFW::errorCheck(
		vkCreateDescriptorSetLayout(m_LogicalDevice, &mvpUboDescriptorSetLayoutCreateInfo, nullptr, &perspectiveProjection_DSL),
		std::string("Fail to Create perspectiveProjection_DSL !")
	);
}


void Sen_06_Triangle::createTriangleDescriptorPool() {
	VkDescriptorPoolSize descriptorPoolSize{};
	descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorPoolSize.descriptorCount = 1;

	std::vector<VkDescriptorPoolSize> descriptorPoolSizeVector;
	descriptorPoolSizeVector.push_back(descriptorPoolSize);

	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{};
	descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCreateInfo.poolSizeCount = descriptorPoolSizeVector.size();
	descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizeVector.data();
	descriptorPoolCreateInfo.maxSets = 1; // Need a new descriptorSetVector

	SLVK_AbstractGLFW::errorCheck(
		vkCreateDescriptorPool(m_LogicalDevice, &descriptorPoolCreateInfo, nullptr, &descriptorPool),
		std::string("Fail to Create descriptorPool !")
	);
}

void Sen_06_Triangle::createTriangleDescriptorSet() {
	std::vector<VkDescriptorSetLayout> descriptorSetLayoutVector;
	descriptorSetLayoutVector.push_back(perspectiveProjection_DSL);

	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
	descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.descriptorPool = descriptorPool;
	descriptorSetAllocateInfo.descriptorSetCount = descriptorSetLayoutVector.size();
	descriptorSetAllocateInfo.pSetLayouts = descriptorSetLayoutVector.data();

	SLVK_AbstractGLFW::errorCheck(
		vkAllocateDescriptorSets(m_LogicalDevice, &descriptorSetAllocateInfo, &perspectiveProjection_DS),
		std::string("Fail to Allocate perspectiveProjection_DS !")
	);

	VkDescriptorBufferInfo mvpDescriptorBufferInfo{};
	mvpDescriptorBufferInfo.buffer = mvpOptimalUniformBuffer;
	mvpDescriptorBufferInfo.offset = 0;
	mvpDescriptorBufferInfo.range = sizeof(MvpUniformBufferObject);

	std::vector<VkDescriptorBufferInfo> descriptorBufferInfoVector;
	descriptorBufferInfoVector.push_back(mvpDescriptorBufferInfo);

	VkWriteDescriptorSet writeDescriptorSet{};
	writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writeDescriptorSet.dstSet = perspectiveProjection_DS;
	writeDescriptorSet.dstBinding = 0;	// binding number, same as the binding index specified in shader for a given shader stage
	writeDescriptorSet.dstArrayElement = 0;	// start from the index dstArrayElement of pBufferInfo (descriptorBufferInfoVector)
	writeDescriptorSet.descriptorCount = descriptorBufferInfoVector.size();// the total number of descriptors to update in pBufferInfo
	writeDescriptorSet.pBufferInfo = descriptorBufferInfoVector.data();

	vkUpdateDescriptorSets(m_LogicalDevice, 1, &writeDescriptorSet, 0, nullptr);
}

void Sen_06_Triangle::createTrianglePipeline() {
	if (VK_NULL_HANDLE != trianglePipeline) {
		vkDestroyPipeline(m_LogicalDevice, trianglePipeline, nullptr);
		vkDestroyPipelineLayout(m_LogicalDevice, trianglePipelineLayout, nullptr);

		trianglePipeline = VK_NULL_HANDLE;
		trianglePipelineLayout = VK_NULL_HANDLE;
	}

	/****************************************************************************************************************************/
	/**********                Reserve pipeline ShaderStage CreateInfos Array           *****************************************/
	/********     Different shader or vertex layout    ==>>   entirely Recreate the graphics pipeline.    ***********************/
	/*--------------------------------------------------------------------------------------------------------------------------*/
	VkShaderModule vertShaderModule, fragShaderModule;

	createVulkanShaderModule(m_LogicalDevice, "SenVulkanTutorial/Shaders/Triangle.vert", vertShaderModule);
	createVulkanShaderModule(m_LogicalDevice, "SenVulkanTutorial/Shaders/Triangle.frag", fragShaderModule);

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
	vertexInputBindingDescription.stride = 5 * sizeof(float);
	vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	std::vector<VkVertexInputBindingDescription> vertexInputBindingDescriptionVector;
	vertexInputBindingDescriptionVector.push_back(vertexInputBindingDescription);


	VkVertexInputAttributeDescription positionVertexInputAttributeDescription;
	positionVertexInputAttributeDescription.location = 0;
	positionVertexInputAttributeDescription.binding = 0;
	positionVertexInputAttributeDescription.format = VK_FORMAT_R32G32_SFLOAT;
	positionVertexInputAttributeDescription.offset = 0;
	VkVertexInputAttributeDescription colorVertexInputAttributeDescription;
	colorVertexInputAttributeDescription.location = 1;
	colorVertexInputAttributeDescription.binding = 0;
	colorVertexInputAttributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
	colorVertexInputAttributeDescription.offset = 2 * sizeof(float);

	std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescriptionVector;
	vertexInputAttributeDescriptionVector.push_back(positionVertexInputAttributeDescription);
	vertexInputAttributeDescriptionVector.push_back(colorVertexInputAttributeDescription);

	VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo{};
	pipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	pipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = vertexInputBindingDescriptionVector.size();// spacing between data && whether the data is per-vertex or per-instance (geometry instancing)
	pipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = vertexInputBindingDescriptionVector.data();
	pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = vertexInputAttributeDescriptionVector.size();
	pipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = vertexInputAttributeDescriptionVector.data();


	VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo{};
	pipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	pipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	pipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

	/*********************************************************************************************/
	/*********************************************************************************************/
	m_SwapchainResize_Viewport.x		= 0.0f;									m_SwapchainResize_Viewport.y		= 0.0f;
	m_SwapchainResize_Viewport.width	= static_cast<float>(m_WidgetWidth);		m_SwapchainResize_Viewport.height	= static_cast<float>(m_WidgetHeight);
	m_SwapchainResize_Viewport.minDepth	= 0.0f;									m_SwapchainResize_Viewport.maxDepth	= 1.0f;

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
	pipelineColorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
		| VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	pipelineColorBlendAttachmentState.blendEnable = VK_FALSE;
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

	/*********************************************************************************************/
	/*********************************************************************************************/
	std::vector<VkDynamicState> dynamicStateEnablesVector;
	dynamicStateEnablesVector.push_back(VK_DYNAMIC_STATE_VIEWPORT);
	dynamicStateEnablesVector.push_back(VK_DYNAMIC_STATE_SCISSOR);

	VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo{};
	pipelineDynamicStateCreateInfo.sType				= VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	pipelineDynamicStateCreateInfo.dynamicStateCount	= dynamicStateEnablesVector.size();
	pipelineDynamicStateCreateInfo.pDynamicStates		= dynamicStateEnablesVector.data();

	/****************************************************************************************************************************/
	/**********   Reserve pipeline Layout, which help access to descriptor sets from a pipeline       ***************************/
	/****************************************************************************************************************************/
	std::vector<VkDescriptorSetLayout> descriptorSetLayoutVector;
	descriptorSetLayoutVector.push_back(perspectiveProjection_DSL);

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = descriptorSetLayoutVector.size();
	pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayoutVector.data();


	SLVK_AbstractGLFW::errorCheck(
		vkCreatePipelineLayout(m_LogicalDevice, &pipelineLayoutCreateInfo, nullptr, &trianglePipelineLayout),
		std::string("Failed to to create pipeline layout !!!")
	);

	/****************************************************************************************************************************/
	/**********                Create   Pipeline            *********************************************************************/
	/****************************************************************************************************************************/
	std::vector<VkGraphicsPipelineCreateInfo> graphicsPipelineCreateInfoVector;
	VkGraphicsPipelineCreateInfo trianglePipelineCreateInfo{};
	trianglePipelineCreateInfo.sType				= VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	trianglePipelineCreateInfo.stageCount			= (uint32_t)pipelineShaderStagesCreateInfoVector.size();
	trianglePipelineCreateInfo.pStages				= pipelineShaderStagesCreateInfoVector.data();
	trianglePipelineCreateInfo.pDynamicState		= &pipelineDynamicStateCreateInfo;
	trianglePipelineCreateInfo.pVertexInputState	= &pipelineVertexInputStateCreateInfo;
	trianglePipelineCreateInfo.pInputAssemblyState	= &pipelineInputAssemblyStateCreateInfo;
	trianglePipelineCreateInfo.pViewportState		= &pipelineViewportStateCreateInfo;
	trianglePipelineCreateInfo.pRasterizationState	= &pipelineRasterizationStateCreateInfo;
	trianglePipelineCreateInfo.pMultisampleState	= &pipelineMultisampleStateCreateInfo;
	trianglePipelineCreateInfo.pColorBlendState		= &pipelineColorBlendStateCreateInfo;
	trianglePipelineCreateInfo.layout				= trianglePipelineLayout;
	trianglePipelineCreateInfo.renderPass			= m_ColorAttachOnlyRenderPass;
	trianglePipelineCreateInfo.subpass				= 0; // index of this trianglePipeline's subpass of the m_ColorAttachOnlyRenderPass
														//trianglePipelineCreateInfo.basePipelineHandle	= VK_NULL_HANDLE;

	graphicsPipelineCreateInfoVector.push_back(trianglePipelineCreateInfo);

	SLVK_AbstractGLFW::errorCheck(
		vkCreateGraphicsPipelines(
			m_LogicalDevice, VK_NULL_HANDLE,
			(uint32_t)graphicsPipelineCreateInfoVector.size(),
			graphicsPipelineCreateInfoVector.data(),
			nullptr,
			&trianglePipeline), // could be a pipelineArray
		std::string("Failed to create graphics pipeline !!!")
	);

	vkDestroyShaderModule(m_LogicalDevice, vertShaderModule, nullptr);
	vkDestroyShaderModule(m_LogicalDevice, fragShaderModule, nullptr);
}


void Sen_06_Triangle::createTriangleVertexBuffer() {
	float vertices[] = {
		// Positions	// Colors
		-0.1f,	-0.75f,	1.0f,	0.0f,	0.0f,  // Bottom Right
		0.0f,	-0.75f,	0.0f,	0.0f,	1.0f,  // Bottom Left
		0.0f,	0.75f,	1.0f,	1.0f,	1.0f,   // Top Right
		0.1f,	0.75f,	0.0f,	1.0f,	0.0f   // Top Left
	};
	size_t verticesBufferSize = sizeof(vertices);

	/****************************************************************************************************************************************************/
	/***************   Create temporary stagingBuffer to transfer from to get Optimal Buffer Resource   *************************************************/
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferDeviceMemory;
	SLVK_AbstractGLFW::createResourceBuffer(m_LogicalDevice, verticesBufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE, physicalDeviceMemoryProperties,
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
	/***************   Transfer from stagingBuffer to Optimal triangleVertexBuffer   ********************************************************************/
	SLVK_AbstractGLFW::createResourceBuffer(m_LogicalDevice, verticesBufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE, physicalDeviceMemoryProperties,
		triangleVertexBuffer, triangleVertexBufferMemory, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	SLVK_AbstractGLFW::transferResourceBuffer(m_DefaultThreadCommandPool, m_LogicalDevice, graphicsQueue, stagingBuffer,
		triangleVertexBuffer, verticesBufferSize);

	vkDestroyBuffer(m_LogicalDevice, stagingBuffer, nullptr);
	vkFreeMemory(m_LogicalDevice, stagingBufferDeviceMemory, nullptr);	// always try to destroy before free
}

void Sen_06_Triangle::createTriangleCommandBuffers() {
	if (m_SwapchainCommandBufferVector.size() > 0) {
		vkFreeCommandBuffers(m_LogicalDevice, m_DefaultThreadCommandPool, (uint32_t)m_SwapchainCommandBufferVector.size(), m_SwapchainCommandBufferVector.data());
	}
	/****************************************************************************************************************************/
	/**********           Allocate Swapchain CommandBuffers         *************************************************************/
	/****************************************************************************************************************************/
	m_SwapchainCommandBufferVector.resize(m_SwapChain_ImagesCount);

	VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
	commandBufferAllocateInfo.sType					= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandPool			= m_DefaultThreadCommandPool;
	commandBufferAllocateInfo.level					= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount	= static_cast<uint32_t>(m_SwapchainCommandBufferVector.size());

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
		//commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT; // In case we may already be scheduling the drawing commands for the next frame while the last frame hass not finished yet.
		vkBeginCommandBuffer(m_SwapchainCommandBufferVector[i], &commandBufferBeginInfo);

		//======================================================================================
		//======================================================================================
		VkRenderPassBeginInfo renderPassBeginInfo{};
		renderPassBeginInfo.sType				= VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass			= m_ColorAttachOnlyRenderPass;
		renderPassBeginInfo.framebuffer			= m_SwapchainFramebufferVector[i];
		renderPassBeginInfo.renderArea.offset	= { 0, 0 };
		renderPassBeginInfo.renderArea.extent.width		= m_WidgetWidth;
		renderPassBeginInfo.renderArea.extent.height	= m_WidgetHeight;

		std::vector<VkClearValue> clearValueVector;
		clearValueVector.push_back(VkClearValue{ 0.2f, 0.3f, 0.3f, 1.0f });
		renderPassBeginInfo.clearValueCount = (uint32_t)clearValueVector.size();
		renderPassBeginInfo.pClearValues = clearValueVector.data();

		vkCmdBeginRenderPass(m_SwapchainCommandBufferVector[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		//======================================================================================
		//======================================================================================
		vkCmdBindPipeline(m_SwapchainCommandBufferVector[i], VK_PIPELINE_BIND_POINT_GRAPHICS, trianglePipeline);
		VkDeviceSize offsetDeviceSize = 0;
		vkCmdBindVertexBuffers(m_SwapchainCommandBufferVector[i], 0, 1, &triangleVertexBuffer, &offsetDeviceSize);
		vkCmdBindIndexBuffer(m_SwapchainCommandBufferVector[i], singleRectIndexBuffer, 0, VK_INDEX_TYPE_UINT16);
		vkCmdBindDescriptorSets(m_SwapchainCommandBufferVector[i], VK_PIPELINE_BIND_POINT_GRAPHICS, trianglePipelineLayout, 0, 1, &perspectiveProjection_DS, 0, nullptr);

		//vkCmdDraw(
		//	m_SwapchainCommandBufferVector[i],
		//	3, // vertexCount
		//	1, // instanceCount
		//	0, // firstVertex
		//	0  // firstInstance
		//);

		vkCmdSetViewport(m_SwapchainCommandBufferVector[i], 0, 1, &m_SwapchainResize_Viewport);
		vkCmdSetScissor(m_SwapchainCommandBufferVector[i], 0, 1, &m_SwapchainResize_ScissorRect2D);

		vkCmdDrawIndexed(m_SwapchainCommandBufferVector[i], 6, 1, 0, 0, 0);

		vkCmdEndRenderPass(m_SwapchainCommandBufferVector[i]);

		SLVK_AbstractGLFW::errorCheck(
			vkEndCommandBuffer(m_SwapchainCommandBufferVector[i]),
			std::string("Failed to end record of Triangle Swapchain commandBuffers !!!")
		);
	}
}
