#include "Sen_222_TinyObjLoader.h"

//#include "../Support/SenVulkanMeshStruct.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>			// for tinyObjLoader

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

namespace std {
	template<> struct hash<VertexStruct> {
		size_t operator()(VertexStruct const& vertex) const {
			return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}

Sen_222_TinyObjLoader::Sen_222_TinyObjLoader()
{
	std::cout << "Constructor: Sen_222_TinyObjLoader()\n\n";
	strWindowName = "Sen Vulkan Cube Tutorial";

	tinyObjCompleteTextureDiskAddress	= "../Images/MeshLinkModels/Chalet/chalet.jpg";
	tinyMeshLinkModelDiskAddress		= "../Images/MeshLinkModels/Chalet/chalet.obj";
}

Sen_222_TinyObjLoader::~Sen_222_TinyObjLoader()
{
	finalizeWidget();

	OutputDebugString("\n\t ~Sen_222_TinyObjLoader()\n");
}

void Sen_222_TinyObjLoader::loadModel() {
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, tinyMeshLinkModelDiskAddress)) {
		throw std::runtime_error(err);
	}

	std::unordered_map<VertexStruct, uint32_t> uniqueVertices = {};

	//for (const auto& shape : shapes) {
	//	for (const auto& index : shape.mesh.indices) {
	//		VertexStruct vertex = {};

	//		vertex.pos = {
	//			attrib.vertices[3 * index.vertex_index + 0],
	//			attrib.vertices[3 * index.vertex_index + 1],
	//			attrib.vertices[3 * index.vertex_index + 2]
	//		};

	//		vertex.texCoord = {
	//			attrib.texcoords[2 * index.texcoord_index + 0],
	//			1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
	//		};

	//		vertex.color = { 1.0f, 1.0f, 1.0f };

	//		if (uniqueVertices.count(vertex) == 0) {
	//			uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
	//			vertices.push_back(vertex);
	//		}

	//		indices.push_back(uniqueVertices[vertex]);
	//	}
	//}
}

void Sen_222_TinyObjLoader::initVulkanApplication()
{
	createTextureAppDescriptorSetLayout();
	createDefaultCommandPool();

	initTinyObjCompleteTextureImage();
	createMvpUniformBuffers();
	createTextureAppDescriptorPool();
	createTextureAppDescriptorSet();

	/***************************************/
	createDepthTestAttachment();			// has to be called after createDefaultCommandPool();
	createDepthTestRenderPass();			// has to be called after createDepthTestAttachment() for depthTestFormat
	createTinyObjLoaderPipeline();
	createDepthTestSwapchainFramebuffers(); // has to be called after createDepthTestAttachment() for the depthTestImageView
	createMeshLinkModeVertexBuffer();
	createMeshLinkModelndexBuffer();
	/***************************************/

	createTinyObjLoaderCommandBuffers();

	std::cout << "\n Finish  Sen_222_TinyObjLoader::initVulkanApplication()\n";
}

void Sen_222_TinyObjLoader::reCreateRenderTarget()
{
	createDepthTestAttachment();
	createDepthTestSwapchainFramebuffers();
	createTinyObjLoaderCommandBuffers();
}

void Sen_222_TinyObjLoader::cleanUpDepthStencil()
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

