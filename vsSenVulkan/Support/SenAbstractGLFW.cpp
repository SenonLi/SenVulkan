#include "SenAbstractGLFW.h"

// Since stb_image.h header file contains the implementation of functions, only one class source file could include it to make new implementation
// all stb_image realated functions have to be implemented in this class
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

SenAbstractGLFW::SenAbstractGLFW()
{
	std::cout << "\nConstructor: SenAbstractGLFW()\n";
	//showAllSupportedInstanceExtensions(); // Not Useful Functions
	//showAllSupportedInstanceLayers(); // Not Useful Functions
	//showAllSupportedExtensionsEachUnderInstanceLayer(); // Not Useful Functions

	widgetWidth = DEFAULT_widgetWidth;
	widgetHeight = DEFAULT_widgetHeight;

	strWindowName = "Sen GLFW Vulkan Application";
}

SenAbstractGLFW::~SenAbstractGLFW()
{
	finalizeAbstractGLFW();
	OutputDebugString("\n\t ~SenAbstractGLFW()\n");
}

const std::vector<VkFormat> SenAbstractGLFW::depthStencilSupportCheckFormatsVector = {
	VK_FORMAT_D16_UNORM,
	VK_FORMAT_D32_SFLOAT,
	VK_FORMAT_X8_D24_UNORM_PACK32,
	VK_FORMAT_S8_UINT,
	VK_FORMAT_D16_UNORM_S8_UINT,
	VK_FORMAT_D24_UNORM_S8_UINT,
	VK_FORMAT_D32_SFLOAT_S8_UINT
};

void SenAbstractGLFW::createTextureSampler(const VkDevice& logicalDevice, VkSampler& textureSamplerToCreate) {
	VkSamplerCreateInfo textureSamplerCreateInfo{};
	textureSamplerCreateInfo.sType						= VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	textureSamplerCreateInfo.magFilter					= VK_FILTER_LINEAR; // oversampling
	textureSamplerCreateInfo.minFilter					= VK_FILTER_LINEAR;	// undersampling
	textureSamplerCreateInfo.mipmapMode					= VK_SAMPLER_MIPMAP_MODE_LINEAR;
	textureSamplerCreateInfo.addressModeU				= VK_SAMPLER_ADDRESS_MODE_REPEAT;
	textureSamplerCreateInfo.addressModeV				= VK_SAMPLER_ADDRESS_MODE_REPEAT;
	textureSamplerCreateInfo.addressModeW				= VK_SAMPLER_ADDRESS_MODE_REPEAT;
	textureSamplerCreateInfo.anisotropyEnable			= VK_TRUE;
	textureSamplerCreateInfo.maxAnisotropy				= 16.0;
	textureSamplerCreateInfo.compareEnable				= VK_FALSE; // for shadow maps (percentage-closer filtering)
	textureSamplerCreateInfo.compareOp					= VK_COMPARE_OP_ALWAYS;
	textureSamplerCreateInfo.borderColor				= VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	textureSamplerCreateInfo.unnormalizedCoordinates	= VK_FALSE;

	SenAbstractGLFW::errorCheck(
		vkCreateSampler(logicalDevice, &textureSamplerCreateInfo, nullptr, &textureSamplerToCreate),
		std::string("Failed to create texture sampler !!!")
	);
}

void SenAbstractGLFW::createResourceImage(const VkDevice& logicalDevice,const uint32_t& imageWidth, const uint32_t& imageHeight
	,const VkImageType& imageType, const VkFormat& imageFormat, const VkImageTiling& imageTiling, const VkImageUsageFlags& imageUsageFlags
	,VkImage& imageToCreate, VkDeviceMemory& imageDeviceMemoryToAllocate, const VkMemoryPropertyFlags& requiredMemoryPropertyFlags
	,const VkSharingMode& imageSharingMode, const VkPhysicalDeviceMemoryProperties& gpuMemoryProperties)
{
	/***********************************************************************************************************************************************/
	/*************    VK_IMAGE_TILING_LINEAR  have further restrictions on their limits and capabilities    ****************************************/
	if (imageTiling == VK_IMAGE_TILING_LINEAR) {
		for (auto posibleDepthStencilFormat : SenAbstractGLFW::depthStencilSupportCheckFormatsVector) {
			if (imageFormat == posibleDepthStencilFormat) {
				throw std::runtime_error("Illegal imageFormat to create VK_IMAGE_TILING_LINEAR ResourceImage  !!!");
				break;
			}
		}
		if (imageType != VK_IMAGE_TYPE_2D) 	throw std::runtime_error("Illegal imageType to create VK_IMAGE_TILING_LINEAR ResourceImage  !!!");
		if (~(VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT) & imageUsageFlags) 	
			throw std::runtime_error("Illegal imageUsageFlags to create VK_IMAGE_TILING_LINEAR ResourceImage  !!!");
	}
	/***********************************************************************************************************************************************/
	VkImageCreateInfo imageCreateInfo{};
	imageCreateInfo.sType			= VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.imageType		= imageType;
	imageCreateInfo.extent.width	= imageWidth;
	imageCreateInfo.extent.height	= imageHeight;
	// The extent field specifies the dimensions of the image, basically how many texels there are on each axis;
	// That's why depth must be 1 instead of 0 while VK_IMAGE_TYPE_2D
	imageCreateInfo.extent.depth	= 1; // Need to fix this if create imageType != VK_IMAGE_TYPE_2D
	imageCreateInfo.mipLevels		= 1; // Spec: must be greater than 0; setup later based on function of imageWidth/imageHeight
	imageCreateInfo.arrayLayers		= 1; // Usually =1 if not working as ImageArray; Spec: must be greater than 0; have to be 1 if LINEAR format
	imageCreateInfo.format			= imageFormat;
	imageCreateInfo.tiling			= imageTiling;
	// An initially undefined layout is for images used as attachments (color/depth) that will probably be cleared by a render pass before use;
	// If you want to fill it with data, like a texture, then you should use the preinitialized layout.
	imageCreateInfo.initialLayout	= VK_IMAGE_LAYOUT_PREINITIALIZED;
	imageCreateInfo.usage			= imageUsageFlags;
	imageCreateInfo.samples			= VK_SAMPLE_COUNT_1_BIT;	// Need to fix this if using multisampling
	imageCreateInfo.sharingMode		= imageSharingMode;			// When being attachments and multiple QueueFamilies require

	SenAbstractGLFW::errorCheck(
		vkCreateImage(logicalDevice, &imageCreateInfo, nullptr, &imageToCreate),
		std::string("Failed to create resource image !!!")
	);
	/***********************************************************************************************************************************************/
	VkMemoryRequirements imageMemoryRequirements{};
	vkGetImageMemoryRequirements(logicalDevice, imageToCreate, &imageMemoryRequirements);

	VkMemoryAllocateInfo imageMemoryAllocateInfo{};
	imageMemoryAllocateInfo.sType			= VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	imageMemoryAllocateInfo.allocationSize	= imageMemoryRequirements.size;
	imageMemoryAllocateInfo.memoryTypeIndex
		= SenAbstractGLFW::findPhysicalDeviceMemoryPropertyIndex(gpuMemoryProperties, imageMemoryRequirements, requiredMemoryPropertyFlags);

	SenAbstractGLFW::errorCheck(
		vkAllocateMemory(logicalDevice, &imageMemoryAllocateInfo, nullptr, &imageDeviceMemoryToAllocate),
		std::string("Failed to allocate reource image memory !!!")
	);

	vkBindImageMemory(logicalDevice, imageToCreate, imageDeviceMemoryToAllocate, 0);
}


