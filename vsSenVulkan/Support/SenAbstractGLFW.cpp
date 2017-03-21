#include "SenAbstractGLFW.h"

SenAbstractGLFW::SenAbstractGLFW()
//:xRot(0), yRot(0), aspect(1.0)
{
	//showAllSupportedInstanceExtensions(); // Not Useful Functions
	//showAllSupportedInstanceLayers(); // Not Useful Functions
	//showAllSupportedExtensionsEachUnderInstanceLayer(); // Not Useful Functions

	strWindowName = "Sen GLFW Vulkan Application";

	widgetWidth = DEFAULT_widgetWidth;
	widgetHeight = DEFAULT_widgetHeight;
}

SenAbstractGLFW::~SenAbstractGLFW()
{
	finalize();
	OutputDebugString("\n ~SenAbstractGLWidget()\n");
}

void SenAbstractGLFW::paintVulkan(void)
{
	// make sure we indeed get the surface size we want.
	glfwGetFramebufferSize(widgetGLFW, &widgetWidth, &widgetHeight);
	// Define the viewport dimensions
	//glViewport(0, 0, widgetWidth, widgetHeight);
}

void SenAbstractGLFW::initGlfwVulkan()
{
	// Init GLFW
	glfwInit();
	// Set all the required options for GLFW
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); //tell GLFW to not create an OpenGL context 
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	// Create a GLFWwindow object that we can use for GLFW's functions
	widgetGLFW = glfwCreateWindow(widgetWidth, widgetHeight, strWindowName, nullptr, nullptr);
	glfwSetWindowPos(widgetGLFW, 400, 240);
	glfwMakeContextCurrent(widgetGLFW);

	/*****************************************************************************************************************************/

	// Set the required callback functions
	//keyboardRegister();

	if (layersEnabled) {
		initDebugLayers();
	}
	initExtensions();
	createInstance();
	if (layersEnabled) {
		initDebugReportCallback(); // Need created Instance
	}

	/*******************************************************************************************************************************/
	/********* The window surface needs to be created right after the instance creation, *******************************************/
	/********* because the check of "surface" support will influence the physical device selection.     ****************************/
	createSurface(); // surface == default framebuffer to draw
	pickPhysicalDevice();
	//showPhysicalDeviceSupportedLayersAndExtensions(physicalDevice);// only show physicalDevice after pickPhysicalDevice()
	createLogicalDevice();

	createSwapChain();

	// Clear the colorbuffer
	//glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	//glViewport(0, 0, widgetWidth, widgetHeight);
	//glEnable(GL_DEPTH_TEST);

	std::cout << "\n Finish  initGlfwVulkan()\n";
}

void SenAbstractGLFW::showWidget()
{
	initGlfwVulkan();

	// Game loop
	while (!glfwWindowShouldClose(widgetGLFW))
	{
		// Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
		glfwPollEvents();

		// Render
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//paintVulkan();

		// Swap the screen buffers
		//glfwSwapBuffers(widgetGLFW);
	}


	finalize();// all the clean up works

	// Terminate GLFW, clearing any resources allocated by GLFW.
	glfwDestroyWindow(widgetGLFW);
	glfwTerminate();
}


void SenAbstractGLFW::createSwapChain() {
	/****************************************************************************************************************************/
	/********** Initial Surface Info first to support SwapChain. ****************************************************************/
	/*********** Could not get surface info right after surface creation, because GPU had not been seleted at that time *********/
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);
	// Do the check & assignment below because what we surface size we got may not equal to what we set
	// Make sure the size of swapchain match the size of surface
	if (surfaceCapabilities.currentExtent.width < UINT32_MAX) {
		widgetWidth = surfaceCapabilities.currentExtent.width;
		widgetHeight = surfaceCapabilities.currentExtent.height;
	} 
	
	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
	if (formatCount == 0) {
		throw std::runtime_error("No SurfaceFormat found, not a suitable GPU!");
	}

	surfaceFormatVector.clear();
	surfaceFormatVector.resize(formatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, surfaceFormatVector.data());
	if (surfaceFormatVector[0].format == VK_FORMAT_UNDEFINED) { // the prasentation layer (WSI) doesnot care about the format
		surfaceFormat.format = VK_FORMAT_B8G8R8A8_UNORM;
		surfaceFormat.colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
	}else {
		surfaceFormat = surfaceFormatVector[0];
	}



	//SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

	//VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	//VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	//VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

//	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
//	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
//		imageCount = swapChainSupport.capabilities.maxImageCount;
//	}
//
//	VkSwapchainCreateInfoKHR createInfo = {};
//	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
//	createInfo.surface = surface;
//
//	createInfo.minImageCount = imageCount;
//	createInfo.imageFormat = surfaceFormat.format;
//	createInfo.imageColorSpace = surfaceFormat.colorSpace;
//	createInfo.imageExtent = extent;
//	createInfo.imageArrayLayers = 1;
//	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
//
//	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
//	uint32_t queueFamilyIndices[] = { static_cast<uint32_t>(indices.graphicsFamily), static_cast<uint32_t>(indices.presentFamily) };
//
//	if (indices.graphicsFamily != indices.presentFamily) {
//		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
//		createInfo.queueFamilyIndexCount = 2;
//		createInfo.pQueueFamilyIndices = queueFamilyIndices;
//	}
//	else {
//		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
//	}
//
//	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
//	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
//	createInfo.presentMode = presentMode;
//	createInfo.clipped = VK_TRUE;
//
//	createInfo.oldSwapchain = VK_NULL_HANDLE;
//
//	if (vkCreateSwapchainKHR(device, &createInfo, nullptr, swapChain.replace()) != VK_SUCCESS) {
//		throw std::runtime_error("failed to create swap chain!");
//	}
//
//	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
//	swapChainImages.resize(imageCount);
//	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());
//
//	swapChainImageFormat = surfaceFormat.format;
//	swapChainExtent = extent;
//}
//
//void SenAbstractGLFW::createImageViews() {
//	swapChainImageViews.resize(swapChainImages.size(), VDeleter<VkImageView>{device, vkDestroyImageView});
//
//	for (uint32_t i = 0; i < swapChainImages.size(); i++) {
//		VkImageViewCreateInfo createInfo = {};
//		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
//		createInfo.image = swapChainImages[i];
//		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
//		createInfo.format = swapChainImageFormat;
//		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
//		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
//		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
//		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
//		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//		createInfo.subresourceRange.baseMipLevel = 0;
//		createInfo.subresourceRange.levelCount = 1;
//		createInfo.subresourceRange.baseArrayLayer = 0;
//		createInfo.subresourceRange.layerCount = 1;
//
//		if (vkCreateImageView(device, &createInfo, nullptr, swapChainImageViews[i].replace()) != VK_SUCCESS) {
//			throw std::runtime_error("failed to create image views!");
//		}
//	}
}

