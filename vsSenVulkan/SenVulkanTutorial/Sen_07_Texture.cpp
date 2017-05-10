#include "Sen_07_Texture.h"

Sen_07_Texture::Sen_07_Texture()
{
	std::cout << "Constructor: Sen_07_Texture()\n\n";
	strWindowName = "Sen Vulkan Texture Tutorial";

	backgroundTextureDiskAddress = "../Images/SunRaise.jpg";
}


Sen_07_Texture::~Sen_07_Texture()
{
	finalize();

	OutputDebugString("\n\t ~Sen_07_Texture()\n");
}

void Sen_07_Texture::initVulkanApplication()
{
	// Need to be segmented base on pipleStages in this function

	createTriangleRenderPass();
	createTextureAppDescriptorSetLayout();

	createTrianglePipeline();
	createSwapchainFramebuffers();
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

void Sen_07_Texture::reCreateRenderTarget()
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
	createTrianglePipeline();
	createSwapchainFramebuffers();

	createTriangleCommandBuffers();

	std::cout << "\n Finish  Sen_07_Texture::reCreateRenderTarget()\n";
}

void Sen_07_Texture::finalize()
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
		texture2DSampler			= VK_NULL_HANDLE;
	}
	OutputDebugString("\n\tFinish  Sen_07_Texture::finalize()\n");


	// The device and instance will be finalized at the end !!
	SenAbstractGLFW::finalize();
}

void Sen_07_Texture::createTextureAppVertexBuffer()
{
	float vertices[] = {
		// Positions	// Colors
		-0.5f,	-0.5f,	1.0f,	0.0f,	0.0f,	0.0f,	0.0f,	// Bottom Right
		0.5f,	-0.5f,	0.0f,	0.0f,	1.0f,	1.0f,	0.0f,	// Bottom Left
		0.5f,	0.5f,	1.0f,	1.0f,	1.0f,   1.0f,	1.0f,	// Top Right
		-0.5f,	0.5f,	0.0f,	1.0f,	0.0f,	0.0f,	1.0f   // Top Left
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

	std::vector<VkWriteDescriptorSet> DS_Write_Vector;

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
	DS_Write_Vector.push_back(uniformBuffer_DS_Write);

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
	DS_Write_Vector.push_back(uniformBuffer_DS_Write);


	vkUpdateDescriptorSets(device, DS_Write_Vector.size(), DS_Write_Vector.data(), 0, nullptr);
}
