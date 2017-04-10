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
	createSwapChainImageViews();
	createDepthStencilAttachment();

	createRenderPass();
	createGraphicsPipeline();

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
	/********** Getting Surface Capabilities first to support SwapChain. ********************************************************/
	/*********** Could not do this right after surface creation, because GPU had not been seleted at that time ******************/
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);

	// Do the check & assignment below because what we surface size we got may not equal to what we set
	// Make sure the size of swapchain match the size of surface
	if (surfaceCapabilities.currentExtent.width < UINT32_MAX) {
		widgetWidth = surfaceCapabilities.currentExtent.width;
		widgetHeight = surfaceCapabilities.currentExtent.height;
	}else {
		widgetWidth = (std::max)(surfaceCapabilities.minImageExtent.width, (std::min)(surfaceCapabilities.maxImageExtent.width, static_cast<uint32_t>(widgetWidth)));
		widgetHeight = (std::max)(surfaceCapabilities.minImageExtent.height, (std::min)(surfaceCapabilities.maxImageExtent.height, static_cast<uint32_t>(widgetHeight)));
	}

	/****************************************************************************************************************************/
	/**************************** Reserve swapchain minImageCount ***************************************************************/
	/****************************************************************************************************************************/
	// For best performance, possibly at the price of some latency, the minImageCount should be set to at least 3 if supported;
	// maxImageCount can actually be zero in which case the amount of swapchain images do not have an upper limit other than available memory. 
	// It's also possible that the swapchain image amount is locked to a certain value on certain systems. The code below takes into consideration both of these possibilities.
	if (swapchainImagesCount < surfaceCapabilities.minImageCount + 1) swapchainImagesCount = surfaceCapabilities.minImageCount + 1;
	if (surfaceCapabilities.maxImageCount > 0) {
		if (swapchainImagesCount > surfaceCapabilities.maxImageCount) swapchainImagesCount = surfaceCapabilities.maxImageCount;
	}

	/****************************************************************************************************************************/
	/********** Reserve swapchain imageFormat and imageColorSpace ***************************************************************/
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
	}
	else {
		surfaceFormat = surfaceFormatVector[0];
	}

	/****************************************************************************************************************************/
	/**********                Reserve presentMode                ***************************************************************/
	/****************************************************************************************************************************/
	VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR; // VK_PRESENT_MODE_FIFO_KHR is always available.
	uint32_t presentModeCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
	std::vector<VkPresentModeKHR> presentModeVector(presentModeCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModeVector.data());
	for (auto m : presentModeVector) {
		// VK_PRESENT_MODE_MAILBOX_KHR is good for gaming, but can only get full advantage of MailBox PresentMode with more than 2 buffers,
		// which means triple-buffering
		if (m == VK_PRESENT_MODE_MAILBOX_KHR) {
			presentMode = m;
			break;
		}
	}

	/****************************************************************************************************************************/
	/**********         Populate swapchainCreateInfo              ***************************************************************/
	/****************************************************************************************************************************/
	VkSwapchainCreateInfoKHR swapchainCreateInfo{};
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.surface = surface;
	swapchainCreateInfo.minImageCount = swapchainImagesCount; // This is only to set the min value, instead of the actual imageCount after swapchain creation
	swapchainCreateInfo.imageFormat = surfaceFormat.format;
	swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
	swapchainCreateInfo.imageExtent.width = widgetWidth;
	swapchainCreateInfo.imageExtent.height = widgetHeight;
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	// Attention, figure out if the swapchain image will be shared by multiple Queues of different QueueFamilies !
	uint32_t queueFamilyIndicesArray[] = { static_cast<uint32_t>(graphicsQueueFamilyIndex), static_cast<uint32_t>(presentQueueFamilyIndex) };
	if (graphicsQueueFamilyIndex != presentQueueFamilyIndex) {
		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT; // shared by different QueueFamily with different QueueFamilyIndex 
		swapchainCreateInfo.queueFamilyIndexCount = 2;
		swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndicesArray;
	}
	else {
		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;		// share between QueueFamilies or not
		swapchainCreateInfo.queueFamilyIndexCount = 0;// no QueueFamily share
		swapchainCreateInfo.pQueueFamilyIndices = nullptr;
	}

	swapchainCreateInfo.preTransform = surfaceCapabilities.currentTransform; // Rotate of Mirror before presentation (VkSurfaceTransformFlagBitsKHR)
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;  // 
	swapchainCreateInfo.presentMode = presentMode;
	swapchainCreateInfo.clipped = VK_TRUE; // Typically always set this true, such that Vulkan never render the invisible (out of visible range) image 
	swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE; // resize window

	errorCheck(
		vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &swapChain),
		std::string("Fail to Create SwapChain !")
	);

	// Get actual amount/count of swapchain images
	vkGetSwapchainImagesKHR(device, swapChain, &swapchainImagesCount, nullptr);

	swapchainImagesVector.resize(swapchainImagesCount);
	swapchainImageViewsVector.resize(swapchainImagesCount);
	// Get swapChainImages, instead of asking for real count.
	errorCheck(
		vkGetSwapchainImagesKHR(device, swapChain, &swapchainImagesCount, swapchainImagesVector.data()),
		std::string("Failed to get SwapChain Images")
	);
}