void SenAbstractGLFW::transitionResourceImageLayout(const VkImage& imageToTransitionLayout, const VkImageSubresourceRange& imageSubresourceRangeToTransition
	,const VkImageLayout& oldImageLayout, const VkImageLayout& newImageLayout, const VkFormat& imageFormat
	,const VkDevice& logicalDevice ,const VkCommandPool& transitionImageLayoutCommandPool  ,const VkQueue& imageMemoryTransferQueue) {
	
	if (newImageLayout == VK_IMAGE_LAYOUT_UNDEFINED || newImageLayout == VK_IMAGE_LAYOUT_PREINITIALIZED)
		throw std::runtime_error(" The newImageLayout must not be VK_IMAGE_LAYOUT_UNDEFINED or VK_IMAGE_LAYOUT_PREINITIALIZED  !!!");

	// Transitions can happen with an image memory barrier, included as part of a vkCmdPipelineBarrier;
	//								or a vkCmdWaitEvents command buffer command;
	//								or as part of a subpass dependency within a render pass(see VkSubpassDependency
	//										, like transitions between swapchain colorImage for framebuffer paiting and presentation);
	VkImageMemoryBarrier imageMemoryBarrier{};
	imageMemoryBarrier.sType							= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.oldLayout						= oldImageLayout; // current layout of the image subresource or else be VK_IMAGE_LAYOUT_UNDEFINED
	imageMemoryBarrier.newLayout						= newImageLayout; // must not be VK_IMAGE_LAYOUT_UNDEFINED or VK_IMAGE_LAYOUT_PREINITIALIZED.
	imageMemoryBarrier.srcQueueFamilyIndex				= VK_QUEUE_FAMILY_IGNORED; // must be VK_QUEUE_FAMILY_IGNORED instead of 0
	imageMemoryBarrier.dstQueueFamilyIndex				= VK_QUEUE_FAMILY_IGNORED; // must be VK_QUEUE_FAMILY_IGNORED instead of 0
	imageMemoryBarrier.image							= imageToTransitionLayout;
	imageMemoryBarrier.subresourceRange					= imageSubresourceRangeToTransition;

	if (oldImageLayout == VK_IMAGE_LAYOUT_PREINITIALIZED && newImageLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;	// the first access scope includes command buffer submission (HOST_WRITE)
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	}else if (oldImageLayout == VK_IMAGE_LAYOUT_PREINITIALIZED && newImageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;	// the first access scope includes command buffer submission (HOST_WRITE)
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	}else if (oldImageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newImageLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	}else {
		throw std::invalid_argument("unsupported layout transition!");
	}

	VkCommandBuffer transitionImageLayoutCommandBuffer = VK_NULL_HANDLE;
	SenAbstractGLFW::beginSingleTimeCommandBuffer(transitionImageLayoutCommandPool, logicalDevice, transitionImageLayoutCommandBuffer);
	vkCmdPipelineBarrier(
		transitionImageLayoutCommandBuffer,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,		// srcStageMask
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,		// dstStageMask
		0,						// dependencyFlags, framebuffer-global instead of framebuffer-local 
		0, nullptr,	0, nullptr, // VkMemoryBarrier or VkBufferMemoryBarrirer related
		1,						// imageMemoryBarrierCount
		&imageMemoryBarrier		// pImageMemoryBarriers
	);
	SenAbstractGLFW::endSingleTimeCommandBuffer(transitionImageLayoutCommandPool, logicalDevice, imageMemoryTransferQueue, transitionImageLayoutCommandBuffer);
	transitionImageLayoutCommandBuffer = VK_NULL_HANDLE;
}

void SenAbstractGLFW::transferResourceImage(const VkCommandPool& imageTransferCommandPool, const VkDevice& logicalDevice, const VkQueue& imageTransferQueue,
	const VkImage& srcImage, const VkImage& dstImage, const uint32_t& imageWidth, const uint32_t& imageHeight) {

	VkCommandBuffer imageCopyCommandBuffer = VK_NULL_HANDLE;
	SenAbstractGLFW::beginSingleTimeCommandBuffer(imageTransferCommandPool, logicalDevice, imageCopyCommandBuffer);

	VkImageSubresourceLayers imageSubresourceLayers{};
	imageSubresourceLayers.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
	imageSubresourceLayers.mipLevel			= 0; // the mipmap level to copy from; not sure why there's no mipCount
	imageSubresourceLayers.baseArrayLayer	= 0;
	imageSubresourceLayers.layerCount		= 1;

	VkImageCopy imageCopyRegion{};
	imageCopyRegion.srcSubresource	= imageSubresourceLayers;
	imageCopyRegion.dstSubresource	= imageSubresourceLayers;
	imageCopyRegion.srcOffset		= { 0, 0, 0 };
	imageCopyRegion.dstOffset		= { 0, 0, 0 };
	imageCopyRegion.extent.width	= imageWidth;
	imageCopyRegion.extent.height	= imageHeight;
	imageCopyRegion.extent.depth	= 1;

	vkCmdCopyImage(
		imageCopyCommandBuffer,
		srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, // Assuming here that they've been previously transitioned to the optimal transfer layouts.
		dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, // Assuming here that they've been previously transitioned to the optimal transfer layouts.
		1, &imageCopyRegion
	);

	SenAbstractGLFW::endSingleTimeCommandBuffer(imageTransferCommandPool, logicalDevice, imageTransferQueue, imageCopyCommandBuffer);
	imageCopyCommandBuffer = VK_NULL_HANDLE;
}