//void SenAbstractGLFW::createRenderPass() {
//	VkAttachmentDescription colorAttachment = {};
//	colorAttachment.format = swapChainImageFormat;
//	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
//	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
//	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
//	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
//	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
//	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
//
//	VkAttachmentReference colorAttachmentRef = {};
//	colorAttachmentRef.attachment = 0;
//	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//
//	VkSubpassDescription subpass = {};
//	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
//	subpass.colorAttachmentCount = 1;
//	subpass.pColorAttachments = &colorAttachmentRef;
//
//	VkSubpassDependency dependency = {};
//	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
//	dependency.dstSubpass = 0;
//	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//	dependency.srcAccessMask = 0;
//	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//
//	VkRenderPassCreateInfo renderPassInfo = {};
//	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
//	renderPassInfo.attachmentCount = 1;
//	renderPassInfo.pAttachments = &colorAttachment;
//	renderPassInfo.subpassCount = 1;
//	renderPassInfo.pSubpasses = &subpass;
//	renderPassInfo.dependencyCount = 1;
//	renderPassInfo.pDependencies = &dependency;
//
//	if (vkCreateRenderPass(device, &renderPassInfo, nullptr, renderPass.replace()) != VK_SUCCESS) {
//		throw std::runtime_error("failed to create render pass!");
//	}
//}
//
//void SenAbstractGLFW::createGraphicsPipeline() {
//	auto vertShaderCode = readFile("SenVulkanTutorial/Shaders/Triangle.vert");
//	auto fragShaderCode = readFile("SenVulkanTutorial/Shaders/Triangle.frag");
//
//	VDeleter<VkShaderModule> vertShaderModule{ device, vkDestroyShaderModule };
//	VDeleter<VkShaderModule> fragShaderModule{ device, vkDestroyShaderModule };
//	createShaderModule(vertShaderCode, vertShaderModule);
//	createShaderModule(fragShaderCode, fragShaderModule);
//
//	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
//	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
//	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
//	vertShaderStageInfo.module = vertShaderModule;
//	vertShaderStageInfo.pName = "main";
//
//	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
//	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
//	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
//	fragShaderStageInfo.module = fragShaderModule;
//	fragShaderStageInfo.pName = "main";
//
//	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };
//
//	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
//	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
//	vertexInputInfo.vertexBindingDescriptionCount = 0;
//	vertexInputInfo.vertexAttributeDescriptionCount = 0;
//
//	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
//	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
//	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
//	inputAssembly.primitiveRestartEnable = VK_FALSE;
//
//	VkViewport viewport = {};
//	viewport.x = 0.0f;
//	viewport.y = 0.0f;
//	viewport.width = (float)swapChainExtent.width;
//	viewport.height = (float)swapChainExtent.height;
//	viewport.minDepth = 0.0f;
//	viewport.maxDepth = 1.0f;
//
//	VkRect2D scissor = {};
//	scissor.offset = { 0, 0 };
//	scissor.extent = swapChainExtent;
//
//	VkPipelineViewportStateCreateInfo viewportState = {};
//	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
//	viewportState.viewportCount = 1;
//	viewportState.pViewports = &viewport;
//	viewportState.scissorCount = 1;
//	viewportState.pScissors = &scissor;
//
//	VkPipelineRasterizationStateCreateInfo rasterizer = {};
//	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
//	rasterizer.depthClampEnable = VK_FALSE;
//	rasterizer.rasterizerDiscardEnable = VK_FALSE;
//	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
//	rasterizer.lineWidth = 1.0f;
//	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
//	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
//	rasterizer.depthBiasEnable = VK_FALSE;
//
//	VkPipelineMultisampleStateCreateInfo multisampling = {};
//	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
//	multisampling.sampleShadingEnable = VK_FALSE;
//	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
//
//	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
//	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
//	colorBlendAttachment.blendEnable = VK_FALSE;
//
//	VkPipelineColorBlendStateCreateInfo colorBlending = {};
//	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
//	colorBlending.logicOpEnable = VK_FALSE;
//	colorBlending.logicOp = VK_LOGIC_OP_COPY;
//	colorBlending.attachmentCount = 1;
//	colorBlending.pAttachments = &colorBlendAttachment;
//	colorBlending.blendConstants[0] = 0.0f;
//	colorBlending.blendConstants[1] = 0.0f;
//	colorBlending.blendConstants[2] = 0.0f;
//	colorBlending.blendConstants[3] = 0.0f;
//
//	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
//	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
//	pipelineLayoutInfo.setLayoutCount = 0;
//	pipelineLayoutInfo.pushConstantRangeCount = 0;
//
//	if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, pipelineLayout.replace()) != VK_SUCCESS) {
//		throw std::runtime_error("failed to create pipeline layout!");
//	}
//
//	VkGraphicsPipelineCreateInfo pipelineInfo = {};
//	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
//	pipelineInfo.stageCount = 2;
//	pipelineInfo.pStages = shaderStages;
//	pipelineInfo.pVertexInputState = &vertexInputInfo;
//	pipelineInfo.pInputAssemblyState = &inputAssembly;
//	pipelineInfo.pViewportState = &viewportState;
//	pipelineInfo.pRasterizationState = &rasterizer;
//	pipelineInfo.pMultisampleState = &multisampling;
//	pipelineInfo.pColorBlendState = &colorBlending;
//	pipelineInfo.layout = pipelineLayout;
//	pipelineInfo.renderPass = renderPass;
//	pipelineInfo.subpass = 0;
//	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
//
//	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, graphicsPipeline.replace()) != VK_SUCCESS) {
//		throw std::runtime_error("failed to create graphics pipeline!");
//	}
//}
//
//void SenAbstractGLFW::createFramebuffers() {
//	swapChainFramebuffers.resize(swapChainImageViews.size(), VDeleter<VkFramebuffer>{device, vkDestroyFramebuffer});
//
//	for (size_t i = 0; i < swapChainImageViews.size(); i++) {
//		VkImageView attachments[] = {
//			swapChainImageViews[i]
//		};
//
//		VkFramebufferCreateInfo framebufferInfo = {};
//		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
//		framebufferInfo.renderPass = renderPass;
//		framebufferInfo.attachmentCount = 1;
//		framebufferInfo.pAttachments = attachments;
//		framebufferInfo.width = swapChainExtent.width;
//		framebufferInfo.height = swapChainExtent.height;
//		framebufferInfo.layers = 1;
//
//		if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, swapChainFramebuffers[i].replace()) != VK_SUCCESS) {
//			throw std::runtime_error("failed to create framebuffer!");
//		}
//	}
//}
//
//void SenAbstractGLFW::createCommandPool() {
//	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);
//
//	VkCommandPoolCreateInfo poolInfo = {};
//	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
//	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
//
//	if (vkCreateCommandPool(device, &poolInfo, nullptr, commandPool.replace()) != VK_SUCCESS) {
//		throw std::runtime_error("failed to create command pool!");
//	}
//}
//
//void SenAbstractGLFW::createCommandBuffers() {
//	commandBuffers.resize(swapChainFramebuffers.size());
//
//	VkCommandBufferAllocateInfo allocInfo = {};
//	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
//	allocInfo.commandPool = commandPool;
//	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
//	allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());
//
//	if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
//		throw std::runtime_error("failed to allocate command buffers!");
//	}
//
//	for (size_t i = 0; i < commandBuffers.size(); i++) {
//		VkCommandBufferBeginInfo beginInfo = {};
//		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
//
//		vkBeginCommandBuffer(commandBuffers[i], &beginInfo);
//
//		VkRenderPassBeginInfo renderPassInfo = {};
//		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
//		renderPassInfo.renderPass = renderPass;
//		renderPassInfo.framebuffer = swapChainFramebuffers[i];
//		renderPassInfo.renderArea.offset = { 0, 0 };
//		renderPassInfo.renderArea.extent = swapChainExtent;
//
//		VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
//		renderPassInfo.clearValueCount = 1;
//		renderPassInfo.pClearValues = &clearColor;
//
//		vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
//
//		vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
//
//		vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);
//
//		vkCmdEndRenderPass(commandBuffers[i]);
//
//		if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
//			throw std::runtime_error("failed to record command buffer!");
//		}
//	}
//}
//
//void SenAbstractGLFW::createSemaphores() {
//	VkSemaphoreCreateInfo semaphoreInfo = {};
//	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
//
//	if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, imageAvailableSemaphore.replace()) != VK_SUCCESS ||
//		vkCreateSemaphore(device, &semaphoreInfo, nullptr, renderFinishedSemaphore.replace()) != VK_SUCCESS) {
//
//		throw std::runtime_error("failed to create semaphores!");
//	}
//}
/****************************************************************************************************************************/
/****************************************************************************************************************************/
/****************************************************************************************************************************/
VKAPI_ATTR VkBool32 VKAPI_CALL SenAbstractGLFW::pfnDebugCallback(
	VkFlags msgFlags,
	VkDebugReportObjectTypeEXT objType,
	uint64_t srcObject,
	size_t location,
	int32_t msgCode,
	const char * layerPrefix,
	const char * msg,
	void * userData
)
{
	std::ostringstream stream;
	if (msgFlags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) { stream << "INFO:\t"; }
	if (msgFlags & VK_DEBUG_REPORT_WARNING_BIT_EXT) { stream << "WARNING:\t"; }
	if (msgFlags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) { stream << "PERFORMANCE:\t"; }
	if (msgFlags & VK_DEBUG_REPORT_ERROR_BIT_EXT) { stream << "ERROR:\t"; }
	if (msgFlags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) { stream << "DEBUG:\t"; }
	stream << "@[" << layerPrefix << "]: \t";
	stream << msg << std::endl;
	std::cout << stream.str();

#ifdef _WIN32
	if (msgFlags & VK_DEBUG_REPORT_ERROR_BIT_EXT)	MessageBox(NULL, stream.str().c_str(), "Vulkan Error!", 0);
#endif

	return VK_FALSE;
}