void SenAbstractGLFW::createSwapChainImageViews() {

	for (uint32_t i = 0; i < swapchainImagesCount; ++i) {
		VkImageViewCreateInfo swapchainImageViewCreateInfo{};
		swapchainImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		swapchainImageViewCreateInfo.image = swapchainImagesVector[i];
		swapchainImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; // handling 2D image
		swapchainImageViewCreateInfo.format = surfaceFormat.format;
		swapchainImageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		swapchainImageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		swapchainImageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		swapchainImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		swapchainImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // color/depth/stencil/metadata
		swapchainImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		swapchainImageViewCreateInfo.subresourceRange.levelCount = 1; // amount of mipmaps
		swapchainImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		swapchainImageViewCreateInfo.subresourceRange.layerCount = 1; // if larger than 1, .viewType needs to be array

		errorCheck(
			vkCreateImageView(device, &swapchainImageViewCreateInfo, nullptr, &swapchainImageViewsVector[i]),
			std::string("Failed to create SwapChan ImageViews !!")
		);
	}
}

void SenAbstractGLFW::createDepthStencilAttachment()
{
	/********************************************************************************************************************/
	/******************************  Check Image Format *****************************************************************/
	std::vector<VkFormat> tryFormatsVector{
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		VK_FORMAT_D24_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM_S8_UINT,
		VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_D16_UNORM
	};
	for (auto f : tryFormatsVector) {
		VkFormatProperties formatProperties{};
		vkGetPhysicalDeviceFormatProperties(physicalDevice, f, &formatProperties);
		if (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
			depthStencilFormat = f;
			break;
		}
	}
	if (depthStencilFormat == VK_FORMAT_UNDEFINED) {
		throw std::runtime_error("Depth stencil format not selected.");
		std::exit(-1);
	}
	if ((depthStencilFormat == VK_FORMAT_D32_SFLOAT_S8_UINT) ||
		(depthStencilFormat == VK_FORMAT_D24_UNORM_S8_UINT) ||
		(depthStencilFormat == VK_FORMAT_D16_UNORM_S8_UINT) ||
		(depthStencilFormat == VK_FORMAT_S8_UINT)) {
		stencilAvailable = true;
	}
	else std::cout << "The seleted depthStencilFormat is not in the stencil list !!!!! \n \t Take a check !!!!\n";

	/******************************************************************************************************************************************************/
	/******************************  Create depthStencil Image ********************************************************************************************/
	VkImageCreateInfo depthStencilImageCreateInfo{};
	depthStencilImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	depthStencilImageCreateInfo.flags = 0;
	depthStencilImageCreateInfo.imageType = VK_IMAGE_TYPE_2D; // 2D image
	depthStencilImageCreateInfo.format = depthStencilFormat;
	depthStencilImageCreateInfo.extent.width = widgetWidth;
	depthStencilImageCreateInfo.extent.height = widgetHeight;
	depthStencilImageCreateInfo.extent.depth = 1; // 2D image, has to be 1
	depthStencilImageCreateInfo.mipLevels = 1;
	depthStencilImageCreateInfo.arrayLayers = 1;
	depthStencilImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT; // multi-sampling
	depthStencilImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	depthStencilImageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;// for depth stencil image
	depthStencilImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // no share with anther QueueFamily
	depthStencilImageCreateInfo.queueFamilyIndexCount = 0;
	depthStencilImageCreateInfo.pQueueFamilyIndices = nullptr;
	depthStencilImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // will overwrite this with a command later

	vkCreateImage(device, &depthStencilImageCreateInfo, nullptr, &depthStencilImage);

	/******************************************************************************************************************************************************/
	/***************************  Allocate & Bind memory for depthStencil Image using the created handle *********************************************************/
	VkMemoryRequirements imageMemoryRequirements{};
	vkGetImageMemoryRequirements(device, depthStencilImage, &imageMemoryRequirements);

	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &physicalDeviceMemoryProperties);

	uint32_t memoryTypeIndex = findPhysicalDeviceMemoryPropertyIndex(
		physicalDeviceMemoryProperties,
		imageMemoryRequirements,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT  // Set the resource to reside on GPU itself
	);

	VkMemoryAllocateInfo depthStencilImageMemoryAllocateInfo{};
	depthStencilImageMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	depthStencilImageMemoryAllocateInfo.allocationSize = imageMemoryRequirements.size;
	depthStencilImageMemoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;

	vkAllocateMemory(device, &depthStencilImageMemoryAllocateInfo, nullptr, &depthStencilImageDeviceMemory);
	vkBindImageMemory(device, depthStencilImage, depthStencilImageDeviceMemory, 0);

	/******************************************************************************************************************************************************/
	/******************************  Create depthStencil Image View ***************************************************************************************/
	VkImageViewCreateInfo depthStencilImageViewCreateInfo{};
	depthStencilImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	depthStencilImageViewCreateInfo.image = depthStencilImage;
	depthStencilImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	depthStencilImageViewCreateInfo.format = depthStencilFormat;
	depthStencilImageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	depthStencilImageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	depthStencilImageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	depthStencilImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	depthStencilImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | (stencilAvailable ? VK_IMAGE_ASPECT_STENCIL_BIT : 0);
	depthStencilImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	depthStencilImageViewCreateInfo.subresourceRange.levelCount = 1;
	depthStencilImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	depthStencilImageViewCreateInfo.subresourceRange.layerCount = 1;

	vkCreateImageView(device, &depthStencilImageViewCreateInfo, nullptr, &depthStencilImageView);
}