void SenAbstractGLFW::createDeviceLocalTexture(const VkDevice& logicalDevice, const VkPhysicalDeviceMemoryProperties& gpuMemoryProperties
	,const char*& textureDiskAddress, const VkImageType& imageType,  int& textureWidth, int& textureHeight
	,VkImage& deviceLocalTextureToCreate, VkDeviceMemory& textureDeviceMemoryToAllocate, VkImageView& textureImageViewToCreate
	,const VkSharingMode& imageSharingMode, const VkCommandPool& tmpCommandBufferCommandPool, const VkQueue& imageMemoryTransferQueue)
{
	// The pointer ptrBackgroundTexture returned from stbi_load(...) is the first element in an array of pixel values.
	int actuallyTextureChannels		= 0;
	stbi_uc* ptrDiskTextureToUpload = stbi_load(textureDiskAddress, &textureWidth, &textureHeight, &actuallyTextureChannels, STBI_rgb_alpha);
	if (!ptrDiskTextureToUpload) {
		throw std::runtime_error("failed to load texture image!");
	}
	/***********************************************************************************************************************************************/
	/***********************      First:   Upload/MapMemory texture image file as linear stagingImage       ****************************************/
	VkImage linearStagingImage;
	VkDeviceMemory linearStagingImageDeviceMemory;
	SenAbstractGLFW::createResourceImage(logicalDevice, textureWidth, textureHeight, imageType,
		VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_LINEAR, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, linearStagingImage, linearStagingImageDeviceMemory,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, imageSharingMode, gpuMemoryProperties);

	// The graphics card may assume that one row of pixels is not imageWidth * 4 bytes wide, but rather texWidth * 4 + paddingBytes;
	// To handle this for memcpy correctly, we need to query how bytes are arranged in our staging image using vkGetImageSubresourceLayout.
	VkImageSubresource textureImageSubresource{};
	textureImageSubresource.aspectMask	= VK_IMAGE_ASPECT_COLOR_BIT; // bitMask of color/depth/stencil/metadata
	textureImageSubresource.mipLevel	= 0; // subresource's index of mipLevel, 0 means the first mipLevel, only 0 for a texture
	textureImageSubresource.arrayLayer	= 0; // subresource's index of arrayLayer, 0 means the first arrayLayer, only 0 if not Textures (ArrayImage)

	VkSubresourceLayout linearStagingImageSubresourceLayout;
	vkGetImageSubresourceLayout(logicalDevice, linearStagingImage, &textureImageSubresource, &linearStagingImageSubresourceLayout);

	void* ptrHostVisibleTexture;
	VkDeviceSize hostVisibleImageDeviceSize = linearStagingImageSubresourceLayout.rowPitch * textureHeight ;
	vkMapMemory(logicalDevice, linearStagingImageDeviceMemory, 0, hostVisibleImageDeviceSize, 0, &ptrHostVisibleTexture);

	if (linearStagingImageSubresourceLayout.rowPitch == textureWidth * 4) {  // Channel == 4 due to STBI_rgb_alpha 
		// No padding bytes in rows if in this case; Usually with rowPitch == a power-of-2 size, e.g. 512 or 1024
		memcpy(ptrHostVisibleTexture, ptrDiskTextureToUpload, (size_t)hostVisibleImageDeviceSize);
	}else	{
		// otherwise, have to copy the pixels row-by-row with the right offset based on SubresourceLayout.rowPitch
		uint8_t* ptrHostVisibleDataBytes = reinterpret_cast<uint8_t*>(ptrHostVisibleTexture);
		for (int row = 0; row < textureHeight; row++) {
			memcpy(	&ptrHostVisibleDataBytes[row * linearStagingImageSubresourceLayout.rowPitch],
					&ptrDiskTextureToUpload	[row * textureWidth * 4],
					textureWidth * 4);
		}
	}

	vkUnmapMemory(logicalDevice, linearStagingImageDeviceMemory);
	stbi_image_free(ptrDiskTextureToUpload);
	/***********************************************************************************************************************************************/
	/****************      Second: Transfer stagingImage to deviceLocalTextureImage with correct textureImageLayout      ***************************/
	SenAbstractGLFW::createResourceImage(logicalDevice, textureWidth, textureHeight, imageType,
		VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, deviceLocalTextureToCreate
		, textureDeviceMemoryToAllocate, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, imageSharingMode, gpuMemoryProperties);
	
	VkImageSubresourceRange textureImageSubresourceRange{};
	textureImageSubresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
	textureImageSubresourceRange.baseMipLevel	= 0;	// first mipMap level to start
	textureImageSubresourceRange.levelCount		= 1;
	textureImageSubresourceRange.baseArrayLayer	= 0;	// first arrayLayer to start
	textureImageSubresourceRange.layerCount		= 1;

	SenAbstractGLFW::transitionResourceImageLayout(linearStagingImage, textureImageSubresourceRange, VK_IMAGE_LAYOUT_PREINITIALIZED,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_FORMAT_R8G8B8A8_UNORM, logicalDevice, tmpCommandBufferCommandPool, imageMemoryTransferQueue);
	SenAbstractGLFW::transitionResourceImageLayout(deviceLocalTextureToCreate, textureImageSubresourceRange, VK_IMAGE_LAYOUT_PREINITIALIZED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_FORMAT_R8G8B8A8_UNORM, logicalDevice, tmpCommandBufferCommandPool, imageMemoryTransferQueue);

	SenAbstractGLFW::transferResourceImage(tmpCommandBufferCommandPool, logicalDevice, imageMemoryTransferQueue,
		linearStagingImage, deviceLocalTextureToCreate, textureWidth, textureHeight);
	SenAbstractGLFW::transitionResourceImageLayout(deviceLocalTextureToCreate, textureImageSubresourceRange, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_FORMAT_R8G8B8A8_UNORM, logicalDevice, tmpCommandBufferCommandPool, imageMemoryTransferQueue);

	/***********************************************************************************************************************************************/
	/****************        Third:  clean the staging Image, DeviceMemory       *******************************************************************/
	vkDestroyImage(logicalDevice, linearStagingImage, nullptr);
	vkFreeMemory(logicalDevice, linearStagingImageDeviceMemory, nullptr); 	// always try to destroy before free
	linearStagingImage				= VK_NULL_HANDLE;
	linearStagingImageDeviceMemory	= VK_NULL_HANDLE;

	/***********************************************************************************************************************************************/
	/****************          Fourth:  create textureImageView       ******************************************************************************/
	VkImageViewCreateInfo textureImageViewCreateInfo{};
	textureImageViewCreateInfo.sType	= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	textureImageViewCreateInfo.image	= deviceLocalTextureToCreate;

	if (imageType == VK_IMAGE_TYPE_2D)
		textureImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	else 	throw std::runtime_error("No supported VkImageViewType set up for this imageType, Check it out !!!!\n");

	textureImageViewCreateInfo.format	= VK_FORMAT_R8G8B8A8_UNORM;
	textureImageViewCreateInfo.subresourceRange = textureImageSubresourceRange;

	SenAbstractGLFW::errorCheck(
		vkCreateImageView(logicalDevice, &textureImageViewCreateInfo, nullptr, &textureImageViewToCreate),
		std::string("Failed to create Resource Image View !!!")
	);
}

void SenAbstractGLFW::createPresentationSemaphores() {
	VkSemaphoreCreateInfo semaphoreCreateInfo{};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	SenAbstractGLFW::errorCheck(
		vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &swapchainImageAcquiredSemaphore),
		std::string("Failed to create swapchainImageAcquiredSemaphore !!!")
	);
	SenAbstractGLFW::errorCheck(
		vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &paintReadyToPresentSemaphore),
		std::string("Failed to create paintReadyToPresentSemaphore !!!")
	);
}

void SenAbstractGLFW::initGlfwVulkanDebugWSI()
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

	// GLFW allows us to store an arbitrary pointer in the window object with glfwSetWindowUserPointer,
	//   so we can specify a static class member and get the original class instance back with glfwGetWindowUserPointer;
	// We can then proceed to call recreateSwapChain, but only if the size of the window is non - zero;
	//   This case occurs when the window is minimized and it will cause swap chain creation to fail.
	glfwSetWindowUserPointer(widgetGLFW, this);
	glfwSetWindowSizeCallback(widgetGLFW, SenAbstractGLFW::onWidgetResized);

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
	createDefaultLogicalDevice();
	collectSwapchainFeatures();
	createSwapchain();
	createPresentationSemaphores();

	std::cout << "\n Finish  SenAbstractGLFW::initGlfwVulkanDebugWSI()\n";
}

void SenAbstractGLFW::showWidget()
{
	initGlfwVulkanDebugWSI();
	initVulkanApplication();
	// Game loop
	while (!glfwWindowShouldClose(widgetGLFW))
	{
		// Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
		glfwPollEvents();

		updateUniformBuffer();

		updateSwapchain();
	}

	// All of the operations in drawFrame are asynchronous, which means that when we exit the loop in mainLoop,
	//  drawing and presentation operations may still be going on, and cleaning up resources while that is happening is a bad idea;
	vkDeviceWaitIdle(device);
	// must finalize all objects after corresponding deviceWaitIdle
	finalizeWidget();
	finalizeAbstractGLFW();
	// Terminate GLFW, clearing any resources allocated by GLFW.
	glfwDestroyWindow(widgetGLFW);
	glfwTerminate();
}