/*******************************************************************
* 1. Call initDebugLayers() before createInstance() to track the instance creation procedure.
* Sum:
*********************************************************************/
void SenAbstractGLFW::initDebugLayers()
{
	// choose layers
	debugInstanceLayersVector.push_back("VK_LAYER_LUNARG_standard_validation");
	debugDeviceLayersVector.push_back("VK_LAYER_LUNARG_standard_validation");				// depricated

	// check layer support
	if (!checkInstanceLayersSupport(debugInstanceLayersVector)) {
		throw std::runtime_error("Layer Support Error!");
	}

	// init Debug report callback
	debugReportCallbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
	debugReportCallbackCreateInfo.pfnCallback = pfnDebugCallback;
	debugReportCallbackCreateInfo.flags =
		//VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
		VK_DEBUG_REPORT_WARNING_BIT_EXT |
		VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
		VK_DEBUG_REPORT_ERROR_BIT_EXT |
		//VK_DEBUG_REPORT_DEBUG_BIT_EXT |
		0;
}


void SenAbstractGLFW::initExtensions()
{
	/*****************************************************************************************************************************/
	/*************  For Instance Extensions  *************************************************************************************/
	/*****************************************************************************************************************************/
	uint32_t glfwInstanceExtensionsCount = 0;
	const char** glfwInstanceExtensions;

	glfwInstanceExtensions = glfwGetRequiredInstanceExtensions(&glfwInstanceExtensionsCount);
	//std::cout << "\nGLFW required Vulkan Instance Extensions: \n");
	for (uint32_t i = 0; i < glfwInstanceExtensionsCount; i++) {
		debugInstanceExtensionsVector.push_back(glfwInstanceExtensions[i]);
		//std::string strExtension = std::to_string(i) + ". " + std::string(glfwInstanceExtensions[i]) + "\n";
		//std::cout << strExtension;
	}
	if (layersEnabled) {
		debugInstanceExtensionsVector.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	}

	/*****************************************************************************************************************************/
	/*************  For Physical Device Extensions  ******************************************************************************/
	/*****************************************************************************************************************************/
	debugDeviceExtensionsVector.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME); //There is no quote here
}