void SenAbstractGLFW::createRenderPass() {
	VkAttachmentDescription colorAttachmentDescription{};
	colorAttachmentDescription.format			= surfaceFormat.format;// swapChainImageFormat;
	colorAttachmentDescription.samples			= VK_SAMPLE_COUNT_1_BIT;
	colorAttachmentDescription.loadOp			= VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachmentDescription.storeOp			= VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachmentDescription.stencilLoadOp	= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentDescription.stencilStoreOp	= VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachmentDescription.initialLayout	= VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachmentDescription.finalLayout		= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentReference{};
	colorAttachmentReference.attachment			= 0;
	colorAttachmentReference.layout				= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpassDescription{};
	subpassDescription.pipelineBindPoint		= VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.colorAttachmentCount		= 1;
	subpassDescription.pColorAttachments		= &colorAttachmentReference;

	VkSubpassDependency SubpassDependency{};
	SubpassDependency.srcSubpass				= VK_SUBPASS_EXTERNAL;
	SubpassDependency.dstSubpass				= 0;
	SubpassDependency.srcStageMask				= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	SubpassDependency.srcAccessMask				= 0;
	SubpassDependency.dstStageMask				= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	SubpassDependency.dstAccessMask				= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT 
												| VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassCreateInfo{};
	renderPassCreateInfo.sType					= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount		= 1;
	renderPassCreateInfo.pAttachments			= &colorAttachmentDescription;
	renderPassCreateInfo.subpassCount			= 1;
	renderPassCreateInfo.pSubpasses				= &subpassDescription;
	renderPassCreateInfo.dependencyCount		= 1;
	renderPassCreateInfo.pDependencies			= &SubpassDependency;

	errorCheck(
		vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &renderPass),
		std::string("Failed to create render pass !!")
	);
}

