#include "SenAbstractGLFW.h"

// Since stb_image.h header file contains the implementation of functions, only one class source file could include it to make new implementation
// all stb_image realated functions have to be implemented in this class
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

SenAbstractGLFW::SenAbstractGLFW()
//:xRot(0), yRot(0), aspect(1.0)
{
	//showAllSupportedInstanceExtensions(); // Not Useful Functions
	//showAllSupportedInstanceLayers(); // Not Useful Functions
	//showAllSupportedExtensionsEachUnderInstanceLayer(); // Not Useful Functions

	widgetWidth = DEFAULT_widgetWidth;
	widgetHeight = DEFAULT_widgetHeight;

	strWindowName = "Sen GLFW Vulkan Application";
}

SenAbstractGLFW::~SenAbstractGLFW()
{
	finalize();
	OutputDebugString("\n ~SenAbstractGLWidget()\n");
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


//void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
//	VkImageMemoryBarrier imageMemoryBarrier = {};
//	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
//	barrier.oldLayout = oldLayout;
//	barrier.newLayout = newLayout;
//	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//	barrier.image = image;
//	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//	barrier.subresourceRange.baseMipLevel = 0;
//	barrier.subresourceRange.levelCount = 1;
//	barrier.subresourceRange.baseArrayLayer = 0;
//	barrier.subresourceRange.layerCount = 1;
//
//	if (oldLayout == VK_IMAGE_LAYOUT_PREINITIALIZED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
//		barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
//		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
//	}
//	else if (oldLayout == VK_IMAGE_LAYOUT_PREINITIALIZED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
//		barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
//		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
//	}
//	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
//		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
//		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
//	}
//	else {
//		throw std::invalid_argument("unsupported layout transition!");
//	}
//
//	VkCommandBuffer commandBuffer = beginSingleTimeCommands();
//	vkCmdPipelineBarrier(
//		commandBuffer,
//		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
//		0,
//		0, nullptr,
//		0, nullptr,
//		1, &barrier
//	);
//
//	endSingleTimeCommands(commandBuffer);
//}

void SenAbstractGLFW::createDeviceLocalTextureImage(const VkDevice& logicalDevice
	, const char*& textureDiskAddress, const VkImageType& imageType, int& textureImageWidth, int& textureImageHeight
	, VkImage& deviceLocalTextureToCreate, VkDeviceMemory& deviceLocalTextureDeviceMemoryToAllocate, VkImageView& deviceLocalTextureImageView
	, VkSampler& deviceLocalTextureSampler, const VkSharingMode& imageSharingMode, const VkPhysicalDeviceMemoryProperties& gpuMemoryProperties)
{
	// The pointer ptrBackgroundTexture returned from stbi_load(...) is the first element in an array of pixel values.
	int actuallyTextureChannels		= 0;
	stbi_uc* ptrDiskTextureToUpload = stbi_load(textureDiskAddress, &textureImageWidth, &textureImageHeight, &actuallyTextureChannels, STBI_rgb_alpha);
	if (!ptrDiskTextureToUpload) {
		throw std::runtime_error("failed to load texture image!");
	}
	/***********************************************************************************************************************************************/
	/***********************      First:   Upload/MapMemory texture image as stagingImage       ****************************************************/
	VkImage linearStagingImage;
	VkDeviceMemory linearStagingImageDeviceMemory;
	SenAbstractGLFW::createResourceImage(logicalDevice, textureImageWidth, textureImageHeight, imageType,
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
	VkDeviceSize hostVisibleImageDeviceSize = linearStagingImageSubresourceLayout.rowPitch * textureImageHeight ;
	vkMapMemory(logicalDevice, linearStagingImageDeviceMemory, 0, hostVisibleImageDeviceSize, 0, &ptrHostVisibleTexture);

	if (linearStagingImageSubresourceLayout.rowPitch == textureImageWidth * 4) {  // Channel == 4 due to STBI_rgb_alpha 
		// No padding bytes in rows if in this case; Usually with rowPitch == a power-of-2 size, e.g. 512 or 1024
		memcpy(ptrHostVisibleTexture, ptrDiskTextureToUpload, (size_t)hostVisibleImageDeviceSize);
	}else	{
		// otherwise, have to copy the pixels row-by-row with the right offset based on SubresourceLayout.rowPitch
		uint8_t* ptrHostVisibleDataBytes = reinterpret_cast<uint8_t*>(ptrHostVisibleTexture);
		for (int row = 0; row < textureImageHeight; row++) {
			memcpy(	&ptrHostVisibleDataBytes[row * linearStagingImageSubresourceLayout.rowPitch],
					&ptrDiskTextureToUpload	[row * textureImageWidth * 4],
					textureImageWidth * 4);
		}
	}

	vkUnmapMemory(logicalDevice, linearStagingImageDeviceMemory);
	stbi_image_free(ptrDiskTextureToUpload);
	/***********************************************************************************************************************************************/
	/***********************      Second:          ****************************************************/
	SenAbstractGLFW::createResourceImage(logicalDevice, textureImageWidth, textureImageHeight, imageType,
		VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, deviceLocalTextureToCreate
		, deviceLocalTextureDeviceMemoryToAllocate, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, imageSharingMode, gpuMemoryProperties);
	
	//transitionImageLayout(linearStagingImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	//transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	//copyImage(linearStagingImage, textureImage, textureImageWidth, textureImageHeight);

	//transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void SenAbstractGLFW::paintVulkan(void)
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
		reCreateTriangleSwapchain();
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
	submitInfo.waitSemaphoreCount	= (uint32_t)submitInfoWaitSemaphoresVecotr.size();
	submitInfo.pWaitSemaphores		= submitInfoWaitSemaphoresVecotr.data();
	submitInfo.pWaitDstStageMask	= submitInfoWaitDstStageMaskArray;

	submitInfo.commandBufferCount	= 1;	// wait for submitInfoCommandBuffersVecotr to be created
	submitInfo.pCommandBuffers		= &swapchainCommandBufferVector[swapchainImageIndex];

	std::vector<VkSemaphore> submitInfoSignalSemaphoresVector;  
	submitInfoSignalSemaphoresVector.push_back(paintReadyToPresentSemaphore);
	submitInfo.signalSemaphoreCount	= (uint32_t)submitInfoSignalSemaphoresVector.size();
	submitInfo.pSignalSemaphores	= submitInfoSignalSemaphoresVector.data();

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
	presentInfo.sType				= VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount	= (uint32_t)presentInfoWaitSemaphoresVector.size();
	presentInfo.pWaitSemaphores		= presentInfoWaitSemaphoresVector.data();

	VkSwapchainKHR swapChainsArray[] = { swapChain };
	presentInfo.swapchainCount	= 1;
	presentInfo.pSwapchains		= swapChainsArray;
	presentInfo.pImageIndices	= &swapchainImageIndex;

	result = vkQueuePresentKHR(presentQueue, &presentInfo);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
		reCreateTriangleSwapchain();
	}else if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to present swap chain image !!!");
	}
}