void Sen_222_TinyObjLoader::updateUniformBuffer() {
	static auto startTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();
	float duration = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count() / 220.0f;

	MvpUniformBufferObject mvpUbo{};
	mvpUbo.model = glm::rotate(glm::mat4(), duration * glm::radians(15.0f), glm::vec3(-1.0f, 1.0f, 1.0f))
				* glm::rotate(glm::mat4(), duration * glm::radians(3.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	mvpUbo.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.5f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	mvpUbo.projection = glm::perspective(glm::radians(45.0f), widgetWidth / (float)widgetHeight, 0.1f, 100.0f);
	mvpUbo.projection[1][1] *= -1;

	void* data;
	vkMapMemory(device, mvpUniformStagingBufferDeviceMemory, 0, sizeof(mvpUbo), 0, &data);
	memcpy(data, &mvpUbo, sizeof(mvpUbo));
	vkUnmapMemory(device, mvpUniformStagingBufferDeviceMemory);

	SenAbstractGLFW::transferResourceBuffer(defaultThreadCommandPool, device, graphicsQueue, mvpUniformStagingBuffer,
		mvpOptimalUniformBuffer, sizeof(mvpUbo));
}

void Sen_222_TinyObjLoader::finalizeWidget()
{	
	cleanUpDepthStencil();

	/************************************************************************************************************/
	/******************           Destroy Memory, ImageView, Image          *************************************/
	/************************************************************************************************************/
	if (VK_NULL_HANDLE != tinyObjCompleteImage) {
		vkDestroyImage(device, tinyObjCompleteImage, nullptr);
		if (VK_NULL_HANDLE != tinyObjCompleteImageView)  
			vkDestroyImageView(device, tinyObjCompleteImageView, nullptr);
		if (VK_NULL_HANDLE != texture2DSampler)  
			vkDestroySampler(device, texture2DSampler, nullptr);
		if (VK_NULL_HANDLE != tinyObjCompleteImageDeviceMemory)
			vkFreeMemory(device, tinyObjCompleteImageDeviceMemory, nullptr); 	// always try to destroy before free

		tinyObjCompleteImage				= VK_NULL_HANDLE;
		tinyObjCompleteImageDeviceMemory	= VK_NULL_HANDLE;
		tinyObjCompleteImageView			= VK_NULL_HANDLE;
		texture2DSampler					= VK_NULL_HANDLE;
	}
	/************************************************************************************************************/
	/*********************           Destroy Pipeline, PipelineLayout, and RenderPass         *******************/
	/************************************************************************************************************/
	if (VK_NULL_HANDLE != tinyObjLoaderPipeline) {
		vkDestroyPipeline(device, tinyObjLoaderPipeline, nullptr);
		vkDestroyPipelineLayout(device, tinyObjLoaderPipelineLayout, nullptr);
		vkDestroyRenderPass(device, depthTestRenderPass, nullptr);

		tinyObjLoaderPipeline			= VK_NULL_HANDLE;
		tinyObjLoaderPipelineLayout	= VK_NULL_HANDLE;
		depthTestRenderPass			= VK_NULL_HANDLE;
	}
	/************************************************************************************************************/
	/******************     Destroy VertexBuffer, VertexBufferMemory     ****************************************/
	/************************************************************************************************************/
	if (VK_NULL_HANDLE != tinyMeshLinkModelVertexBuffer) {
		vkDestroyBuffer(device, tinyMeshLinkModelVertexBuffer, nullptr);
		vkFreeMemory(device, tinyMeshLinkModelVertexBufferMemory, nullptr);	// always try to destroy before free

		tinyMeshLinkModelVertexBuffer			= VK_NULL_HANDLE;
		tinyMeshLinkModelVertexBufferMemory	= VK_NULL_HANDLE;
	}
	if (VK_NULL_HANDLE != tinyMeshLinkModelIndexBuffer) {
		vkDestroyBuffer(device, tinyMeshLinkModelIndexBuffer, nullptr);
		vkFreeMemory(device, tinyMeshLinkModelIndexBufferMemory, nullptr);	// always try to destroy before free

		tinyMeshLinkModelIndexBuffer = VK_NULL_HANDLE;
		tinyMeshLinkModelIndexBufferMemory = VK_NULL_HANDLE;
	}
	OutputDebugString("\n\tFinish  Sen_222_TinyObjLoader::finalizeWidget()\n");
}

void Sen_222_TinyObjLoader::createTinyObjLoaderPipeline()
{
	/************************************************************************************************************/
	/*********     Destroy old tinyObjLoaderPipeline first for widgetRezie, if there are      ***********************/
	/************************************************************************************************************/
	if (VK_NULL_HANDLE != tinyObjLoaderPipeline) {
		vkDestroyPipeline(device, tinyObjLoaderPipeline, nullptr);
		vkDestroyPipelineLayout(device, tinyObjLoaderPipelineLayout, nullptr);

		tinyObjLoaderPipeline			= VK_NULL_HANDLE;
		tinyObjLoaderPipelineLayout	= VK_NULL_HANDLE;
	}

	/****************************************************************************************************************************/
	/**********                Reserve pipeline ShaderStage CreateInfos Array           *****************************************/
	/****************************************************************************************************************************/
	VkShaderModule vertShaderModule, fragShaderModule;

	createVulkanShaderModule(device, "SenVulkanTutorial/Shaders/SenCube.vert", vertShaderModule);
	createVulkanShaderModule(device, "SenVulkanTutorial/Shaders/SenCube.frag", fragShaderModule);

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
	vertexInputBindingDescription.stride	= 5 * sizeof(float);
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
	colorVertexInputAttributeDescription.format			= VK_FORMAT_R32G32_SFLOAT;
	colorVertexInputAttributeDescription.offset			= 3 * sizeof(float);
	vertexInputAttributeDescriptionVector.push_back(colorVertexInputAttributeDescription);

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
	resizeViewport.x		= 0.0f;									resizeViewport.y		= 0.0f;
	resizeViewport.width	= static_cast<float>(widgetWidth);		resizeViewport.height	= static_cast<float>(widgetHeight);
	resizeViewport.minDepth	= 0.0f;									resizeViewport.maxDepth	= 1.0f;

	resizeScissorRect2D.offset			= { 0, 0 };
	resizeScissorRect2D.extent.width	= static_cast<uint32_t>(widgetWidth);
	resizeScissorRect2D.extent.height	= static_cast<uint32_t>(widgetHeight);

	VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo{};
	pipelineViewportStateCreateInfo.sType			= VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	pipelineViewportStateCreateInfo.viewportCount	= 1;
	pipelineViewportStateCreateInfo.pViewports		= &resizeViewport;
	pipelineViewportStateCreateInfo.scissorCount	= 1;
	pipelineViewportStateCreateInfo.pScissors		= &resizeScissorRect2D;

	/*********************************************************************************************/
	/*********************************************************************************************/
	VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo{};
	pipelineRasterizationStateCreateInfo.sType						= VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	pipelineRasterizationStateCreateInfo.depthClampEnable			= VK_FALSE;
	pipelineRasterizationStateCreateInfo.rasterizerDiscardEnable	= VK_FALSE;
	pipelineRasterizationStateCreateInfo.polygonMode				= VK_POLYGON_MODE_FILL;
	pipelineRasterizationStateCreateInfo.cullMode					= VK_CULL_MODE_BACK_BIT; 
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
	descriptorSetLayoutVector.push_back(perspectiveProjection_DSL);

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
	pipelineLayoutCreateInfo.sType			= VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = descriptorSetLayoutVector.size();
	pipelineLayoutCreateInfo.pSetLayouts	= descriptorSetLayoutVector.data();

	SenAbstractGLFW::errorCheck(
		vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &tinyObjLoaderPipelineLayout),
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
	depthTestPipelineCreateInfo.layout				= tinyObjLoaderPipelineLayout;
	depthTestPipelineCreateInfo.renderPass			= depthTestRenderPass;
	depthTestPipelineCreateInfo.subpass				= 0;	// index of this tinyObjLoaderPipeline's subpass of the depthTestRenderPass
															//depthTestPipelineCreateInfo.basePipelineHandle	= VK_NULL_HANDLE;

	depthTestGraphicsPipelineCreateInfoVector.push_back(depthTestPipelineCreateInfo);

	SenAbstractGLFW::errorCheck(
		vkCreateGraphicsPipelines(
			device, VK_NULL_HANDLE,
			(uint32_t)depthTestGraphicsPipelineCreateInfoVector.size(),
			depthTestGraphicsPipelineCreateInfoVector.data(),
			nullptr,
			&tinyObjLoaderPipeline), // could be a pipelineArray
		std::string("Failed to create graphics pipeline !!!")
	);

	vkDestroyShaderModule(device, vertShaderModule, nullptr);
	vkDestroyShaderModule(device, fragShaderModule, nullptr);
}

void Sen_222_TinyObjLoader::createMeshLinkModelndexBuffer()
{
	//uint16_t indices[] = {	0, 1, 2, 1, 2, 3, 
	//						4, 5, 6, 5, 6, 7 };

	uint32_t indices[] = {  // Note that we start from 0!
		0,	1,	3,	// Front First Triangle
		1,	2,	3,	// Front Second Triangle
		4,	5,	7,	// Back First Triangle
		5,	6,	7,	// Back Second Triangle
		8,	9,	11,	// Left First Triangle
		9,	10,	11,	// Left Second Triangle
		12,	13,	15,	// Right First Triangle
		13,	14,	15,	// Right Second Triangle
		16,	17,	19,	// Top First Triangle
		17,	18,	19,	// Top Second Triangle
		20,	21,	23,	// Bottom First Triangle
		21,	22,	23	// Bottom Second Triangle
	};
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
		tinyMeshLinkModelIndexBuffer, tinyMeshLinkModelIndexBufferMemory, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	SenAbstractGLFW::transferResourceBuffer(defaultThreadCommandPool, device, graphicsQueue, stagingBuffer,
		tinyMeshLinkModelIndexBuffer, indicesBufferSize);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferDeviceMemory, nullptr);	// always try to destroy before free
}

void Sen_222_TinyObjLoader::createMeshLinkModeVertexBuffer()
{
	//float vertices[] = {
	//	// Positions			// Colors				// TexCoord
	//	-0.5f,	-0.5f,	0.5f,	1.0f,	0.0f,	0.0f,	0.0f,	0.0f,	// Bottom Right
	//	0.5f,	-0.5f,	0.5f,	0.0f,	0.0f,	1.0f,	1.0f,	0.0f,	// Bottom Left
	//	-0.5f,	0.5f,	0.5f,	1.0f,	1.0f,	1.0f,   0.0f,	1.0f,	// Top Right
	//	0.5f,	0.5f,	0.5f,	0.0f,	1.0f,	0.0f,	1.0f,	1.0f,   // Top Left

	//	-0.5f,	-0.5f,	-0.5f,	1.0f,	0.0f,	0.0f,	0.0f,	0.0f,	// Bottom Right
	//	0.5f,	-0.5f,	-0.5f,	0.0f,	0.0f,	1.0f,	1.0f,	0.0f,	// Bottom Left
	//	-0.5f,	0.5f,	-0.5f,	1.0f,	1.0f,	1.0f,   0.0f,	1.0f,	// Top Right
	//	0.5f,	0.5f,	-0.5f,	0.0f,	1.0f,	0.0f,	1.0f,	1.0f   // Top Left
	//};

	float vertices[] = {
		// Positions           // Texture Coords
		-0.5f, 0.5f, -0.5f, 1.0f, 0.0f, // Front Top Right
		-0.5f, -0.5f, -0.5f, 1.0f, 1.0f, // Front Bottom Right
		0.5f, -0.5f, -0.5f, 0.0f, 1.0f, // Front Bottom Left
		0.5f, 0.5f, -0.5f, 0.0f, 0.0f, // Front Top Left 

		0.5f, 0.5f, 0.5f, 1.0f, 0.0f, // Back Top Right
		0.5f, -0.5f, 0.5f, 1.0f, 1.0f, // Back Bottom Right
		-0.5f, -0.5f, 0.5f, 0.0f, 1.0f, // Back Bottom Left
		-0.5f, 0.5f, 0.5f, 0.0f, 0.0f,  // Back Top Left 

		-0.5f, 0.5f, 0.5f, 1.0f, 0.0f, // Left Top Right
		-0.5f, -0.5f, 0.5f, 1.0f, 1.0f, // Left Bottom Right
		-0.5f, -0.5f, -0.5f, 0.0f, 1.0f, // Left Bottom Left
		-0.5f, 0.5f, -0.5f, 0.0f, 0.0f, // Left Top Left 

		0.5f, 0.5f, -0.5f, 1.0f, 0.0f, // Right Top Right
		0.5f, -0.5f, -0.5f, 1.0f, 1.0f, // Right Bottom Right
		0.5f, -0.5f, 0.5f, 0.0f, 1.0f, // Right Bottom Left
		0.5f, 0.5f, 0.5f, 0.0f, 0.0f, // Right Top Left 

		0.5f, 0.5f, -0.5f, 1.0f, 0.0f, // Top Top Right
		0.5f, 0.5f, 0.5f, 1.0f, 1.0f, // Top Bottom Right
		-0.5f, 0.5f, 0.5f, 0.0f, 1.0f, // Top Bottom Left
		-0.5f, 0.5f, -0.5f, 0.0f, 0.0f, // Top Top Left 

		-0.5f, -0.5f, -0.5f, 1.0f, 0.0f, // Bottom Top Right
		-0.5f, -0.5f, 0.5f, 1.0f, 1.0f, // Bottom Bottom Right
		0.5f, -0.5f, 0.5f, 0.0f, 1.0f, // Bottom Bottom Left
		0.5f, -0.5f, -0.5f, 0.0f, 0.0f // Bottom Top Left 
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
	/***************   Transfer from stagingBuffer to Optimal tinyMeshLinkModelVertexBuffer   ********************************************************************/
	SenAbstractGLFW::createResourceBuffer(device, verticesBufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE, physicalDeviceMemoryProperties,
		tinyMeshLinkModelVertexBuffer, tinyMeshLinkModelVertexBufferMemory, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	SenAbstractGLFW::transferResourceBuffer(defaultThreadCommandPool, device, graphicsQueue, stagingBuffer,
		tinyMeshLinkModelVertexBuffer, verticesBufferSize);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferDeviceMemory, nullptr);	// always try to destroy before free
}

void Sen_222_TinyObjLoader::initTinyObjCompleteTextureImage()
{
	SenAbstractGLFW::createDeviceLocalTexture(device, physicalDeviceMemoryProperties
		, tinyObjCompleteTextureDiskAddress, VK_IMAGE_TYPE_2D, tinyObjCompleteTextureWidth, tinyObjCompleteTextureHeight
		, tinyObjCompleteImage, tinyObjCompleteImageDeviceMemory, tinyObjCompleteImageView
		, VK_SHARING_MODE_EXCLUSIVE, defaultThreadCommandPool, graphicsQueue);

	SenAbstractGLFW::createTextureSampler(device, texture2DSampler);
}

void Sen_222_TinyObjLoader::createTextureAppDescriptorPool()
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

void Sen_222_TinyObjLoader::createTextureAppDescriptorSetLayout()
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

void Sen_222_TinyObjLoader::createTextureAppDescriptorSet()
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
	backgroundTextureDescriptorImageInfo.imageView		= tinyObjCompleteImageView;
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

void Sen_222_TinyObjLoader::createTinyObjLoaderCommandBuffers()
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
	commandBufferAllocateInfo.sType			= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandPool	= defaultThreadCommandPool;
	commandBufferAllocateInfo.level			= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
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
		vkCmdBindPipeline(swapchainCommandBufferVector[i], VK_PIPELINE_BIND_POINT_GRAPHICS, tinyObjLoaderPipeline);
		VkDeviceSize offsetDeviceSize = 0;
		vkCmdBindVertexBuffers(swapchainCommandBufferVector[i], 0, 1, &tinyMeshLinkModelVertexBuffer, &offsetDeviceSize);
		vkCmdBindIndexBuffer(swapchainCommandBufferVector[i], tinyMeshLinkModelIndexBuffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdBindDescriptorSets(swapchainCommandBufferVector[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
			tinyObjLoaderPipelineLayout, 0, 1, &perspectiveProjection_DS, 0, nullptr);

		//vkCmdDraw(
		//	swapchainCommandBufferVector[i],
		//	3, // vertexCount
		//	1, // instanceCount
		//	0, // firstVertex
		//	0  // firstInstance
		//);
		vkCmdSetViewport(swapchainCommandBufferVector[i], 0, 1, &resizeViewport);
		vkCmdSetScissor(swapchainCommandBufferVector[i], 0, 1, &resizeScissorRect2D);

		vkCmdDrawIndexed(swapchainCommandBufferVector[i], 6*6, 1, 0, 0, 0);

		vkCmdEndRenderPass(swapchainCommandBufferVector[i]);

		SenAbstractGLFW::errorCheck(
			vkEndCommandBuffer(swapchainCommandBufferVector[i]),
			std::string("Failed to end record of Triangle Swapchain commandBuffers !!!")
		);
	}
}