/*******************************************************************
* 1. vkCreateInstance requires VkInstanceCreateInfo
* 2. VkInstanceCreateInfo requires VkApplicationInfo, setup of extensions (instance and device) and layers
* Sum: createInstance requires basic appInfo, InstanceExtensionInfo, DeviceExtensionInfo and LayerInfo
*********************************************************************/
void SenAbstractGLFW::createInstance()
{
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Sen Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);

	VkInstanceCreateInfo instanceCreateInfo = {};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pApplicationInfo = &appInfo;

	instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(debugInstanceExtensionsVector.size());
	instanceCreateInfo.ppEnabledExtensionNames = debugInstanceExtensionsVector.data();

	if (layersEnabled) {
		instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(debugInstanceLayersVector.size());
		instanceCreateInfo.ppEnabledLayerNames = debugInstanceLayersVector.data();
		instanceCreateInfo.pNext = &debugReportCallbackCreateInfo;
	}

	errorCheck(
		vkCreateInstance(&instanceCreateInfo, nullptr, &instance),
		std::string("Failed to create instance! \t Error:\t")
	);

	//if (vkCreateInstance(&instanceCreateInfo, nullptr, &instance) != VK_SUCCESS) {
	//	std::string errString = std::string("Failed to create instance!");
	//	throw std::runtime_error(errString);
	//}
}

void SenAbstractGLFW::initDebugReportCallback()
{
	fetch_vkCreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT"));
	fetch_vkDestroyDebugReportCallbackEXT = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT"));

	if (nullptr == fetch_vkCreateDebugReportCallbackEXT || nullptr == fetch_vkDestroyDebugReportCallbackEXT) {
		throw std::runtime_error("Vulkan Error: Can't fetch debug function pointers.");
		std::exit(-1);
	}

	errorCheck(
		fetch_vkCreateDebugReportCallbackEXT(instance, &debugReportCallbackCreateInfo, nullptr, &debugReportCallback),
		std::string("Create debugReportCallback Error!")
	);
}