void SenAbstractGLFW::createSemaphores() {
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

void SenAbstractGLFW::reCreateTriangleSwapchain()
{
	// Call vkDeviceWaitIdle() here, because we shouldn't touch resources that may still be in use. 
	vkDeviceWaitIdle(device);

	// Have to use this 3 commands to get currentExtent
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);
	if (surfaceCapabilities.currentExtent.width < UINT32_MAX) {
		widgetWidth = surfaceCapabilities.currentExtent.width;
		widgetHeight = surfaceCapabilities.currentExtent.height;
	}else {
		glfwGetWindowSize(widgetGLFW, &widgetWidth, &widgetHeight);
	}

	createSwapchain();
	createSwapchainImageViews();
	createTrianglePipeline();
	createSwapchainFramebuffers();
	createTriangleCommandBuffers();
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
	createLogicalDevice();

	collectSwapchainFeatures();
	createSwapchain();
	createSwapchainImageViews();

	createTriangleRenderPass();
	createDescriptorSetLayout();

	createTrianglePipeline();
	createSwapchainFramebuffers();
	createCommandPool();

	createTriangleVertexBuffer();
	createTriangleIndexBuffer();
	
	createUniformBuffers();
	createDescriptorPool();
	createDescriptorSet();

	createTriangleCommandBuffers();

	createSemaphores();

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

		updateUniformBuffer();

		paintVulkan();
	}

	// All of the operations in drawFrame are asynchronous, which means that when we exit the loop in mainLoop,
	//  drawing and presentation operations may still be going on, and cleaning up resources while that is happening is a bad idea;
	vkDeviceWaitIdle(device);
	// must finalize all objects after corresponding deviceWaitIdle
	finalize();	

	// Terminate GLFW, clearing any resources allocated by GLFW.
	glfwDestroyWindow(widgetGLFW);
	glfwTerminate();
}