void SenAbstractGLFW::createMvpUniformBuffers() {
	VkDeviceSize mvpUboUniformBufferDeviceSize = sizeof(MvpUniformBufferObject);

	SenAbstractGLFW::createResourceBuffer(device, mvpUboUniformBufferDeviceSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE, physicalDeviceMemoryProperties,
		mvpUniformStagingBuffer, mvpUniformStagingBufferDeviceMemory, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	SenAbstractGLFW::createResourceBuffer(device, mvpUboUniformBufferDeviceSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE, physicalDeviceMemoryProperties,
		mvpOptimalUniformBuffer, mvpOptimalUniformBufferMemory, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

}

void SenAbstractGLFW::collectSwapchainFeatures()
{
	/****************************************************************************************************************************/
	/********** Getting Surface Capabilities first to support SwapChain. ********************************************************/
	/*********** Could not do this right after surface creation, because GPU had not been seleted at that time ******************/
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);

	// Do the check & assignment below because what we surface size we got may not equal to what we set
	// Make sure the size of swapchain match the size of surface
	if (surfaceCapabilities.currentExtent.width < UINT32_MAX) {
		widgetWidth = surfaceCapabilities.currentExtent.width;
		widgetHeight = surfaceCapabilities.currentExtent.height;
	}
	else {
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
}

void SenAbstractGLFW::createSwapchain() {
	VkSwapchainCreateInfoKHR swapchainCreateInfo{};
	swapchainCreateInfo.sType				= VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.surface				= surface;
	swapchainCreateInfo.minImageCount		= swapchainImagesCount; // This is only to set the min value, instead of the actual imageCount after swapchain creation
	swapchainCreateInfo.imageFormat			= surfaceFormat.format;
	swapchainCreateInfo.imageColorSpace		= surfaceFormat.colorSpace;
	swapchainCreateInfo.imageExtent.width	= widgetWidth;
	swapchainCreateInfo.imageExtent.height	= widgetHeight;
	swapchainCreateInfo.imageArrayLayers	= 1;
	swapchainCreateInfo.imageUsage			= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	// Attention, figure out if the swapchain image will be shared by multiple Queues of different QueueFamilies !
	uint32_t queueFamilyIndicesArray[] = { static_cast<uint32_t>(graphicsQueueFamilyIndex), static_cast<uint32_t>(presentQueueFamilyIndex) };
	if (graphicsQueueFamilyIndex != presentQueueFamilyIndex) {
		swapchainCreateInfo.imageSharingMode		= VK_SHARING_MODE_CONCURRENT; // shared by different QueueFamily with different QueueFamilyIndex 
		swapchainCreateInfo.queueFamilyIndexCount	= 2;
		swapchainCreateInfo.pQueueFamilyIndices		= queueFamilyIndicesArray;
	}else {
		swapchainCreateInfo.imageSharingMode		= VK_SHARING_MODE_EXCLUSIVE;		// share between QueueFamilies or not
		swapchainCreateInfo.queueFamilyIndexCount	= 0;// no QueueFamily share
		swapchainCreateInfo.pQueueFamilyIndices		= nullptr;
	}

	swapchainCreateInfo.preTransform	= surfaceCapabilities.currentTransform; // Rotate of Mirror before presentation (VkSurfaceTransformFlagBitsKHR)
	swapchainCreateInfo.compositeAlpha	= VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;  // 
	swapchainCreateInfo.presentMode		= presentMode;
	swapchainCreateInfo.clipped			= VK_TRUE;	// Typically always set this true, such that Vulkan never render the invisible (out of visible range) image 

	swapchainCreateInfo.oldSwapchain	= swapChain;// resize window
	VkSwapchainKHR newSwapChain;
	SenAbstractGLFW::errorCheck(
		vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &newSwapChain),
		std::string("Fail to Create SwapChain !")
	);

	/**************************************************************************************************************************/
	/****************         Clean the old swapChain first, if exist, then assign new          *******************************/
	/**************************************************************************************************************************/
	if (VK_NULL_HANDLE != swapChain) {
		// swapChainImages will be handled by the destroy of swapchain
		// But swapchainImageViews need to be dstroyed first, before the destroy of swapchain.
		for (auto swapchainImageView : swapchainImageViewsVector) {
			vkDestroyImageView(device, swapchainImageView, nullptr);
		}
		swapchainImageViewsVector.clear();

		vkDestroySwapchainKHR(device, swapChain, nullptr);		swapChain = VK_NULL_HANDLE;
		swapchainImagesVector.clear();
	}
	swapChain = newSwapChain;

	// Get actual amount/count of swapchain images
	vkGetSwapchainImagesKHR(device, swapChain, &swapchainImagesCount, nullptr);

	swapchainImagesVector.resize(swapchainImagesCount);
	swapchainImageViewsVector.resize(swapchainImagesCount);
	// Get swapChainImages, instead of asking for real count.
	SenAbstractGLFW::errorCheck(
		vkGetSwapchainImagesKHR(device, swapChain, &swapchainImagesCount, swapchainImagesVector.data()),
		std::string("Failed to get SwapChain Images")
	);

	/**************************************************************************************************************************/
	/****************       Create corresponding swapchainImageViewVector       ***********************************************/
	/**************************************************************************************************************************/
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
		swapchainImageViewCreateInfo.subresourceRange.baseMipLevel = 0; // first mipmap level accessible to the view
		swapchainImageViewCreateInfo.subresourceRange.levelCount = 1; // amount of mipmaps
		swapchainImageViewCreateInfo.subresourceRange.baseArrayLayer = 0; // first array layer accessible to the view
		swapchainImageViewCreateInfo.subresourceRange.layerCount = 1; // if larger than 1, .viewType needs to be array

		SenAbstractGLFW::errorCheck(
			vkCreateImageView(device, &swapchainImageViewCreateInfo, nullptr, &swapchainImageViewsVector[i]),
			std::string("Failed to create SwapChan ImageViews !!")
		);
	}
}

void SenAbstractGLFW::updateSwapchain()
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
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
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

void SenAbstractGLFW::reInitPresentation()
{
	// Call vkDeviceWaitIdle() here, because we shouldn't touch resources that may still be in use. 
	vkDeviceWaitIdle(device);

	// Have to use this 3 commands to get currentExtent
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);
	if (surfaceCapabilities.currentExtent.width < UINT32_MAX) {
		widgetWidth = surfaceCapabilities.currentExtent.width;
		widgetHeight = surfaceCapabilities.currentExtent.height;
	}
	else {
		glfwGetWindowSize(widgetGLFW, &widgetWidth, &widgetHeight);
	}

	createSwapchain();
}