void SenAbstractGLFW::pickPhysicalDevice()
{
	uint32_t physicalDevicesCount = 0;
	vkEnumeratePhysicalDevices(instance, &physicalDevicesCount, nullptr);

	if (physicalDevicesCount == 0) {
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> physicalDevicesVector(physicalDevicesCount);
	vkEnumeratePhysicalDevices(instance, &physicalDevicesCount, physicalDevicesVector.data());

	/******************************************************************************************************/
	/******** Select the first suitable GPU  **************************************************************/

	//for (const auto& detectedGPU : physicalDevicesVector) {
	//	if (isPhysicalDeviceSuitable(detectedGPU, graphicsQueueFamilyIndex, presentQueueFamilyIndex)) {
	//		physicalDevice = detectedGPU;
	//		break;
	//	}
	//}

	//if (VK_NULL_HANDLE == physicalDevice) {
	//	throw std::runtime_error("failed to find a suitable GPU!");
	//}

	/******************************************************************************************************/
	/******** Rate all available PhysicalDevices and  Pick the best suitable one  *************************/
	/******** Use an ordered map to automatically sort candidates by increasing score *********************/

	std::multimap<int, VkPhysicalDevice> physicalDevicesScoredMap;
	std::cout << "All Detected GPUs Properties: \n";
	for (int i = 0; i < physicalDevicesVector.size(); i++) {
		int score = ratePhysicalDevice(physicalDevicesVector[i], graphicsQueueFamilyIndex, presentQueueFamilyIndex);// Primary check function in this block
		std::cout << "\t[" << i + 1 << "]\t";	showPhysicalDeviceInfo(physicalDevicesVector[i]);
		physicalDevicesScoredMap.insert(std::make_pair(score, physicalDevicesVector[i]));
	}
	// Check if the best candidate is suitable at all
	if (physicalDevicesScoredMap.rbegin()->first > 0) {
		physicalDevice = physicalDevicesScoredMap.rbegin()->second;
	}
	else {
		throw std::runtime_error("failed to find a suitable GPU!");
	}
	/******************************************************************************************************/
	/******************************************************************************************************/
	std::cout << "\n\nSelected GPU Properties:\n\t\t";	showPhysicalDeviceInfo(physicalDevice);
	std::cout << std::endl;
}

void SenAbstractGLFW::createSurface()
{
	errorCheck(
		glfwCreateWindowSurface(instance, widgetGLFW, nullptr, &surface),
		std::string("Failed to create window surface!")
	);
}

void SenAbstractGLFW::showPhysicalDeviceInfo(const VkPhysicalDevice & gpuToCheck)
{
	if (VK_NULL_HANDLE == gpuToCheck) 		throw std::runtime_error("Wrong GPU!");

	VkPhysicalDeviceProperties physicalDeviceProperties{};
	vkGetPhysicalDeviceProperties(gpuToCheck, &physicalDeviceProperties);
	std::ostringstream stream;
	std::cout << " GPU Name: [" << physicalDeviceProperties.deviceName << "]\tType: \"";
	switch (physicalDeviceProperties.deviceType) {
	case 0:			stream << "VK_PHYSICAL_DEVICE_TYPE_OTHER\"\n ";				break;
	case 1:			stream << "VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU\"\n ";	break;
	case 2:			stream << "VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU\"\n ";		break;
	case 3:			stream << "VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU\"\n ";		break;
	case 4:			stream << "VK_PHYSICAL_DEVICE_TYPE_CPU\"\n ";				break;
	case 5:			stream << "VK_PHYSICAL_DEVICE_TYPE_RANGE_SIZE\"\n ";		break;
	default:		stream << "Unrecognized GPU Property.deviceType! \n";		break;
	}
	std::cout << stream.str();
}

bool SenAbstractGLFW::isPhysicalDeviceSuitable(const VkPhysicalDevice& gpuToCheck, int32_t& graphicsQueueIndex, int32_t& presentQueueIndex)
{
	graphicsQueueIndex = -1; presentQueueIndex = -1;
	if (VK_NULL_HANDLE == gpuToCheck) return VK_FALSE;
	/**************************************************************************************************************/
	/**************************************************************************************************************/
	VkPhysicalDeviceProperties physicalDeviceProperties{};
	vkGetPhysicalDeviceProperties(gpuToCheck, &physicalDeviceProperties);
	VkPhysicalDeviceFeatures physicalDeviceFeatures{};
	vkGetPhysicalDeviceFeatures(gpuToCheck, &physicalDeviceFeatures);

	if (!physicalDeviceFeatures.geometryShader)		return VK_FALSE;

	/**************************************************************************************************************/
	/**************************************************************************************************************/
	//  Check Graphics Queue Family support of GPU and get QueueFamilyIndex of Graphics 
	uint32_t gpuQueueFamiliesCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(gpuToCheck, &gpuQueueFamiliesCount, nullptr);
	std::vector<VkQueueFamilyProperties> gpuQueueFamilyVector(gpuQueueFamiliesCount);
	vkGetPhysicalDeviceQueueFamilyProperties(gpuToCheck, &gpuQueueFamiliesCount, gpuQueueFamilyVector.data());

	for (uint32_t i = 0; i < gpuQueueFamiliesCount; i++) {
		if (graphicsQueueIndex < 0 && gpuQueueFamilyVector[i].queueCount > 0
			&& gpuQueueFamilyVector[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			graphicsQueueIndex = i;
		}

		if (presentQueueIndex < 0 && gpuQueueFamilyVector[i].queueCount > 0) {
			VkBool32 presentSupport = VK_FALSE;// WSI_supported, or surface support
			vkGetPhysicalDeviceSurfaceSupportKHR(gpuToCheck, i, surface, &presentSupport);
			if (presentSupport) {
				presentQueueIndex = i;
			}
		}

		if (graphicsQueueIndex >= 0 && presentQueueIndex >= 0)
			break;
	}

	return graphicsQueueIndex >= 0 && presentQueueIndex >= 0;

	//QueueFamilyIndices indices = findQueueFamilies(gpuToCheck);
	//bool extensionsSupported = checkDeviceExtensionSupport(gpuToCheck);// Not necessary

	//bool swapChainAdequate = false;
	//if (extensionsSupported) {
	//	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(gpuToCheck);
	//	swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	//}

	//return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

int SenAbstractGLFW::ratePhysicalDevice(const VkPhysicalDevice & gpuToCheck, int32_t& graphicsQueueIndex, int32_t& presentQueueIndex)
{
	graphicsQueueIndex = -1; presentQueueIndex = -1;
	if (VK_NULL_HANDLE == gpuToCheck) return 0;

	/************************************************************************************************************/
	/******* Check Graphics Queue Family support of GPU and get QueueFamilyIndex of Graphics ********************/
	/************************************************************************************************************/
	uint32_t gpuQueueFamiliesCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(gpuToCheck, &gpuQueueFamiliesCount, nullptr);
	std::vector<VkQueueFamilyProperties> gpuQueueFamilyVector(gpuQueueFamiliesCount);
	vkGetPhysicalDeviceQueueFamilyProperties(gpuToCheck, &gpuQueueFamiliesCount, gpuQueueFamilyVector.data());

	//	1. Get the number of Queues supported by the Physical device
	//	2. Get the properties each Queue type or Queue Family
	//			There could be 4 Queue type or Queue families supported by physical device - 
	//			Graphics Queue	- VK_QUEUE_GRAPHICS_BIT 
	//			Compute Queue	- VK_QUEUE_COMPUTE_BIT
	//			DMA				- VK_QUEUE_TRANSFER_BIT
	//			Sparse memory	- VK_QUEUE_SPARSE_BINDING_BIT
	//	3. Get the index ID for the required Queue family, this ID will act like a handle index to queue.

	for (uint32_t i = 0; i < gpuQueueFamiliesCount; i++) {
		if (graphicsQueueIndex < 0 && gpuQueueFamilyVector[i].queueCount > 0
			&& gpuQueueFamilyVector[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			graphicsQueueIndex = i;
		}

		if (presentQueueIndex < 0 && gpuQueueFamilyVector[i].queueCount > 0) {
			VkBool32 presentSupport = VK_FALSE;// WSI_supported, or surface support
			vkGetPhysicalDeviceSurfaceSupportKHR(gpuToCheck, i, surface, &presentSupport);
			if (presentSupport) {
				presentQueueIndex = i;
			}
		}

		if (graphicsQueueIndex >= 0 && presentQueueIndex >= 0)
			break;
	}
	if (graphicsQueueIndex < 0 || presentQueueIndex < 0) return 0; // If graphics QueueFamilyIndex is still -1 as default, this GPU doesn't support Graphics drawing

	/************************************************************************************************************/
	/******* If gets here, Graphics Queue Family support of this this GPU is good *******************************/
	/************************************************************************************************************/
	VkPhysicalDeviceFeatures physicalDeviceFeatures{};
	vkGetPhysicalDeviceFeatures(gpuToCheck, &physicalDeviceFeatures);
	if (!physicalDeviceFeatures.geometryShader) 	return 0;	// Application can't function without geometry shaders

	/************************************************************************************************************/
	/****** If gets here, Geometry Shader of this GPU is good, let's Rate Score *********************************/
	/************************************************************************************************************/
	int score = 0;
	VkPhysicalDeviceProperties physicalDeviceProperties{};
	vkGetPhysicalDeviceProperties(gpuToCheck, &physicalDeviceProperties);
	if (physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
		score += 1000;	// Discrete GPUs have a significant performance advantage
	}
	score += physicalDeviceProperties.limits.maxImageDimension2D;// Maximum possible size of textures affects graphics quality

	return score;
}

void SenAbstractGLFW::createLogicalDevice()
{
	std::vector<VkDeviceQueueCreateInfo> deviceQueuesCreateInfosVector;
	/*************************************************************************************************************/
	/*** Attension! Two Queues for one logical device have to use two different QueueFamilyIndex *****************/
	/*************************************************************************************************************/
	std::set<int> uniqueQueueFamilyIndicesSet = { graphicsQueueFamilyIndex, presentQueueFamilyIndex };

	float queuePriority = 1.0f;
	for (int uniqueQueueFamilyIndex : uniqueQueueFamilyIndicesSet) {
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = uniqueQueueFamilyIndex;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;

		deviceQueuesCreateInfosVector.push_back(queueCreateInfo);
	}

	VkDeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(deviceQueuesCreateInfosVector.size());
	deviceCreateInfo.pQueueCreateInfos = deviceQueuesCreateInfosVector.data();

	if (layersEnabled) {
		deviceCreateInfo.enabledLayerCount = debugDeviceLayersVector.size();   				// depricated
		deviceCreateInfo.ppEnabledLayerNames = debugDeviceLayersVector.data();				// depricated
	}
	deviceCreateInfo.enabledExtensionCount = debugDeviceExtensionsVector.size();
	deviceCreateInfo.ppEnabledExtensionNames = debugDeviceExtensionsVector.data();

	errorCheck(
		vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device),
		std::string("Fail at Create Logical Device!")
	);

	// Retrieve queue handles for each queue family
	vkGetDeviceQueue(device, graphicsQueueFamilyIndex, 0, &graphicsQueue);
	vkGetDeviceQueue(device, presentQueueFamilyIndex, 0, &presentQueue); // Not sure if the third parameter 0 is correct
}

void SenAbstractGLFW::finalize() {
	// Destroy logical device
	if (VK_NULL_HANDLE != device) {
		vkDestroyDevice(device, VK_NULL_HANDLE);

		// Device queues are implicitly cleaned up when the device is destroyed
		if (VK_NULL_HANDLE != graphicsQueue) { graphicsQueue = VK_NULL_HANDLE; }
		if (VK_NULL_HANDLE != presentQueue) { presentQueue = VK_NULL_HANDLE; }

		device = VK_NULL_HANDLE;
	}

	// Destroy window surface, Note that this is a native Vulkan API function
	if (VK_NULL_HANDLE != surface) {
		vkDestroySurfaceKHR(instance, surface, nullptr);	//  surface was created with GLFW function
		surface = VK_NULL_HANDLE;
	}

	//Must destroy debugReportCallback before destroy instance
	if (layersEnabled) {
		if (VK_NULL_HANDLE != debugReportCallback) {
			fetch_vkDestroyDebugReportCallbackEXT(instance, debugReportCallback, VK_NULL_HANDLE);
			debugReportCallback = VK_NULL_HANDLE;
		}
	}

	// Destroy Instance
	if (VK_NULL_HANDLE != instance) {
		vkDestroyInstance(instance, VK_NULL_HANDLE); 	instance = VK_NULL_HANDLE;
	}
}



bool SenAbstractGLFW::checkInstanceLayersSupport(std::vector<const char*> layersVector) {
	uint32_t layersCount;
	vkEnumerateInstanceLayerProperties(&layersCount, nullptr);

	std::vector<VkLayerProperties> supportedInstanceLayersVector(layersCount);
	vkEnumerateInstanceLayerProperties(&layersCount, supportedInstanceLayersVector.data());

	for (const char* layerNameToTest : layersVector) {
		bool layerFound = false;

		for (const auto& supportedLayerProperties : supportedInstanceLayersVector) {
			if (std::strcmp(layerNameToTest, supportedLayerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			return false;
		}
	}

	return true;
}




void SenAbstractGLFW::errorCheck(VkResult result, std::string msg)
{
	if (result != VK_SUCCESS) {

		std::string errString;
		switch (result)
		{
		case VK_NOT_READY:
			errString = "VK_NOT_READY    \n";
			break;
		case VK_TIMEOUT:
			errString = "VK_TIMEOUT    \n";
			break;
		case VK_EVENT_SET:
			errString = "VK_EVENT_SET    \n";
			break;
		case VK_EVENT_RESET:
			errString = "VK_EVENT_RESET    \n";
			break;
		case VK_INCOMPLETE:
			errString = "VK_INCOMPLETE    \n";
			break;

		case VK_ERROR_OUT_OF_HOST_MEMORY:
			errString = "VK_ERROR_OUT_OF_HOST_MEMORY    \n";
			break;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:
			errString = "VK_ERROR_OUT_OF_DEVICE_MEMORY    \n";
			break;
		case VK_ERROR_INITIALIZATION_FAILED:
			errString = "VK_ERROR_INITIALIZATION_FAILED    \n";
			break;
		case VK_ERROR_DEVICE_LOST:
			errString = "VK_ERROR_DEVICE_LOST    \n";
			break;
		case VK_ERROR_MEMORY_MAP_FAILED:
			errString = "VK_ERROR_MEMORY_MAP_FAILED    \n";
			break;
		case VK_ERROR_LAYER_NOT_PRESENT:
			errString = "VK_ERROR_LAYER_NOT_PRESENT    \n";
			break;
		case VK_ERROR_EXTENSION_NOT_PRESENT:
			errString = "VK_ERROR_EXTENSION_NOT_PRESENT    \n";
			break;
		case VK_ERROR_FEATURE_NOT_PRESENT:
			errString = "VK_ERROR_FEATURE_NOT_PRESENT    \n";
			break;
		case VK_ERROR_INCOMPATIBLE_DRIVER:
			errString = "VK_ERROR_INCOMPATIBLE_DRIVER    \n";
			break;
		case VK_ERROR_TOO_MANY_OBJECTS:
			errString = "VK_ERROR_TOO_MANY_OBJECTS    \n";
			break;
		case VK_ERROR_FORMAT_NOT_SUPPORTED:
			errString = "VK_ERROR_FORMAT_NOT_SUPPORTED    \n";
			break;
		case VK_ERROR_FRAGMENTED_POOL:
			errString = "VK_ERROR_FRAGMENTED_POOL    \n";
			break;
		case VK_ERROR_SURFACE_LOST_KHR:
			errString = "VK_ERROR_SURFACE_LOST_KHR    \n";
			break;
		case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
			errString = "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR    \n";
			break;
		case VK_SUBOPTIMAL_KHR:
			errString = "VK_SUBOPTIMAL_KHR    \n";
			break;
		case VK_ERROR_OUT_OF_DATE_KHR:
			errString = "VK_ERROR_OUT_OF_DATE_KHR    \n";
			break;
		case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
			errString = "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR    \n";
			break;
		case VK_ERROR_VALIDATION_FAILED_EXT:
			errString = "VK_ERROR_VALIDATION_FAILED_EXT    \n";
			break;
		case VK_ERROR_INVALID_SHADER_NV:
			errString = "VK_ERROR_INVALID_SHADER_NV    \n";
			break;
		case VK_ERROR_OUT_OF_POOL_MEMORY_KHR:
			errString = "VK_ERROR_OUT_OF_POOL_MEMORY_KHR    \n";
			break;
		case VK_ERROR_INVALID_EXTERNAL_HANDLE_KHX:
			errString = "VK_ERROR_INVALID_EXTERNAL_HANDLE_KHX    \n";
			break;
		default:
			std::cout << result << std::endl;
			errString = "\n\n Cannot tell error type, check std::cout!    \n";
			break;
		}

		throw std::runtime_error(msg + "\t" + errString);
	}
}


//*************************************************/
//*************                       *************/
//*************  Not Useful Functions *************/
//*************       Below           *************/
//*************************************************/
void SenAbstractGLFW::showAllSupportedInstanceExtensions()
{
	uint32_t extensionsCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, nullptr);

	std::vector<VkExtensionProperties> instanceExtensionsVector(extensionsCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, instanceExtensionsVector.data());

	std::ostringstream stream;
	stream << "\nAll Supported Instance Extensions: \n";
	for (uint32_t i = 0; i < extensionsCount; i++) {
		stream << "\t" << i + 1 << ". " << instanceExtensionsVector[i].extensionName << "\n";
	}
	std::cout << stream.str();
}

void SenAbstractGLFW::showAllSupportedInstanceLayers()
{
	uint32_t layersCount = 0;
	vkEnumerateInstanceLayerProperties(&layersCount, nullptr);

	std::vector<VkLayerProperties> instanceLayersVector(layersCount);
	vkEnumerateInstanceLayerProperties(&layersCount, instanceLayersVector.data());

	std::ostringstream stream;
	stream << "\nAll Supported Instance Layers: \n";
	for (uint32_t i = 0; i < layersCount; i++) {
		stream << "\t" << i + 1 << ". " << instanceLayersVector[i].layerName << "\n";
	}
	std::cout << stream.str();
}

void SenAbstractGLFW::showAllSupportedExtensionsEachUnderInstanceLayer()
{
	uint32_t layersCount = 0;
	vkEnumerateInstanceLayerProperties(&layersCount, nullptr);

	std::vector<VkLayerProperties> instanceLayersVector(layersCount);
	vkEnumerateInstanceLayerProperties(&layersCount, instanceLayersVector.data());

	std::ostringstream stream;
	stream << "\nAll Supported Instance Extensions under Layers: \n";
	for (uint32_t i = 0; i < layersCount; i++) {
		stream << "\t" << i + 1 << ". " << instanceLayersVector[i].description << "\t\t\t\t [\t" << instanceLayersVector[i].layerName << "\t]\n";

		uint32_t extensionsCount = 0;
		errorCheck(vkEnumerateInstanceExtensionProperties(instanceLayersVector[i].layerName, &extensionsCount, NULL), std::string("Extension Count under Layer Error!"));
		std::vector<VkExtensionProperties> extensionsPropVec(extensionsCount);
		errorCheck(vkEnumerateInstanceExtensionProperties(instanceLayersVector[i].layerName, &extensionsCount, extensionsPropVec.data()), std::string("Extension Data() under Layer Error!"));

		if (extensionsPropVec.size()) {
			for (int j = 0; j < extensionsPropVec.size(); j++) {
				stream << "\t\t\t|-- [ " << j + 1 << " ] . " << extensionsPropVec[j].extensionName << "\n";
			}
		}
		else {
			stream << "\t\t\t|-- [ 0 ] .  None of Instance Extensions \n";
		}
		stream << std::endl;
	}
	std::cout << stream.str();
}

void SenAbstractGLFW::showPhysicalDeviceSupportedLayersAndExtensions(const VkPhysicalDevice & gpuToCheck)
{
	/************************************************************************************************************/
	/******* Check PhysicalDevice Layers with each supported Extensions under Layer******************************/
	/************************************************************************************************************/
	std::ostringstream stream;
	stream << "\nAll Supported PysicalDevice Extensions: \n----------------------------------------------------\n";
	uint32_t gpuExtensionsCount = 0;
	errorCheck(vkEnumerateDeviceExtensionProperties(gpuToCheck, nullptr, &gpuExtensionsCount, nullptr), std::string("GPU Extension Count under Layer Error!"));
	std::vector<VkExtensionProperties> gpuExtensionsPropVec(gpuExtensionsCount);
	errorCheck(vkEnumerateDeviceExtensionProperties(gpuToCheck, nullptr, &gpuExtensionsCount, gpuExtensionsPropVec.data()), std::string("GPU Extension Data() under Layer Error!"));
	if (gpuExtensionsPropVec.size()) {
		for (int j = 0; j < gpuExtensionsPropVec.size(); j++) {
			stream << "\t\t\t    [ " << j + 1 << " ] . " << gpuExtensionsPropVec[j].extensionName << "\n";
		}
	}
	else {
		stream << "\t\t\t|-- [ 0 ] .  None of PhysicalDevice Extensions \n";
	}
	/************************************************************************************************************/
	/******* Check PhysicalDevice Layers with each supported Extensions under Layer******************************/
	/************************************************************************************************************/
	uint32_t gpuLayersCount = 0;
	vkEnumerateDeviceLayerProperties(gpuToCheck, &gpuLayersCount, nullptr);
	std::vector<VkLayerProperties> physicalDeviceLayersVector(gpuLayersCount);
	vkEnumerateDeviceLayerProperties(gpuToCheck, &gpuLayersCount, physicalDeviceLayersVector.data());

	stream << "\nSupported Extensions under PysicalDevice Supported Layers: \n----------------------------------------------------\n";
	for (uint32_t i = 0; i < gpuLayersCount; i++) {
		stream << "\t" << i + 1 << ". " << physicalDeviceLayersVector[i].description << "\t\t\t\t [\t" << physicalDeviceLayersVector[i].layerName << "\t]\n";

		uint32_t gpuExtensionsCount = 0;
		errorCheck(vkEnumerateDeviceExtensionProperties(gpuToCheck, physicalDeviceLayersVector[i].layerName, &gpuExtensionsCount, NULL), std::string("GPU Extension Count under Layer Error!"));
		std::vector<VkExtensionProperties> gpuExtensionsPropVec(gpuExtensionsCount);
		errorCheck(vkEnumerateDeviceExtensionProperties(gpuToCheck, physicalDeviceLayersVector[i].layerName, &gpuExtensionsCount, gpuExtensionsPropVec.data()), std::string("GPU Extension Data() under Layer Error!"));

		if (gpuExtensionsPropVec.size()) {
			for (int j = 0; j < gpuExtensionsPropVec.size(); j++) {
				stream << "\t\t\t|-- [ " << j + 1 << " ] . " << gpuExtensionsPropVec[j].extensionName << "\n";
			}
		}
		else {
			stream << "\t\t\t|-- [ 0 ] .  None of PhysicalDevice Extensions \n";
		}
		stream << "\n";
	}

	std::cout << stream.str();
}




//SenAbstractGLFW* currentInstance;
//
//extern "C" void _KeyDetection(GLFWwindow* widget, int key, int scancode, int action, int mode)
//{
//	currentInstance->_protectedKeyDetection(widget, key, scancode, action, mode);
//}
//
//void SenAbstractGLFW::keyboardRegister()
//{
//	::currentInstance = this;
//	::glfwSetKeyCallback(widgetGLFW, _KeyDetection);
//
//}
//
//void SenAbstractGLFW::keyDetection(GLFWwindow* widget, int key, int scancode, int action, int mode)
//{
//	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
//		glfwSetWindowShouldClose(widget, GL_TRUE);
//}
//
//