void SenAbstractGLFW::createShaderModule(const VkDevice& device, const std::vector<char>& SPIRV_Vector, VkShaderModule & targetShaderModule)
{
	VkShaderModuleCreateInfo shaderModuleCreateInfo{};
	shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderModuleCreateInfo.codeSize = SPIRV_Vector.size();
	shaderModuleCreateInfo.pCode = (uint32_t*)SPIRV_Vector.data();

	errorCheck(
		vkCreateShaderModule(device, &shaderModuleCreateInfo, nullptr, &targetShaderModule),
		std::string("Failed to create the shader module !!")
	);
}

void SenAbstractGLFW::createGraphicsPipeline() {
	VkShaderModule vertShaderModule;
	VkShaderModule fragShaderModule;

	//std::vector<char> vertShaderSPIRV_Vector = readFileBinaryStream("SenVulkanTutorial/Shaders/Triangle.vert");
	//std::vector<char> fragShaderSPIRV_Vector = readFileBinaryStream("SenVulkanTutorial/Shaders/Triangle.frag");
	//createShaderModule(device, vertShaderSPIRV_Vector, vertShaderModule);
	//createShaderModule(device, fragShaderSPIRV_Vector, fragShaderModule);

	createShaderModule(device, readFileBinaryStream("SenVulkanTutorial/Shaders/triangleVert.spv"), vertShaderModule);
	createShaderModule(device, readFileBinaryStream("SenVulkanTutorial/Shaders/triangleFrag.spv"), fragShaderModule);

	VkPipelineShaderStageCreateInfo vertPipelineShaderStageCreateInfo{};
	vertPipelineShaderStageCreateInfo.sType		= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertPipelineShaderStageCreateInfo.stage		= VK_SHADER_STAGE_VERTEX_BIT;
	vertPipelineShaderStageCreateInfo.module	= vertShaderModule;
	vertPipelineShaderStageCreateInfo.pName		= "main";

	VkPipelineShaderStageCreateInfo fragPipelineShaderStageCreateInfo{};
	fragPipelineShaderStageCreateInfo.sType		= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragPipelineShaderStageCreateInfo.stage		= VK_SHADER_STAGE_FRAGMENT_BIT;
	fragPipelineShaderStageCreateInfo.module	= fragShaderModule;
	fragPipelineShaderStageCreateInfo.pName		= "main";

	VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfosArray[] = { vertPipelineShaderStageCreateInfo, fragPipelineShaderStageCreateInfo };

	VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo{};
	pipelineVertexInputStateCreateInfo.sType							= VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	pipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount	= 0;
	pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount	= 0;

	VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo = {};
	pipelineInputAssemblyStateCreateInfo.sType							= VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	pipelineInputAssemblyStateCreateInfo.topology						= VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	pipelineInputAssemblyStateCreateInfo.primitiveRestartEnable			= VK_FALSE;

	VkViewport viewport{};
	viewport.x			= 0.0f;
	viewport.y			= 0.0f;
	viewport.width		= static_cast<float>(widgetWidth);
	viewport.height		= static_cast<float>(widgetHeight);
	viewport.minDepth	= 0.0f;
	viewport.maxDepth	= 1.0f;

	VkRect2D scissorRect2D{};
	scissorRect2D.offset			= { 0, 0 };
	scissorRect2D.extent.width	= static_cast<uint32_t>(widgetWidth);
	scissorRect2D.extent.height	= static_cast<uint32_t>(widgetHeight);

	VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo{};
	pipelineViewportStateCreateInfo.sType				= VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	pipelineViewportStateCreateInfo.viewportCount		= 1;
	pipelineViewportStateCreateInfo.pViewports			= &viewport;
	pipelineViewportStateCreateInfo.scissorCount		= 1;
	pipelineViewportStateCreateInfo.pScissors			= &scissorRect2D;

	VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo{};
	pipelineRasterizationStateCreateInfo.sType						= VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	pipelineRasterizationStateCreateInfo.depthClampEnable			= VK_FALSE;
	pipelineRasterizationStateCreateInfo.rasterizerDiscardEnable	= VK_FALSE;
	pipelineRasterizationStateCreateInfo.polygonMode				= VK_POLYGON_MODE_FILL;
	pipelineRasterizationStateCreateInfo.lineWidth					= 1.0f;
	pipelineRasterizationStateCreateInfo.cullMode					= VK_CULL_MODE_BACK_BIT;
	pipelineRasterizationStateCreateInfo.frontFace					= VK_FRONT_FACE_CLOCKWISE;
	pipelineRasterizationStateCreateInfo.depthBiasEnable			= VK_FALSE;

	VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo{};
	pipelineMultisampleStateCreateInfo.sType						= VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	pipelineMultisampleStateCreateInfo.sampleShadingEnable			= VK_FALSE;
	pipelineMultisampleStateCreateInfo.rasterizationSamples			= VK_SAMPLE_COUNT_1_BIT;

	VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState{};
	pipelineColorBlendAttachmentState.colorWriteMask				= VK_COLOR_COMPONENT_R_BIT 
																		| VK_COLOR_COMPONENT_G_BIT 
																		| VK_COLOR_COMPONENT_B_BIT 
																		| VK_COLOR_COMPONENT_A_BIT;
	pipelineColorBlendAttachmentState.blendEnable					= VK_FALSE;

	VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo{};
	pipelineColorBlendStateCreateInfo.sType							= VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	pipelineColorBlendStateCreateInfo.logicOpEnable					= VK_FALSE;
	pipelineColorBlendStateCreateInfo.logicOp						= VK_LOGIC_OP_COPY;
	pipelineColorBlendStateCreateInfo.attachmentCount				= 1;
	pipelineColorBlendStateCreateInfo.pAttachments					= &pipelineColorBlendAttachmentState;
	pipelineColorBlendStateCreateInfo.blendConstants[0]				= 0.0f;
	pipelineColorBlendStateCreateInfo.blendConstants[1]				= 0.0f;
	pipelineColorBlendStateCreateInfo.blendConstants[2]				= 0.0f;
	pipelineColorBlendStateCreateInfo.blendConstants[3]				= 0.0f;

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
	pipelineLayoutCreateInfo.sType									= VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount							= 0;
	pipelineLayoutCreateInfo.pushConstantRangeCount					= 0;

	errorCheck(
		vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &graphicsPipelineLayout),
		std::string("Failed to to create pipeline layout !!!")
	);

	VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo{};
	graphicsPipelineCreateInfo.sType				= VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	graphicsPipelineCreateInfo.stageCount			= 2;
	graphicsPipelineCreateInfo.pStages				= pipelineShaderStageCreateInfosArray;
	graphicsPipelineCreateInfo.pVertexInputState	= &pipelineVertexInputStateCreateInfo;
	graphicsPipelineCreateInfo.pInputAssemblyState	= &pipelineInputAssemblyStateCreateInfo;
	graphicsPipelineCreateInfo.pViewportState		= &pipelineViewportStateCreateInfo;
	graphicsPipelineCreateInfo.pRasterizationState	= &pipelineRasterizationStateCreateInfo;
	graphicsPipelineCreateInfo.pMultisampleState	= &pipelineMultisampleStateCreateInfo;
	graphicsPipelineCreateInfo.pColorBlendState		= &pipelineColorBlendStateCreateInfo;
	graphicsPipelineCreateInfo.layout				= graphicsPipelineLayout;
	graphicsPipelineCreateInfo.renderPass			= renderPass;
	graphicsPipelineCreateInfo.subpass				= 0;
	graphicsPipelineCreateInfo.basePipelineHandle	= VK_NULL_HANDLE;

	errorCheck(
		vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, &graphicsPipeline),
		std::string("Failed to create graphics pipeline !!!")
	);

	vkDestroyShaderModule(device, vertShaderModule, nullptr);
	vkDestroyShaderModule(device, fragShaderModule, nullptr);
}

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
//		VkRenderPassBeginInfo renderPassCreateInfo = {};
//		renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
//		renderPassCreateInfo.renderPass = renderPass;
//		renderPassCreateInfo.framebuffer = swapChainFramebuffers[i];
//		renderPassCreateInfo.renderArea.offset = { 0, 0 };
//		renderPassCreateInfo.renderArea.extent = swapChainExtent;
//
//		VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
//		renderPassCreateInfo.clearValueCount = 1;
//		renderPassCreateInfo.pClearValues = &clearColor;
//
//		vkCmdBeginRenderPass(commandBuffers[i], &renderPassCreateInfo, VK_SUBPASS_CONTENTS_INLINE);
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
	std::vector<VkQueueFamilyProperties> gpuQueueFamiliesPropertiesVector(gpuQueueFamiliesCount);
	vkGetPhysicalDeviceQueueFamilyProperties(gpuToCheck, &gpuQueueFamiliesCount, gpuQueueFamiliesPropertiesVector.data());

	for (uint32_t i = 0; i < gpuQueueFamiliesCount; i++) {
		if (graphicsQueueIndex < 0 && gpuQueueFamiliesPropertiesVector[i].queueCount > 0
			&& gpuQueueFamiliesPropertiesVector[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			graphicsQueueIndex = i;
		}

		if (presentQueueIndex < 0 && gpuQueueFamiliesPropertiesVector[i].queueCount > 0) {
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
	std::vector<VkQueueFamilyProperties> gpuQueueFamiliesPropertiesVector(gpuQueueFamiliesCount);
	vkGetPhysicalDeviceQueueFamilyProperties(gpuToCheck, &gpuQueueFamiliesCount, gpuQueueFamiliesPropertiesVector.data());

	//	1. Get the number of Queues supported by the Physical device
	//	2. Get the properties each Queue type or Queue Family
	//			There could be 4 Queue type or Queue families supported by physical device - 
	//			Graphics Queue	- VK_QUEUE_GRAPHICS_BIT 
	//			Compute Queue	- VK_QUEUE_COMPUTE_BIT
	//			DMA				- VK_QUEUE_TRANSFER_BIT
	//			Sparse memory	- VK_QUEUE_SPARSE_BINDING_BIT
	//	3. Get the index ID for the required Queue family, this ID will act like a handle index to queue.

	for (uint32_t i = 0; i < gpuQueueFamiliesCount; i++) {
		if (graphicsQueueIndex < 0 && gpuQueueFamiliesPropertiesVector[i].queueCount > 0
			&& gpuQueueFamiliesPropertiesVector[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			graphicsQueueIndex = i;
		}

		if (presentQueueIndex < 0 && gpuQueueFamiliesPropertiesVector[i].queueCount > 0) {
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
	/*******************************************************************************************************************************/
	/*** Attension! Multiple Queues with same QueueFamilyIndex can only be created using one deviceQueueCreateInfo *****************/
	/*************  Different QueueFamilyIndex's Queues need to be created using a vector of DeviceQueueCreateInfos ****************/
	std::set<int> uniqueQueueFamilyIndicesSet = { graphicsQueueFamilyIndex, presentQueueFamilyIndex };

	float queuePriority = 1.0f;
	for (int uniqueQueueFamilyIndex : uniqueQueueFamilyIndicesSet) {
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = uniqueQueueFamilyIndex;
		queueCreateInfo.queueCount = 1; // This queues count number is under one uniqueQueueFamily, with uniqueQueueFamilyIndex
		queueCreateInfo.pQueuePriorities = &queuePriority;

		deviceQueuesCreateInfosVector.push_back(queueCreateInfo);
	}

	VkDeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(deviceQueuesCreateInfosVector.size());
	deviceCreateInfo.pQueueCreateInfos = deviceQueuesCreateInfosVector.data();

	if (layersEnabled) {
		deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(debugDeviceLayersVector.size());   				// depricated
		deviceCreateInfo.ppEnabledLayerNames = debugDeviceLayersVector.data();				// depricated
	}
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(debugDeviceExtensionsVector.size());
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
	/************************************************************************************************************/
	/*********************           Destroy Pipeline, PipelineLayout, and RenderPass         *******************/
	/************************************************************************************************************/
	if (VK_NULL_HANDLE != graphicsPipeline) {
		vkDestroyPipeline(device, graphicsPipeline, nullptr);
		vkDestroyPipelineLayout(device, graphicsPipelineLayout, nullptr);
		vkDestroyRenderPass(device, renderPass, nullptr);

		graphicsPipeline		= VK_NULL_HANDLE;
		graphicsPipelineLayout	= VK_NULL_HANDLE;
		renderPass				= VK_NULL_HANDLE;
	}
	
	/************************************************************************************************************/
	/*********************           Destroy depthStencil                ****************************************/
	/************************************************************************************************************/
	if (VK_NULL_HANDLE != depthStencilImage) {
		vkFreeMemory(device, depthStencilImageDeviceMemory, nullptr);
		vkDestroyImageView(device, depthStencilImageView, nullptr);
		vkDestroyImage(device, depthStencilImage, nullptr);

		depthStencilImage = VK_NULL_HANDLE;
		depthStencilImageDeviceMemory = VK_NULL_HANDLE;
		depthStencilImageView = VK_NULL_HANDLE;
	}
	/************************************************************************************************************/
	/*****  SwapChain is a child of Logical Device, must be destroyed before Logical Device  ********************/
	/****************   A surface must outlive any swapchains targeting it    ***********************************/
	if (VK_NULL_HANDLE != swapChain) {
		// swapChainImages will be handled by the destroy of swapchain
		// But swapchainImageViews need to be dstroyed first, before the destroy of swapchain.
		for (auto swapchainImageView : swapchainImageViewsVector) {
			vkDestroyImageView(device, swapchainImageView, nullptr);
		}
		swapchainImageViewsVector.clear();

		vkDestroySwapchainKHR(device, swapChain, nullptr);
		swapChain = VK_NULL_HANDLE;
		swapchainImagesVector.clear();

		// The memory of swapChain images is not managed by programmer (No allocation, nor free)
		// It may not be freed until the window is destroyed, or another swapchain is created for the window.
	}

	/************************************************************************************************************/
	/*********************           Destroy logical device                **************************************/
	/************************************************************************************************************/
	if (VK_NULL_HANDLE != device) {
		vkDestroyDevice(device, VK_NULL_HANDLE);

		// Device queues are implicitly cleaned up when the device is destroyed
		if (VK_NULL_HANDLE != graphicsQueue) { graphicsQueue = VK_NULL_HANDLE; }
		if (VK_NULL_HANDLE != presentQueue) { presentQueue = VK_NULL_HANDLE; }

		device = VK_NULL_HANDLE;
	}

	/************************************************************************************************************/
	/*********************  Must destroy debugReportCallback before destroy instance   **************************/
	/************************************************************************************************************/
	if (layersEnabled) {
		if (VK_NULL_HANDLE != debugReportCallback) {
			fetch_vkDestroyDebugReportCallbackEXT(instance, debugReportCallback, VK_NULL_HANDLE);
			debugReportCallback = VK_NULL_HANDLE;
		}
	}

	/************************************************************************************************************/
	/*************  Destroy window surface, Note that this is a native Vulkan API function  *********************/
	/*****  Surface survives longer than device than swapchain, and depends only on Instance, or platform  ******/
	/************************************************************************************************************/
	if (VK_NULL_HANDLE != surface) {
		vkDestroySurfaceKHR(instance, surface, nullptr);	//  surface was created with GLFW function
		surface = VK_NULL_HANDLE;
	}

	/************************************************************************************************************/
	/*********************           Destroy Instance                ********************************************/
	/************************************************************************************************************/
	if (VK_NULL_HANDLE != instance) {
		vkDestroyInstance(instance, VK_NULL_HANDLE); 	instance = VK_NULL_HANDLE;
	}
}

uint32_t SenAbstractGLFW::findPhysicalDeviceMemoryPropertyIndex(
	const VkPhysicalDeviceMemoryProperties& gpuMemoryProperties,
	const VkMemoryRequirements& memoryRequirements,
	const VkMemoryPropertyFlags& requiredMemoryPropertyFlags)
{
	for (uint32_t index = 0; index < gpuMemoryProperties.memoryTypeCount; ++index) {
		if (memoryRequirements.memoryTypeBits & (1 << index)) {
			if ((gpuMemoryProperties.memoryTypes[index].propertyFlags & requiredMemoryPropertyFlags) == requiredMemoryPropertyFlags) {
				return index;
			}
		}
	}
	throw std::runtime_error("Couldn't find proper GPU memory Property Index.");
	return UINT32_MAX;
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

std::vector<char> SenAbstractGLFW::readFileBinaryStream(const std::string& filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file!");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
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