void SenAbstractGLFW::createDepthStencilAttachment()
{
	/********************************************************************************************************************/
	/******************************  Check Image Format *****************************************************************/
	for (auto f : SenAbstractGLFW::depthStencilSupportCheckFormatsVector) {
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

	uint32_t gpuMemoryTypeIndex = SenAbstractGLFW::findPhysicalDeviceMemoryPropertyIndex(
		physicalDeviceMemoryProperties,
		imageMemoryRequirements,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT  // Set the resource to reside on GPU itself
	);

	VkMemoryAllocateInfo depthStencilImageMemoryAllocateInfo{};
	depthStencilImageMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	depthStencilImageMemoryAllocateInfo.allocationSize = imageMemoryRequirements.size;
	depthStencilImageMemoryAllocateInfo.memoryTypeIndex = gpuMemoryTypeIndex;

	vkAllocateMemory(device, &depthStencilImageMemoryAllocateInfo, nullptr, &depthStencilImageDeviceMemory);
	vkBindImageMemory(device, depthStencilImage, depthStencilImageDeviceMemory, 0);

	/******************************************************************************************************************************************************/
	/******************************  Create depthStencil Image View ***************************************************************************************/
	VkImageViewCreateInfo depthStencilImageViewCreateInfo{};
	depthStencilImageViewCreateInfo.sType		= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	depthStencilImageViewCreateInfo.image		= depthStencilImage;
	depthStencilImageViewCreateInfo.viewType	= VK_IMAGE_VIEW_TYPE_2D;
	depthStencilImageViewCreateInfo.format		= depthStencilFormat;
	depthStencilImageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	depthStencilImageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	depthStencilImageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	depthStencilImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	depthStencilImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | (stencilAvailable ? VK_IMAGE_ASPECT_STENCIL_BIT : 0);
	depthStencilImageViewCreateInfo.subresourceRange.baseMipLevel	= 0;
	depthStencilImageViewCreateInfo.subresourceRange.levelCount		= 1;
	depthStencilImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	depthStencilImageViewCreateInfo.subresourceRange.layerCount		= 1;

	vkCreateImageView(device, &depthStencilImageViewCreateInfo, nullptr, &depthStencilImageView);


	//// Use command buffer to create the depth image. This includes -
	//// Command buffer allocation, recording with begin/end scope and submission.
	//CommandBufferMgr::allocCommandBuffer(&deviceObj->device, cmdPool, &cmdDepthImageCommandBuffer);
	//CommandBufferMgr::beginCommandBuffer(cmdDepthImageCommandBuffer);
	//{
	//	// Set the image layout to depth stencil optimal
	//	setImageLayout(FormatImageMemoryViewDepthStruct.image,
	//		depthImageViewCreateInfo.subresourceRange.aspectMask,
	//		VK_IMAGE_LAYOUT_UNDEFINED,
	//		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, (VkAccessFlagBits)0, cmdDepthImageCommandBuffer);
	//}
	//CommandBufferMgr::endCommandBuffer(cmdDepthImageCommandBuffer);
	//CommandBufferMgr::submitCommandBuffer(deviceObj->queue, &cmdDepthImageCommandBuffer);

	//// Create the image view and allow the application to use the images.
	//depthImageViewCreateInfo.image = FormatImageMemoryViewDepthStruct.image;
	//result = vkCreateImageView(deviceObj->device, &depthImageViewCreateInfo, NULL, &FormatImageMemoryViewDepthStruct.view);
	//assert(result == VK_SUCCESS);
}

void SenAbstractGLFW::createColorAttachOnlyRenderPass() {
	/********************************************************************************************************************/
	/************    Setting AttachmentDescription:  Only colorAttachment is needed for Triangle      *******************/
	/********************************************************************************************************************/
	std::array<VkAttachmentDescription, 1> attachmentDescriptionArray{};
	attachmentDescriptionArray[0].format			= surfaceFormat.format;// swapChainImageFormat;
	attachmentDescriptionArray[0].samples			= VK_SAMPLE_COUNT_1_BIT; // Not using multi-sampling
	attachmentDescriptionArray[0].loadOp			= VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescriptionArray[0].storeOp			= VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescriptionArray[0].stencilLoadOp		= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescriptionArray[0].stencilStoreOp	= VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescriptionArray[0].initialLayout		= VK_IMAGE_LAYOUT_UNDEFINED;       // layout before renderPass
	attachmentDescriptionArray[0].finalLayout		= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // auto transition after renderPass

	/********************************************************************************************************************/
	/********    Setting Subpasses with Dependencies: One subpass is enough to paint the triangle     *******************/
	/********************************************************************************************************************/
	std::array<VkAttachmentReference, 1> colorAttachmentReferenceArray{};
	colorAttachmentReferenceArray[0].attachment		= 0; // The colorAttachment index is 0
	colorAttachmentReferenceArray[0].layout			= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // auto transition during renderPass

	std::array<VkSubpassDescription, 1> subpassDescriptionArray{};
	subpassDescriptionArray[0].pipelineBindPoint	= VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescriptionArray[0].colorAttachmentCount = (uint32_t)colorAttachmentReferenceArray.size();	// Every subpass references one or more attachments
	subpassDescriptionArray[0].pColorAttachments	= colorAttachmentReferenceArray.data();
	
	/******* There are two built-in dependencies that take care of the transition at the start of the render pass and at the end of the render pass,
	/////       but the former does not occur at the right time. 
	/////       It assumes that the transition occurs at the start of the pipeline, but we haven't acquired the image yet at that point! ****/
	/******* There are two ways to deal with this problem.
	/////       We could change the waitStages for the imageAvailableSemaphore to VK_PIPELINE_STAGE_TOP_OF_PIPELINE_BIT
	/////        to ensure that the render passes don't begin until the image is available, 
	/////       or we can make the render pass wait for the VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT stage. ****************************/
	std::vector<VkSubpassDependency> subpassDependencyVector;
	VkSubpassDependency headSubpassDependency{};
	headSubpassDependency.srcSubpass		= VK_SUBPASS_EXTERNAL;		// subpassIndex, from external
	headSubpassDependency.dstSubpass		= 0;						// subpassIndex, to the first subpass, which is also the only one
	headSubpassDependency.srcStageMask		= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // specify the operations to wait on and the stages in which these operations occur.
	headSubpassDependency.dstStageMask		= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	headSubpassDependency.srcAccessMask		= 0;											 // specify the operations to wait on and the stages in which these operations occur.
	headSubpassDependency.dstAccessMask		= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT
												| VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	
	subpassDependencyVector.push_back(headSubpassDependency);

	/********************************************************************************************************************/
	/*********************    Create RenderPass for rendering triangle      *********************************************/
	/********************************************************************************************************************/
	VkRenderPassCreateInfo triangleRenderPassCreateInfo{};
	triangleRenderPassCreateInfo.sType				= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	triangleRenderPassCreateInfo.attachmentCount	= (uint32_t)attachmentDescriptionArray.size();
	triangleRenderPassCreateInfo.pAttachments		= attachmentDescriptionArray.data();
	triangleRenderPassCreateInfo.subpassCount		= (uint32_t)subpassDescriptionArray.size();
	triangleRenderPassCreateInfo.pSubpasses			= subpassDescriptionArray.data();
	triangleRenderPassCreateInfo.dependencyCount	= (uint32_t)subpassDependencyVector.size();
	triangleRenderPassCreateInfo.pDependencies		= subpassDependencyVector.data();

	SenAbstractGLFW::errorCheck(
		vkCreateRenderPass(device, &triangleRenderPassCreateInfo, nullptr, &colorAttachOnlyRenderPass),
		std::string("Failed to create render pass !!")
	);
}

void SenAbstractGLFW::createDepthStencilRenderPass()
{
	std::array<VkAttachmentDescription, 2> attachmentDescriptionsArray{}; // for both of color and depthStencil
	attachmentDescriptionsArray[0].flags = 0;
	attachmentDescriptionsArray[0].format = depthStencilFormat;
	attachmentDescriptionsArray[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachmentDescriptionsArray[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescriptionsArray[0].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescriptionsArray[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescriptionsArray[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescriptionsArray[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachmentDescriptionsArray[0].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	attachmentDescriptionsArray[1].flags = 0;
	attachmentDescriptionsArray[1].format = surfaceFormat.format;
	attachmentDescriptionsArray[1].samples = VK_SAMPLE_COUNT_1_BIT;
	attachmentDescriptionsArray[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescriptionsArray[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescriptionsArray[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachmentDescriptionsArray[1].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference depthStencilAttachmentReference{};
	depthStencilAttachmentReference.attachment = 0;
	depthStencilAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	std::array<VkAttachmentReference, 1> colorAttachmentReferenceArray{};
	colorAttachmentReferenceArray[0].attachment = 1;
	colorAttachmentReferenceArray[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	std::array<VkSubpassDescription, 1> subpassDescriptionArray{};
	subpassDescriptionArray[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescriptionArray[0].colorAttachmentCount = (uint32_t)colorAttachmentReferenceArray.size();
	subpassDescriptionArray[0].pColorAttachments = colorAttachmentReferenceArray.data();		// layout(location=0) out vec4 FinalColor;
	subpassDescriptionArray[0].pDepthStencilAttachment = &depthStencilAttachmentReference;


	VkRenderPassCreateInfo renderPassCreateInfo{};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = (uint32_t)attachmentDescriptionsArray.size();
	renderPassCreateInfo.pAttachments = attachmentDescriptionsArray.data();
	renderPassCreateInfo.subpassCount = (uint32_t)subpassDescriptionArray.size();
	renderPassCreateInfo.pSubpasses = subpassDescriptionArray.data();

	SenAbstractGLFW::errorCheck(
		vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &depthTestRenderPass),
		std::string("Failed to create colorDepthStencil render pass !!")
	);
}

void SenAbstractGLFW::createDepthStencilGraphicsPipeline()
{
}

void SenAbstractGLFW::createShaderModuleFromSPIRV(const VkDevice& logicalDevice, const std::vector<char>& SPIRV_Vector, VkShaderModule & targetShaderModule)
{
	VkShaderModuleCreateInfo shaderModuleCreateInfo{};
	shaderModuleCreateInfo.sType	= VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderModuleCreateInfo.codeSize = SPIRV_Vector.size();
	shaderModuleCreateInfo.pCode	= (uint32_t*)SPIRV_Vector.data();

	SenAbstractGLFW::errorCheck(
		vkCreateShaderModule(logicalDevice, &shaderModuleCreateInfo, nullptr, &targetShaderModule),
		std::string("Failed to create the shader module !!")
	);
}

void SenAbstractGLFW::createVulkanShaderModule(const VkDevice& logicalDevice, const std::string& diskFileAddress, VkShaderModule& shaderModule) {
	std::vector<char> spirvCharVector;// (spirv32Vector.size() * sizeof(uint32_t) / sizeof(char));

	if (diskFileAddress.substr(diskFileAddress.length() - 4, 4).compare(".spv") == 0)
		spirvCharVector = SenAbstractGLFW::readFileStream(diskFileAddress, true);
	else {
		std::string shaderTypeString = diskFileAddress.substr(diskFileAddress.length() - 5, 5);
		shaderc_shader_kind shadercType;
		if		(shaderTypeString.compare(".vert") == 0)		shadercType = shaderc_glsl_vertex_shader;
		else if (shaderTypeString.compare(".frag") == 0)		shadercType = shaderc_glsl_fragment_shader;
		else if (shaderTypeString.compare(".geom") == 0)		shadercType = shaderc_glsl_geometry_shader;
		// Android system Attension:   sourceString size for shadercToSPIRV() below may change.
		std::vector<uint32_t> spirv32Vector = SenAbstractGLFW::shadercToSPIRV("glShaderSrc", shadercType, SenAbstractGLFW::readFileStream(diskFileAddress).data());
		spirvCharVector.resize(spirv32Vector.size() * sizeof(uint32_t) / sizeof(char));
		memcpy(spirvCharVector.data(), spirv32Vector.data(), spirvCharVector.size());
	}

	createShaderModuleFromSPIRV(logicalDevice, spirvCharVector, shaderModule);
}

void SenAbstractGLFW::createColorAttachOnlySwapchainFramebuffers() {
	/************************************************************************************************************/
	/*****************     Destroy old swapchainFramebuffers first, if there are      ****************************/
	/************************************************************************************************************/
	for (auto swapchainFramebuffer : swapchainFramebufferVector) {
		vkDestroyFramebuffer(device, swapchainFramebuffer, nullptr);
	}
	swapchainFramebufferVector.clear();
	swapchainFramebufferVector.resize(swapchainImagesCount);

	for (size_t i = 0; i < swapchainImagesCount; i++) {

		std::array<VkImageView, 1> imageViewAttachmentArray{};
		imageViewAttachmentArray[0] = swapchainImageViewsVector[i];

		VkFramebufferCreateInfo framebufferCreateInfo{};
		framebufferCreateInfo.sType				= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.renderPass		= colorAttachOnlyRenderPass;
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

void SenAbstractGLFW::createDefaultCommandPool() {

	VkCommandPoolCreateInfo commandPoolCreateInfo{};
	commandPoolCreateInfo.sType				= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.flags				= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // allow commandBuffer to be individually reset
	commandPoolCreateInfo.queueFamilyIndex	= graphicsQueueFamilyIndex;

	SenAbstractGLFW::errorCheck(
		vkCreateCommandPool(device, &commandPoolCreateInfo, nullptr, &defaultThreadCommandPool),
		std::string("Failed to create defaultThreadCommandPool !!!")
	);

}

void SenAbstractGLFW::createResourceBuffer(const VkDevice& logicalDevice, const VkDeviceSize& bufferDeviceSize,
	const VkBufferUsageFlags& bufferUsageFlags, const VkSharingMode& bufferSharingMode, const VkPhysicalDeviceMemoryProperties& gpuMemoryProperties,
	VkBuffer& bufferToCreate, VkDeviceMemory& bufferDeviceMemoryToAllocate, const VkMemoryPropertyFlags& requiredMemoryPropertyFlags) {
	/*****************************************************************************************************************************************************/
	VkBufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = bufferDeviceSize;
	bufferCreateInfo.usage = bufferUsageFlags;
	bufferCreateInfo.sharingMode = bufferSharingMode;

	SenAbstractGLFW::errorCheck(
		vkCreateBuffer(logicalDevice, &bufferCreateInfo, nullptr, &bufferToCreate),
		std::string("Failed to create a Buffer Resource !!!")
	);
	/*****************************************************************************************************************************************************/
	VkMemoryRequirements bufferMemoryRequirements{};
	vkGetBufferMemoryRequirements(logicalDevice, bufferToCreate, &bufferMemoryRequirements);

	VkMemoryAllocateInfo bufferMemoryAllocateInfo{};
	bufferMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	bufferMemoryAllocateInfo.allocationSize = bufferMemoryRequirements.size;
	bufferMemoryAllocateInfo.memoryTypeIndex
		= SenAbstractGLFW::findPhysicalDeviceMemoryPropertyIndex(gpuMemoryProperties, bufferMemoryRequirements, requiredMemoryPropertyFlags);

	SenAbstractGLFW::errorCheck(
		vkAllocateMemory(logicalDevice, &bufferMemoryAllocateInfo, nullptr, &bufferDeviceMemoryToAllocate),
		std::string("Failed to allocate triangleVertexBufferMemory !!!")
	);

	vkBindBufferMemory(logicalDevice, bufferToCreate, bufferDeviceMemoryToAllocate, 0);
}

void SenAbstractGLFW::beginSingleTimeCommandBuffer(const VkCommandPool& tmpCommandBufferCommandPool, const VkDevice& logicalDevice, 
												VkCommandBuffer& tempCommandBufferToBegin) {
	// You may wish to create a separate command pool for these kinds of short - lived buffers, 
	// because the implementation may be able to apply memory allocation optimizations.
	// You should use the VK_COMMAND_POOL_CREATE_TRANSIENT_BIT flag during command pool generation in that case.
	
	VkCommandBufferAllocateInfo tmpCommandBufferAllocateInfo{};
	tmpCommandBufferAllocateInfo.sType				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	tmpCommandBufferAllocateInfo.level				= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	tmpCommandBufferAllocateInfo.commandPool		= tmpCommandBufferCommandPool;
	tmpCommandBufferAllocateInfo.commandBufferCount = 1;
	vkAllocateCommandBuffers(logicalDevice, &tmpCommandBufferAllocateInfo, &tempCommandBufferToBegin);

	VkCommandBufferBeginInfo tmpCommandBufferBeginInfo{};
	tmpCommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	tmpCommandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(tempCommandBufferToBegin, &tmpCommandBufferBeginInfo);
}

void SenAbstractGLFW::endSingleTimeCommandBuffer(const VkCommandPool& tmpCommandBufferCommandPool, const VkDevice& logicalDevice
											, const VkQueue& tmpCommandBufferQueue, const VkCommandBuffer& tmpCommandBufferToEnd) {
	// You may wish to create a separate command pool for these kinds of short - lived buffers, 
	// because the implementation may be able to apply memory allocation optimizations.
	// You should use the VK_COMMAND_POOL_CREATE_TRANSIENT_BIT flag during command pool generation in that case.
	vkEndCommandBuffer(tmpCommandBufferToEnd);

	VkSubmitInfo tmpCommandBufferSubmitInfo{};
	tmpCommandBufferSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	tmpCommandBufferSubmitInfo.commandBufferCount = 1;
	tmpCommandBufferSubmitInfo.pCommandBuffers = &tmpCommandBufferToEnd;

	vkQueueSubmit(tmpCommandBufferQueue, 1, &tmpCommandBufferSubmitInfo, VK_NULL_HANDLE);
	// There are again two possible ways to wait on this transfer to complete:
	//   1. We could use a fence and wait with vkWaitForFences, which would allow you to schedule multiple transfers simultaneously 
	//			and wait for all of them complete, instead of executing one at a time;
	//   2. Simply wait for the transfer queue to become idle with vkQueueWaitIdle.
	vkQueueWaitIdle(tmpCommandBufferQueue);
	vkFreeCommandBuffers(logicalDevice, tmpCommandBufferCommandPool, 1, &tmpCommandBufferToEnd);
}

void SenAbstractGLFW::transferResourceBuffer(const VkCommandPool& bufferTransferCommandPool, const VkDevice& logicalDevice, const VkQueue& bufferMemoryTransferQueue,
	const VkBuffer& srcBuffer, const VkBuffer& dstBuffer, const VkDeviceSize& resourceBufferSize) {

	VkCommandBuffer bufferCopyCommandBuffer = VK_NULL_HANDLE;
	SenAbstractGLFW::beginSingleTimeCommandBuffer(bufferTransferCommandPool, logicalDevice,	bufferCopyCommandBuffer);

	VkBufferCopy bufferCopyRegion{};
	bufferCopyRegion.size = resourceBufferSize;
	vkCmdCopyBuffer(bufferCopyCommandBuffer, srcBuffer, dstBuffer, 1, &bufferCopyRegion);

	SenAbstractGLFW::endSingleTimeCommandBuffer(bufferTransferCommandPool, logicalDevice, bufferMemoryTransferQueue, bufferCopyCommandBuffer);
	bufferCopyCommandBuffer = VK_NULL_HANDLE;
}

void SenAbstractGLFW::createSingleRectIndexBuffer()
{
	uint16_t indices[] = { 0, 1, 2, 1, 2, 3 };
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

	SenAbstractGLFW::errorCheck(
		vkCreateInstance(&instanceCreateInfo, nullptr, &instance),
		std::string("Failed to create instance! \t Error:\t")
	);
}

void SenAbstractGLFW::initDebugReportCallback()
{
	fetch_vkCreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT"));
	fetch_vkDestroyDebugReportCallbackEXT = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT"));

	if (nullptr == fetch_vkCreateDebugReportCallbackEXT || nullptr == fetch_vkDestroyDebugReportCallbackEXT) {
		throw std::runtime_error("Vulkan Error: Can't fetch debug function pointers.");
		std::exit(-1);
	}

	SenAbstractGLFW::errorCheck(
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
	/*****  Acquire the physicalDeviceMemoryProperties & physicalDeviceFeatures of the selected GPU   *****/
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &physicalDeviceMemoryProperties);
	vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatures);
	std::cout << "\n\nSelected GPU Properties:\n\t\t";	showPhysicalDeviceInfo(physicalDevice);
	std::cout << std::endl;
}

void SenAbstractGLFW::createSurface()
{
	SenAbstractGLFW::errorCheck(
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

	if (!physicalDeviceFeatures.geometryShader || !physicalDeviceFeatures.samplerAnisotropy)	return VK_FALSE;
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
	if (!physicalDeviceFeatures.geometryShader || !physicalDeviceFeatures.samplerAnisotropy)
		return 0;	// Application can't function without geometry shaders
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

void SenAbstractGLFW::createDefaultLogicalDevice()
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
	deviceCreateInfo.pEnabledFeatures = &physicalDeviceFeatures;

	if (layersEnabled) {
		deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(debugDeviceLayersVector.size());   				// depricated
		deviceCreateInfo.ppEnabledLayerNames = debugDeviceLayersVector.data();				// depricated
	}
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(debugDeviceExtensionsVector.size());
	deviceCreateInfo.ppEnabledExtensionNames = debugDeviceExtensionsVector.data();

	SenAbstractGLFW::errorCheck(
		vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device),
		std::string("Fail at Create Logical Device!")
	);

	// Retrieve queue handles for each queue family
	vkGetDeviceQueue(device, graphicsQueueFamilyIndex, 0, &graphicsQueue);
	vkGetDeviceQueue(device, presentQueueFamilyIndex, 0, &presentQueue); // We only need 1 queue, so the third parameter (index) we give is 0.
}

void SenAbstractGLFW::finalizeAbstractGLFW() {
	/************************************************************************************************************/
	/*********************           Destroy defaultThreadCommandPool         ***********************************/
	/************************************************************************************************************/
	if (VK_NULL_HANDLE != defaultThreadCommandPool) {
		vkDestroyCommandPool(device, defaultThreadCommandPool, nullptr);
		defaultThreadCommandPool = VK_NULL_HANDLE;
		
		swapchainCommandBufferVector.clear();
	}
	/************************************************************************************************************/
	/*************      Destroy descriptorPool,  perspectiveProjection_DSL, perspectiveProjection_DS      ************/
	/************************************************************************************************************/
	if (VK_NULL_HANDLE != descriptorPool) {
		vkDestroyDescriptorPool(device, descriptorPool, nullptr);
		// When a DescriptorPool is destroyed, all descriptor sets allocated from the pool are implicitly freed and become invalid
		vkDestroyDescriptorSetLayout(device, perspectiveProjection_DSL, nullptr);

		perspectiveProjection_DSL	= VK_NULL_HANDLE;
		descriptorPool				= VK_NULL_HANDLE;
		perspectiveProjection_DS	= VK_NULL_HANDLE;
	}
	/************************************************************************************************************/
	/*********************           Destroy common RenderPass if not VK_NULL_HANDLE        *********************/
	/************************************************************************************************************/
	if (VK_NULL_HANDLE != colorAttachOnlyRenderPass) {
		vkDestroyRenderPass(device, colorAttachOnlyRenderPass, nullptr);
		colorAttachOnlyRenderPass = VK_NULL_HANDLE;
	}
	/************************************************************************************************************/
	/******************     Destroy VertexBuffer, VertexBufferMemory     ****************************************/
	/************************************************************************************************************/
	if (VK_NULL_HANDLE != singleRectIndexBuffer) {
		vkDestroyBuffer(device, singleRectIndexBuffer, nullptr);
		vkFreeMemory(device, singleRectIndexBufferMemory, nullptr);	// always try to destroy before free

		singleRectIndexBuffer		= VK_NULL_HANDLE;
		singleRectIndexBufferMemory	= VK_NULL_HANDLE;
	}
	if (VK_NULL_HANDLE != mvpUniformStagingBuffer) {
		vkDestroyBuffer(device, mvpUniformStagingBuffer, nullptr);
		vkFreeMemory(device, mvpUniformStagingBufferDeviceMemory, nullptr);	// always try to destroy before free

		mvpUniformStagingBuffer				= VK_NULL_HANDLE;
		mvpUniformStagingBufferDeviceMemory = VK_NULL_HANDLE;
	}
	if (VK_NULL_HANDLE != mvpOptimalUniformBuffer) {
		vkDestroyBuffer(device, mvpOptimalUniformBuffer, nullptr);
		vkFreeMemory(device, mvpOptimalUniformBufferMemory, nullptr);	// always try to destroy before free

		mvpOptimalUniformBuffer			= VK_NULL_HANDLE;
		mvpOptimalUniformBufferMemory	= VK_NULL_HANDLE;
	}
	/************************************************************************************************************/
	/******************     Destroy depthStencil Memory, ImageView, Image     ***********************************/
	/************************************************************************************************************/
	if (VK_NULL_HANDLE != depthStencilImage) {
		vkDestroyImage(device, depthStencilImage, nullptr);
		vkDestroyImageView(device, depthStencilImageView, nullptr);
		vkFreeMemory(device, depthStencilImageDeviceMemory, nullptr); 	// always try to destroy before free

		depthStencilImage				= VK_NULL_HANDLE;
		depthStencilImageDeviceMemory	= VK_NULL_HANDLE;
		depthStencilImageView			= VK_NULL_HANDLE;
	}
	/************************************************************************************************************/
	/*****  SwapChain is a child of Logical Device, must be destroyed before Logical Device  ********************/
	/****************   A surface must outlive any swapchains targeting it    ***********************************/
	if (VK_NULL_HANDLE != swapChain) {
		vkDestroySwapchainKHR(device, swapChain, nullptr);
		// swapChainImages will be handled by the destroy of swapchain
		// But swapchainImageViews need to be dstroyed first, before the destroy of swapchain.
		for (auto swapchainImageView : swapchainImageViewsVector) {
			vkDestroyImageView(device, swapchainImageView, nullptr);
		}
		swapchainImageViewsVector.clear();

		swapChain = VK_NULL_HANDLE;
		swapchainImagesVector.clear();

		// The memory of swapChain images is not managed by programmer (No allocation, nor free)
		// It may not be freed until the window is destroyed, or another swapchain is created for the window.

		/************************************************************************************************************/
		/*********************           Destroy swapchainFramebuffer         ***************************************/
		/************************************************************************************************************/
		for (auto swapchainFramebuffer : swapchainFramebufferVector) {
			vkDestroyFramebuffer(device, swapchainFramebuffer, nullptr);
		}
		swapchainFramebufferVector.clear();
	}
	/************************************************************************************************************/
	/*********************           Destroy Synchronization Items             **********************************/
	/************************************************************************************************************/
	if (VK_NULL_HANDLE != swapchainImageAcquiredSemaphore) {
		vkDestroySemaphore(device, swapchainImageAcquiredSemaphore, nullptr);
		swapchainImageAcquiredSemaphore = VK_NULL_HANDLE;
	}
	if (VK_NULL_HANDLE != paintReadyToPresentSemaphore) {
		vkDestroySemaphore(device, paintReadyToPresentSemaphore, nullptr);
		paintReadyToPresentSemaphore = VK_NULL_HANDLE;
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

	OutputDebugString("\n\tFinish  SenAbstractGLFW::finalizeAbstractGLFW()\n");
}

uint32_t SenAbstractGLFW::findPhysicalDeviceMemoryPropertyIndex(
	const VkPhysicalDeviceMemoryProperties& gpuMemoryProperties,
	const VkMemoryRequirements& memoryRequirements,
	const VkMemoryPropertyFlags& requiredMemoryPropertyFlags)
{
	for (uint32_t gpuMemoryTypeIndex = 0; gpuMemoryTypeIndex < gpuMemoryProperties.memoryTypeCount; ++gpuMemoryTypeIndex) {
		if (memoryRequirements.memoryTypeBits & (1 << gpuMemoryTypeIndex)) {
			if ((gpuMemoryProperties.memoryTypes[gpuMemoryTypeIndex].propertyFlags & requiredMemoryPropertyFlags) == requiredMemoryPropertyFlags) {
				return gpuMemoryTypeIndex;
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

void SenAbstractGLFW::onWidgetResized(GLFWwindow* widget, int width, int height) {
	if (width == 0 || height == 0) return;

	SenAbstractGLFW* ptrAbstractWidget = reinterpret_cast<SenAbstractGLFW*>(glfwGetWindowUserPointer(widget));
	ptrAbstractWidget->reInitPresentation();
	ptrAbstractWidget->reCreateRenderTarget();
}

std::vector<char> SenAbstractGLFW::readFileStream(const std::string& diskFileAddress, bool binary) {
	std::ifstream file(diskFileAddress, binary ? std::ios::ate | std::ios::binary : std::ios::ate);

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

// Compiles a shader to a SPIR-V binary. Returns the binary as a vector of 32-bit words.
std::vector<uint32_t> SenAbstractGLFW::shadercToSPIRV(const std::string& source_name,
	shaderc_shader_kind kind, const std::string& source, bool optimize) {
	shaderc::Compiler compiler;
	shaderc::CompileOptions options;

	// Like -DMY_DEFINE=1
	options.AddMacroDefinition("MY_DEFINE", "1");
	if (optimize) options.SetOptimizationLevel(shaderc_optimization_level_size);

	shaderc::SpvCompilationResult module =
		compiler.CompileGlslToSpv(source, kind, source_name.c_str(), options);

	if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
		std::cerr << module.GetErrorMessage();
		return std::vector<uint32_t>();
	}

	return{ module.cbegin(), module.cend() };
}

/************************************************************************************************************************************************************************************************************/
/***********     This is a helper function that records memory barrirers using the vkCmdPipelineBarrier() command      **************************************************************************************/
/************************************************************************************************************************************************************************************************************/
void SenAbstractGLFW::setImageMemoryBarrier(VkImage image, VkImageAspectFlags imageAspectFlags
	, VkImageLayout oldImageLayout, VkImageLayout newImageLayout
	, VkAccessFlagBits srcAccessFlagBits, const VkCommandBuffer& imageLayoutTransitionCommandBuffer)
{
	// Dependency on imageLayoutTransitionCommandBuffer
	if (VK_NULL_HANDLE == imageLayoutTransitionCommandBuffer) {
		throw std::runtime_error("failed to open file!");
	}

	VkImageMemoryBarrier imgMemoryBarrier{};
	imgMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imgMemoryBarrier.pNext = NULL;
	imgMemoryBarrier.srcAccessMask = srcAccessFlagBits;
	imgMemoryBarrier.dstAccessMask = 0;
	imgMemoryBarrier.oldLayout = oldImageLayout;
	imgMemoryBarrier.newLayout = newImageLayout;
	imgMemoryBarrier.image = image;
	imgMemoryBarrier.subresourceRange.aspectMask = imageAspectFlags;
	imgMemoryBarrier.subresourceRange.baseMipLevel = 0;
	imgMemoryBarrier.subresourceRange.levelCount = 1;
	imgMemoryBarrier.subresourceRange.layerCount = 1;

	if (oldImageLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
		imgMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	}

	switch (newImageLayout)
	{
		// Ensure that anything that was copying from this image has completed
		// An image in this layout can only be used as the destination operand of the commands
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
			imgMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;

		// Ensure any Copy or CPU writes to image are flushed
		// An image in this layout can only be used as a read-only shader resource
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			imgMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			imgMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			break;

		// An image in this layout can only be used as a framebuffer color attachment
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			imgMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
			break;

		// An image in this layout can only be used as a framebuffer depth/stencil attachment
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			imgMemoryBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;
	}

	VkPipelineStageFlags srcStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	VkPipelineStageFlags destStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

	vkCmdPipelineBarrier(imageLayoutTransitionCommandBuffer, srcStages, destStages, 0, 0, NULL, 0, NULL, 1, &imgMemoryBarrier);
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
		SenAbstractGLFW::errorCheck(vkEnumerateInstanceExtensionProperties(instanceLayersVector[i].layerName, &extensionsCount, NULL), std::string("Extension Count under Layer Error!"));
		std::vector<VkExtensionProperties> extensionsPropVec(extensionsCount);
		SenAbstractGLFW::errorCheck(vkEnumerateInstanceExtensionProperties(instanceLayersVector[i].layerName, &extensionsCount, extensionsPropVec.data()), std::string("Extension Data() under Layer Error!"));

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
	SenAbstractGLFW::errorCheck(vkEnumerateDeviceExtensionProperties(gpuToCheck, nullptr, &gpuExtensionsCount, nullptr), std::string("GPU Extension Count under Layer Error!"));
	std::vector<VkExtensionProperties> gpuExtensionsPropVec(gpuExtensionsCount);
	SenAbstractGLFW::errorCheck(vkEnumerateDeviceExtensionProperties(gpuToCheck, nullptr, &gpuExtensionsCount, gpuExtensionsPropVec.data()), std::string("GPU Extension Data() under Layer Error!"));
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
		SenAbstractGLFW::errorCheck(vkEnumerateDeviceExtensionProperties(gpuToCheck, physicalDeviceLayersVector[i].layerName, &gpuExtensionsCount, NULL), std::string("GPU Extension Count under Layer Error!"));
		std::vector<VkExtensionProperties> gpuExtensionsPropVec(gpuExtensionsCount);
		SenAbstractGLFW::errorCheck(vkEnumerateDeviceExtensionProperties(gpuToCheck, physicalDeviceLayersVector[i].layerName, &gpuExtensionsCount, gpuExtensionsPropVec.data()), std::string("GPU Extension Data() under Layer Error!"));

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