// createDescriptorSetLayout() need to be called before createPipeline for the pipelineLayout
void SenAbstractGLFW::createDescriptorSetLayout() {
	VkDescriptorSetLayoutBinding mvpUboDescriptorSetLayoutBinding{};
	mvpUboDescriptorSetLayoutBinding.binding			= 0;
	mvpUboDescriptorSetLayoutBinding.descriptorCount	= 1;
	mvpUboDescriptorSetLayoutBinding.descriptorType		= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	mvpUboDescriptorSetLayoutBinding.pImmutableSamplers = nullptr;
	mvpUboDescriptorSetLayoutBinding.stageFlags			= VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutCreateInfo mvpUboDescriptorSetLayoutCreateInfo{};
	mvpUboDescriptorSetLayoutCreateInfo.sType			= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	mvpUboDescriptorSetLayoutCreateInfo.bindingCount	= 1;
	mvpUboDescriptorSetLayoutCreateInfo.pBindings		= &mvpUboDescriptorSetLayoutBinding;

	SenAbstractGLFW::errorCheck(
		vkCreateDescriptorSetLayout(device, &mvpUboDescriptorSetLayoutCreateInfo, nullptr, &mvpUboDescriptorSetLayout),
		std::string("Fail to Create mvpUboDescriptorSetLayout !")
	);
}

void SenAbstractGLFW::createUniformBuffers() {
	VkDeviceSize mvpUboUniformBufferDeviceSize = sizeof(MvpUniformBufferObject);

	SenAbstractGLFW::createResourceBuffer(device, mvpUboUniformBufferDeviceSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE, physicalDeviceMemoryProperties,
		mvpUniformStagingBuffer, mvpUniformStagingBufferDeviceMemory, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	SenAbstractGLFW::createResourceBuffer(device, mvpUboUniformBufferDeviceSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE, physicalDeviceMemoryProperties,
		mvpOptimalUniformBuffer, mvpOptimalUniformBufferMemory, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

}

void SenAbstractGLFW::createDescriptorPool() {
	VkDescriptorPoolSize descriptorPoolSize{};
	descriptorPoolSize.type				= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorPoolSize.descriptorCount	= 1;

	std::vector<VkDescriptorPoolSize> descriptorPoolSizeVector;
	descriptorPoolSizeVector.push_back(descriptorPoolSize);

	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{};
	descriptorPoolCreateInfo.sType			= VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCreateInfo.poolSizeCount	= descriptorPoolSizeVector.size();
	descriptorPoolCreateInfo.pPoolSizes		= descriptorPoolSizeVector.data();
	descriptorPoolCreateInfo.maxSets		= descriptorPoolSizeVector.size();

	SenAbstractGLFW::errorCheck(
		vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, nullptr, &descriptorPool),
		std::string("Fail to Create descriptorPool !")
	);
}

void SenAbstractGLFW::createDescriptorSet() {
	std::vector<VkDescriptorSetLayout> descriptorSetLayoutVector;
	descriptorSetLayoutVector.push_back(mvpUboDescriptorSetLayout);

	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
	descriptorSetAllocateInfo.sType					= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.descriptorPool		= descriptorPool;
	descriptorSetAllocateInfo.descriptorSetCount	= descriptorSetLayoutVector.size();
	descriptorSetAllocateInfo.pSetLayouts			= descriptorSetLayoutVector.data();

	SenAbstractGLFW::errorCheck(
		vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, &mvpUboDescriptorSet),
		std::string("Fail to Allocate mvpUboDescriptorSet !")
	);

	VkDescriptorBufferInfo mvpDescriptorBufferInfo{};
	mvpDescriptorBufferInfo.buffer	= mvpOptimalUniformBuffer;
	mvpDescriptorBufferInfo.offset	= 0;
	mvpDescriptorBufferInfo.range	= sizeof(MvpUniformBufferObject);

	std::vector<VkDescriptorBufferInfo> descriptorBufferInfoVector;
	descriptorBufferInfoVector.push_back(mvpDescriptorBufferInfo);

	VkWriteDescriptorSet writeDescriptorSet{};
	writeDescriptorSet.sType			= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writeDescriptorSet.dstSet			= mvpUboDescriptorSet;
	writeDescriptorSet.dstBinding		= 0;	// binding number, same as the binding index specified in shader for a given shader stage
	writeDescriptorSet.dstArrayElement	= 0;	// start from the index dstArrayElement of pBufferInfo (descriptorBufferInfoVector)
	writeDescriptorSet.descriptorCount	= descriptorBufferInfoVector.size();// the total number of descriptors to update in pBufferInfo
	writeDescriptorSet.pBufferInfo		= descriptorBufferInfoVector.data();

	vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, nullptr);
}

void SenAbstractGLFW::updateUniformBuffer() {
	static auto startTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();
	int duration = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count() / 1000.0f;

	MvpUniformBufferObject mvpUbo{};
	mvpUbo.model = glm::rotate(glm::mat4(), -duration * glm::radians(15.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	//mvpUbo.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	//mvpUbo.projection = glm::perspective(glm::radians(45.0f), widgetWidth / (float)widgetHeight, 0.1f, 100.0f);
	mvpUbo.projection[1][1] *= -1;

	void* data;
	vkMapMemory(device, mvpUniformStagingBufferDeviceMemory, 0, sizeof(mvpUbo), 0, &data);
	memcpy(data, &mvpUbo, sizeof(mvpUbo));
	vkUnmapMemory(device, mvpUniformStagingBufferDeviceMemory);

	SenAbstractGLFW::transferResourceBuffer(defaultThreadCommandPool, device, graphicsQueue, mvpUniformStagingBuffer,
		mvpOptimalUniformBuffer, sizeof(mvpUbo));
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

	/************************************************************************************************/
	/****************   Clean the old swapChain first, if exist, then assign new   ******************/
	/************************************************************************************************/
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
}

void SenAbstractGLFW::createSwapchainImageViews() {

	for (uint32_t i = 0; i < swapchainImagesCount; ++i) {
		VkImageViewCreateInfo swapchainImageViewCreateInfo{};
		swapchainImageViewCreateInfo.sType			= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		swapchainImageViewCreateInfo.image			= swapchainImagesVector[i];
		swapchainImageViewCreateInfo.viewType		= VK_IMAGE_VIEW_TYPE_2D; // handling 2D image
		swapchainImageViewCreateInfo.format			= surfaceFormat.format;
		swapchainImageViewCreateInfo.components.r	= VK_COMPONENT_SWIZZLE_IDENTITY;
		swapchainImageViewCreateInfo.components.g	= VK_COMPONENT_SWIZZLE_IDENTITY;
		swapchainImageViewCreateInfo.components.b	= VK_COMPONENT_SWIZZLE_IDENTITY;
		swapchainImageViewCreateInfo.components.a	= VK_COMPONENT_SWIZZLE_IDENTITY;
		swapchainImageViewCreateInfo.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT; // color/depth/stencil/metadata
		swapchainImageViewCreateInfo.subresourceRange.baseMipLevel		= 0;
		swapchainImageViewCreateInfo.subresourceRange.levelCount		= 1; // amount of mipmaps
		swapchainImageViewCreateInfo.subresourceRange.baseArrayLayer	= 0;
		swapchainImageViewCreateInfo.subresourceRange.layerCount		= 1; // if larger than 1, .viewType needs to be array

		SenAbstractGLFW::errorCheck(
			vkCreateImageView(device, &swapchainImageViewCreateInfo, nullptr, &swapchainImageViewsVector[i]),
			std::string("Failed to create SwapChan ImageViews !!")
		);
	}
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

void SenAbstractGLFW::createTriangleRenderPass() {
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
		vkCreateRenderPass(device, &triangleRenderPassCreateInfo, nullptr, &triangleRenderPass),
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

void SenAbstractGLFW::createShaderModule(const VkDevice& logicalDevice, const std::vector<char>& SPIRV_Vector, VkShaderModule & targetShaderModule)
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

void SenAbstractGLFW::createTrianglePipeline() {
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
	createShaderModule(device, SenAbstractGLFW::readFileBinaryStream("SenVulkanTutorial/Shaders/triangleVert.spv"), vertShaderModule);
	createShaderModule(device, SenAbstractGLFW::readFileBinaryStream("SenVulkanTutorial/Shaders/triangleFrag.spv"), fragShaderModule);

	VkPipelineShaderStageCreateInfo vertPipelineShaderStageCreateInfo{};
	vertPipelineShaderStageCreateInfo.sType		= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertPipelineShaderStageCreateInfo.stage		= VK_SHADER_STAGE_VERTEX_BIT;
	vertPipelineShaderStageCreateInfo.module	= vertShaderModule;
	vertPipelineShaderStageCreateInfo.pName		= "main"; // shader's entry point name

	VkPipelineShaderStageCreateInfo fragPipelineShaderStageCreateInfo{};
	fragPipelineShaderStageCreateInfo.sType		= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragPipelineShaderStageCreateInfo.stage		= VK_SHADER_STAGE_FRAGMENT_BIT;
	fragPipelineShaderStageCreateInfo.module	= fragShaderModule;
	fragPipelineShaderStageCreateInfo.pName		= "main"; // shader's entry point name

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


	VkVertexInputAttributeDescription positionVertexInputAttributeDescription;
	positionVertexInputAttributeDescription.location	= 0;
	positionVertexInputAttributeDescription.binding		= 0;
	positionVertexInputAttributeDescription.format		= VK_FORMAT_R32G32_SFLOAT;
	positionVertexInputAttributeDescription.offset		= 0;
	VkVertexInputAttributeDescription colorVertexInputAttributeDescription;
	colorVertexInputAttributeDescription.location	= 1;
	colorVertexInputAttributeDescription.binding	= 0;
	colorVertexInputAttributeDescription.format		= VK_FORMAT_R32G32B32_SFLOAT;
	colorVertexInputAttributeDescription.offset		= 2 * sizeof(float);

	std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescriptionVector;
	vertexInputAttributeDescriptionVector.push_back(positionVertexInputAttributeDescription);
	vertexInputAttributeDescriptionVector.push_back(colorVertexInputAttributeDescription);

	VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo{};
	pipelineVertexInputStateCreateInfo.sType							= VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	pipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount	= vertexInputBindingDescriptionVector.size();// spacing between data && whether the data is per-vertex or per-instance (geometry instancing)
	pipelineVertexInputStateCreateInfo.pVertexBindingDescriptions		= vertexInputBindingDescriptionVector.data();
	pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount	= vertexInputAttributeDescriptionVector.size();
	pipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions		= vertexInputAttributeDescriptionVector.data();


	VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo{};
	pipelineInputAssemblyStateCreateInfo.sType							= VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	pipelineInputAssemblyStateCreateInfo.topology						= VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	pipelineInputAssemblyStateCreateInfo.primitiveRestartEnable			= VK_FALSE;
	
	/*********************************************************************************************/
	/*********************************************************************************************/
	VkViewport viewport{};
	viewport.x			= 0.0f;									viewport.y			= 0.0f;
	viewport.width		= static_cast<float>(widgetWidth);		viewport.height		= static_cast<float>(widgetHeight);
	viewport.minDepth	= 0.0f;									viewport.maxDepth	= 1.0f;
	VkRect2D scissorRect2D{};
	scissorRect2D.offset		= { 0, 0 };
	scissorRect2D.extent.width	= static_cast<uint32_t>(widgetWidth);
	scissorRect2D.extent.height	= static_cast<uint32_t>(widgetHeight);

	VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo{};
	pipelineViewportStateCreateInfo.sType				= VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	pipelineViewportStateCreateInfo.viewportCount		= 1;
	pipelineViewportStateCreateInfo.pViewports			= &viewport;
	pipelineViewportStateCreateInfo.scissorCount		= 1;
	pipelineViewportStateCreateInfo.pScissors			= &scissorRect2D;

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
	pipelineMultisampleStateCreateInfo.sType						= VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	pipelineMultisampleStateCreateInfo.sampleShadingEnable			= VK_FALSE;
	pipelineMultisampleStateCreateInfo.rasterizationSamples			= VK_SAMPLE_COUNT_1_BIT;
	
	/*********************************************************************************************/
	/*********************************************************************************************/
	std::vector<VkPipelineColorBlendAttachmentState> pipelineColorBlendAttachmentStateVector; // for multi-framebuffer rendering
	VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState{};
	pipelineColorBlendAttachmentState.colorWriteMask				= VK_COLOR_COMPONENT_R_BIT 	| VK_COLOR_COMPONENT_G_BIT 
																		| VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	pipelineColorBlendAttachmentState.blendEnable					= VK_FALSE;
	pipelineColorBlendAttachmentStateVector.push_back(pipelineColorBlendAttachmentState);

	VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo{};
	pipelineColorBlendStateCreateInfo.sType							= VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	pipelineColorBlendStateCreateInfo.logicOpEnable					= VK_FALSE;
	//pipelineColorBlendStateCreateInfo.logicOp						= VK_LOGIC_OP_COPY;
	pipelineColorBlendStateCreateInfo.attachmentCount				= (uint32_t)pipelineColorBlendAttachmentStateVector.size();
	pipelineColorBlendStateCreateInfo.pAttachments					= pipelineColorBlendAttachmentStateVector.data();
	pipelineColorBlendStateCreateInfo.blendConstants[0]				= 0.0f;
	pipelineColorBlendStateCreateInfo.blendConstants[1]				= 0.0f;
	pipelineColorBlendStateCreateInfo.blendConstants[2]				= 0.0f;
	pipelineColorBlendStateCreateInfo.blendConstants[3]				= 0.0f;

	/****************************************************************************************************************************/
	/**********   Reserve pipeline Layout, which help access to descriptor sets from a pipeline       ***************************/
	/****************************************************************************************************************************/
	std::vector<VkDescriptorSetLayout> descriptorSetLayoutVector;
	descriptorSetLayoutVector.push_back(mvpUboDescriptorSetLayout);

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
	pipelineLayoutCreateInfo.sType			= VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount	= descriptorSetLayoutVector.size();
	pipelineLayoutCreateInfo.pSetLayouts	= descriptorSetLayoutVector.data();


	SenAbstractGLFW::errorCheck(
		vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &trianglePipelineLayout),
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
	trianglePipelineCreateInfo.pVertexInputState	= &pipelineVertexInputStateCreateInfo;
	trianglePipelineCreateInfo.pInputAssemblyState	= &pipelineInputAssemblyStateCreateInfo;
	trianglePipelineCreateInfo.pViewportState		= &pipelineViewportStateCreateInfo;
	trianglePipelineCreateInfo.pRasterizationState	= &pipelineRasterizationStateCreateInfo;
	trianglePipelineCreateInfo.pMultisampleState	= &pipelineMultisampleStateCreateInfo;
	trianglePipelineCreateInfo.pColorBlendState		= &pipelineColorBlendStateCreateInfo;
	trianglePipelineCreateInfo.layout				= trianglePipelineLayout;
	trianglePipelineCreateInfo.renderPass			= triangleRenderPass;
	trianglePipelineCreateInfo.subpass				= 0; // index of this trianglePipeline's subpass of the triangleRenderPass
	//trianglePipelineCreateInfo.basePipelineHandle	= VK_NULL_HANDLE;

	graphicsPipelineCreateInfoVector.push_back(trianglePipelineCreateInfo);

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

void SenAbstractGLFW::createSwapchainFramebuffers() {
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
		framebufferCreateInfo.renderPass		= triangleRenderPass;
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

void SenAbstractGLFW::createCommandPool() {

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

void SenAbstractGLFW::transferResourceBuffer(const VkCommandPool& bufferTransferCommandPool, const VkDevice& logicalDevice, const VkQueue& bufferTransferQueue,
	const VkBuffer& srcBuffer, const VkBuffer& dstBuffer, const VkDeviceSize& resourceBufferSize) {

	VkCommandBuffer bufferCopyCommandBuffer = VK_NULL_HANDLE;
	SenAbstractGLFW::beginSingleTimeCommandBuffer(bufferTransferCommandPool, logicalDevice,	bufferCopyCommandBuffer);

	VkBufferCopy bufferCopyRegion{};
	bufferCopyRegion.size = resourceBufferSize;
	vkCmdCopyBuffer(bufferCopyCommandBuffer, srcBuffer, dstBuffer, 1, &bufferCopyRegion);

	SenAbstractGLFW::endSingleTimeCommandBuffer(bufferTransferCommandPool, logicalDevice, bufferTransferQueue, bufferCopyCommandBuffer);
	bufferCopyCommandBuffer = VK_NULL_HANDLE;
}

void SenAbstractGLFW::createTriangleVertexBuffer() {
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

	SenAbstractGLFW::transferResourceBuffer( defaultThreadCommandPool, device, graphicsQueue, stagingBuffer,
		triangleVertexBuffer, verticesBufferSize);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferDeviceMemory, nullptr);	// always try to destroy before free
}

void SenAbstractGLFW::createTriangleIndexBuffer()
{
	uint16_t indices[] = {	0, 1, 2, 1, 2, 3 };
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
		triangleIndexBuffer, triangleIndexBufferMemory, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	SenAbstractGLFW::transferResourceBuffer(defaultThreadCommandPool, device, graphicsQueue, stagingBuffer, 
		triangleIndexBuffer, indicesBufferSize);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferDeviceMemory, nullptr);	// always try to destroy before free
}

void SenAbstractGLFW::createTriangleCommandBuffers() {
	if (swapchainCommandBufferVector.size() > 0) {
		vkFreeCommandBuffers(device, defaultThreadCommandPool, (uint32_t)swapchainCommandBufferVector.size(), swapchainCommandBufferVector.data());
	}
	/****************************************************************************************************************************/
	/**********           Allocate Swapchain CommandBuffers         *************************************************************/
	/****************************************************************************************************************************/
	swapchainCommandBufferVector.resize(swapchainImagesCount);

	VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
	commandBufferAllocateInfo.sType					= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandPool			= defaultThreadCommandPool;
	commandBufferAllocateInfo.level					= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount	= static_cast<uint32_t>(swapchainCommandBufferVector.size());

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
		renderPassBeginInfo.sType						= VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass					= triangleRenderPass;
		renderPassBeginInfo.framebuffer					= swapchainFramebufferVector[i];
		renderPassBeginInfo.renderArea.offset			= { 0, 0 };
		renderPassBeginInfo.renderArea.extent.width		= widgetWidth;
		renderPassBeginInfo.renderArea.extent.height	= widgetHeight;

		std::vector<VkClearValue> clearValueVector;
		clearValueVector.push_back(VkClearValue{ 0.2f, 0.3f, 0.3f, 1.0f });
		renderPassBeginInfo.clearValueCount = (uint32_t)clearValueVector.size();
		renderPassBeginInfo.pClearValues	= clearValueVector.data();

		vkCmdBeginRenderPass(swapchainCommandBufferVector[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		//======================================================================================
		//======================================================================================
		vkCmdBindPipeline(swapchainCommandBufferVector[i], VK_PIPELINE_BIND_POINT_GRAPHICS, trianglePipeline);
		VkDeviceSize offsetDeviceSize = 0;
		vkCmdBindVertexBuffers(swapchainCommandBufferVector[i], 0, 1, &triangleVertexBuffer, &offsetDeviceSize);

		//vkCmdDraw(
		//	swapchainCommandBufferVector[i],
		//	3, // vertexCount
		//	1, // instanceCount
		//	0, // firstVertex
		//	0  // firstInstance
		//);

		vkCmdBindIndexBuffer(swapchainCommandBufferVector[i], triangleIndexBuffer, 0, VK_INDEX_TYPE_UINT16);

		vkCmdBindDescriptorSets(swapchainCommandBufferVector[i], VK_PIPELINE_BIND_POINT_GRAPHICS, trianglePipelineLayout, 0, 1, &mvpUboDescriptorSet, 0, nullptr);


		vkCmdDrawIndexed(swapchainCommandBufferVector[i], 6, 1, 0, 0, 0);

		vkCmdEndRenderPass(swapchainCommandBufferVector[i]);

		SenAbstractGLFW::errorCheck(
			vkEndCommandBuffer(swapchainCommandBufferVector[i]),
			std::string("Failed to end record of Triangle Swapchain commandBuffers !!!")
		);
	}
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
	/******************************************************************************************************/
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &physicalDeviceMemoryProperties);
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

	SenAbstractGLFW::errorCheck(
		vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device),
		std::string("Fail at Create Logical Device!")
	);

	// Retrieve queue handles for each queue family
	vkGetDeviceQueue(device, graphicsQueueFamilyIndex, 0, &graphicsQueue);
	vkGetDeviceQueue(device, presentQueueFamilyIndex, 0, &presentQueue); // We only need 1 queue, so the third parameter (index) we give is 0.
}

void SenAbstractGLFW::finalize() {
	/************************************************************************************************************/
	/*********************           Destroy defaultThreadCommandPool         ***********************************/
	/************************************************************************************************************/
	if (VK_NULL_HANDLE != defaultThreadCommandPool) {
		vkDestroyCommandPool(device, defaultThreadCommandPool, nullptr);
		defaultThreadCommandPool = VK_NULL_HANDLE;
		
		swapchainCommandBufferVector.clear();
	}
	/************************************************************************************************************/
	/*************      Destroy descriptorPool,  mvpUboDescriptorSetLayout, mvpUboDescriptorSet      ************************/
	/************************************************************************************************************/
	if (VK_NULL_HANDLE != descriptorPool) {
		vkDestroyDescriptorPool(device, descriptorPool, nullptr);
		vkDestroyDescriptorSetLayout(device, mvpUboDescriptorSetLayout, nullptr);

		mvpUboDescriptorSetLayout = VK_NULL_HANDLE;
		descriptorPool = VK_NULL_HANDLE;
		mvpUboDescriptorSet = VK_NULL_HANDLE;
	}
	/************************************************************************************************************/
	/*********************           Destroy Pipeline, PipelineLayout, and RenderPass         *******************/
	/************************************************************************************************************/
	if (VK_NULL_HANDLE != trianglePipeline) {
		vkDestroyPipeline(device, trianglePipeline, nullptr);
		vkDestroyPipelineLayout(device, trianglePipelineLayout, nullptr);
		vkDestroyRenderPass(device, triangleRenderPass, nullptr);

		trianglePipeline		= VK_NULL_HANDLE;
		trianglePipelineLayout	= VK_NULL_HANDLE;
		triangleRenderPass		= VK_NULL_HANDLE;
	}
	/************************************************************************************************************/
	/******************     Destroy VertexBuffer, VertexBufferMemory     ***********************************/
	/************************************************************************************************************/
	if (VK_NULL_HANDLE != triangleVertexBuffer) {
		vkDestroyBuffer(device, triangleVertexBuffer, nullptr);		
		vkFreeMemory(device, triangleVertexBufferMemory, nullptr);	// always try to destroy before free

		triangleVertexBuffer		= VK_NULL_HANDLE;
		triangleVertexBufferMemory	= VK_NULL_HANDLE;
	}
	if (VK_NULL_HANDLE != triangleIndexBuffer) {
		vkDestroyBuffer(device, triangleIndexBuffer, nullptr);
		vkFreeMemory(device, triangleIndexBufferMemory, nullptr);	// always try to destroy before free

		triangleIndexBuffer = VK_NULL_HANDLE;
		triangleIndexBufferMemory = VK_NULL_HANDLE;
	}
	if (VK_NULL_HANDLE != mvpUniformStagingBuffer) {
		vkDestroyBuffer(device, mvpUniformStagingBuffer, nullptr);
		vkFreeMemory(device, mvpUniformStagingBufferDeviceMemory, nullptr);	// always try to destroy before free

		mvpUniformStagingBuffer = VK_NULL_HANDLE;
		mvpUniformStagingBufferDeviceMemory = VK_NULL_HANDLE;
	}
	if (VK_NULL_HANDLE != mvpOptimalUniformBuffer) {
		vkDestroyBuffer(device, mvpOptimalUniformBuffer, nullptr);
		vkFreeMemory(device, mvpOptimalUniformBufferMemory, nullptr);	// always try to destroy before free

		mvpOptimalUniformBuffer = VK_NULL_HANDLE;
		mvpOptimalUniformBufferMemory = VK_NULL_HANDLE;
	}
	/************************************************************************************************************/
	/******************     Destroy depthStencil Memory, ImageView, Image     ***********************************/
	/************************************************************************************************************/
	if (VK_NULL_HANDLE != depthStencilImage) {
		vkDestroyImage(device, depthStencilImage, nullptr);
		vkDestroyImageView(device, depthStencilImageView, nullptr);
		vkFreeMemory(device, depthStencilImageDeviceMemory, nullptr); 	// always try to destroy before free

		depthStencilImage = VK_NULL_HANDLE;
		depthStencilImageDeviceMemory = VK_NULL_HANDLE;
		depthStencilImageView = VK_NULL_HANDLE;
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
	ptrAbstractWidget->reCreateTriangleSwapchain();
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
