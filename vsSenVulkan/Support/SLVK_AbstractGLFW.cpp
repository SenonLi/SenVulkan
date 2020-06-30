#include "pch.h"
#include "SLVK_AbstractGLFW.h"

// Since stb_image.h header file contains the implementation of functions, only one class source file could include it to make new implementation
// all stb_image realated functions have to be implemented in this class
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <gli/gli.hpp> // to load KTX image file

/****************************************************************************************************************************/
/****************************************************************************************************************************/
/****************************************************************************************************************************/
/***********************************************************************************************************************************/
/*************    Public Static Functions Below      ********************************************************************************/
/*************    Public Static Functions Below      ********************************************************************************/
/*************    Public Static Functions Below      ********************************************************************************/
/*---------------------------------------------------------------------------------------------------------------------------------*/

uint32_t SLVK_AbstractGLFW::findPhysicalDeviceMemoryPropertyIndex(
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
}

void SLVK_AbstractGLFW::errorCheck(VkResult result, std::string msg)
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
			//case VK_ERROR_INVALID_EXTERNAL_HANDLE_KHX:
			//	errString = "VK_ERROR_INVALID_EXTERNAL_HANDLE_KHX    \n";
			//	break;
		default:
			std::cout << result << std::endl;
			errString = "\n\n Cannot tell error type, check std::cout!    \n";
			break;
		}

		throw std::runtime_error(msg + "\t" + errString);
	}
}


void SLVK_AbstractGLFW::createVulkanShaderModule(const VkDevice& logicalDevice, const std::string& diskFileAddress, VkShaderModule& shaderModule) {
	std::vector<char> spirvCharVector;// (spirv32Vector.size() * sizeof(uint32_t) / sizeof(char));

	if (diskFileAddress.substr(diskFileAddress.length() - 4, 4).compare(".spv") == 0)
		spirvCharVector = SLVK_AbstractGLFW::readFileStream(diskFileAddress, true);
	else {
		std::string shaderTypeString = diskFileAddress.substr(diskFileAddress.length() - 5, 5);
		shaderc_shader_kind shadercType;
		if (shaderTypeString.compare(".vert") == 0)		shadercType = shaderc_glsl_vertex_shader;
		else if (shaderTypeString.compare(".frag") == 0)		shadercType = shaderc_glsl_fragment_shader;
		else if (shaderTypeString.compare(".geom") == 0)		shadercType = shaderc_glsl_geometry_shader;
		else assert(false);
		// Android system Attension:   sourceString size for shadercToSPIRV() below may change.
		std::vector<uint32_t> spirv32Vector = SLVK_AbstractGLFW::shadercToSPIRV("glShaderSrc", shadercType, SLVK_AbstractGLFW::readFileStream(diskFileAddress).data());
		spirvCharVector.resize(spirv32Vector.size() * sizeof(uint32_t) / sizeof(char));
		memcpy(spirvCharVector.data(), spirv32Vector.data(), spirvCharVector.size());
	}

	createShaderModuleFromSPIRV(logicalDevice, spirvCharVector, shaderModule);
}

/*---------------------------------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------------------------------*/
const std::vector<VkFormat> SLVK_AbstractGLFW::depthStencilSupportCheckFormatsVector = {
	VK_FORMAT_D16_UNORM,
	VK_FORMAT_D32_SFLOAT,
	VK_FORMAT_X8_D24_UNORM_PACK32,
	VK_FORMAT_S8_UINT,
	VK_FORMAT_D16_UNORM_S8_UINT,
	VK_FORMAT_D24_UNORM_S8_UINT,
	VK_FORMAT_D32_SFLOAT_S8_UINT
};

bool SLVK_AbstractGLFW::hasStencilComponent(VkFormat formatToCheck)	{
	return	formatToCheck == VK_FORMAT_D32_SFLOAT_S8_UINT ||
		formatToCheck == VK_FORMAT_D24_UNORM_S8_UINT ||
		formatToCheck == VK_FORMAT_D16_UNORM_S8_UINT ||
		formatToCheck == VK_FORMAT_S8_UINT;
}

/*---------------------------------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------------------------------*/
void SLVK_AbstractGLFW::beginSingleTimeCommandBuffer(const VkCommandPool& tmpCommandBufferCommandPool, const VkDevice& logicalDevice,
	VkCommandBuffer& tempCommandBufferToBegin) {
	// You may wish to create a separate command pool for these kinds of short - lived buffers, 
	// because the implementation may be able to apply memory allocation optimizations.
	// You should use the VK_COMMAND_POOL_CREATE_TRANSIENT_BIT flag during command pool generation in that case.

	VkCommandBufferAllocateInfo tmpCommandBufferAllocateInfo{};
	tmpCommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	tmpCommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	tmpCommandBufferAllocateInfo.commandPool = tmpCommandBufferCommandPool;
	tmpCommandBufferAllocateInfo.commandBufferCount = 1;
	vkAllocateCommandBuffers(logicalDevice, &tmpCommandBufferAllocateInfo, &tempCommandBufferToBegin);

	VkCommandBufferBeginInfo tmpCommandBufferBeginInfo{};
	tmpCommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	tmpCommandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(tempCommandBufferToBegin, &tmpCommandBufferBeginInfo);
}

void SLVK_AbstractGLFW::endSingleTimeCommandBuffer(const VkCommandPool& tmpCommandBufferCommandPool, const VkDevice& logicalDevice
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

/*---------------------------------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------------------------------*/
void SLVK_AbstractGLFW::createResourceBuffer(const VkDevice& logicalDevice, const VkDeviceSize& bufferDeviceSize,
	const VkBufferUsageFlags& bufferUsageFlags, const VkSharingMode& bufferSharingMode, const VkPhysicalDeviceMemoryProperties& gpuMemoryProperties,
	VkBuffer& bufferToCreate, VkDeviceMemory& bufferDeviceMemoryToAllocate, const VkMemoryPropertyFlags& requiredMemoryPropertyFlags) {
	/*****************************************************************************************************************************************************/
	VkBufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = bufferDeviceSize;
	bufferCreateInfo.usage = bufferUsageFlags;
	bufferCreateInfo.sharingMode = bufferSharingMode;

	SLVK_AbstractGLFW::errorCheck(
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
		= SLVK_AbstractGLFW::findPhysicalDeviceMemoryPropertyIndex(gpuMemoryProperties, bufferMemoryRequirements, requiredMemoryPropertyFlags);

	SLVK_AbstractGLFW::errorCheck(
		vkAllocateMemory(logicalDevice, &bufferMemoryAllocateInfo, nullptr, &bufferDeviceMemoryToAllocate),
		std::string("Failed to allocate m_TriangleVertexBufferMemory !!!")
	);

	vkBindBufferMemory(logicalDevice, bufferToCreate, bufferDeviceMemoryToAllocate, 0);
}

void SLVK_AbstractGLFW::transferResourceBuffer(const VkCommandPool& bufferTransferCommandPool, const VkDevice& logicalDevice, const VkQueue& bufferMemoryTransferQueue,
	const VkBuffer& srcBuffer, const VkBuffer& dstBuffer, const VkDeviceSize& resourceBufferSize) {

	VkCommandBuffer bufferCopyCommandBuffer = VK_NULL_HANDLE;
	SLVK_AbstractGLFW::beginSingleTimeCommandBuffer(bufferTransferCommandPool, logicalDevice, bufferCopyCommandBuffer);

	VkBufferCopy bufferCopyRegion{};
	bufferCopyRegion.size = resourceBufferSize;
	vkCmdCopyBuffer(bufferCopyCommandBuffer, srcBuffer, dstBuffer, 1, &bufferCopyRegion);

	SLVK_AbstractGLFW::endSingleTimeCommandBuffer(bufferTransferCommandPool, logicalDevice, bufferMemoryTransferQueue, bufferCopyCommandBuffer);
	bufferCopyCommandBuffer = VK_NULL_HANDLE;
}

/*---------------------------------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------------------------------*/
void SLVK_AbstractGLFW::createResourceImage(const VkDevice& logicalDevice,const uint32_t& imageWidth, const uint32_t& imageHeight
	,const VkImageType& imageType, const VkFormat& imageFormat, const VkImageTiling& imageTiling, const VkImageUsageFlags& imageUsageFlags
	,VkImage& imageToCreate, VkDeviceMemory& imageDeviceMemoryToAllocate, const VkMemoryPropertyFlags& requiredMemoryPropertyFlags
	,const VkSharingMode& imageSharingMode, const VkPhysicalDeviceMemoryProperties& gpuMemoryProperties, const uint32_t& layerCount = 1)
{
	/***********************************************************************************************************************************************/
	/*************    VK_IMAGE_TILING_LINEAR  have further restrictions on their limits and capabilities    ****************************************/
	if (imageTiling == VK_IMAGE_TILING_LINEAR) {
		for (auto posibleDepthStencilFormat : SLVK_AbstractGLFW::depthStencilSupportCheckFormatsVector) {
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
	imageCreateInfo.arrayLayers		= layerCount; // 1 if not working as ImageArray; Spec: must be greater than 0; have to be 1 if LINEAR format
	imageCreateInfo.format			= imageFormat;
	imageCreateInfo.tiling			= imageTiling;
	// An initially undefined layout is for images used as attachments (color/depth) that will probably be cleared by a render pass before use;
	// If you want to fill it with data, like a texture, then you should use the preinitialized layout.
	imageCreateInfo.initialLayout	= VK_IMAGE_LAYOUT_PREINITIALIZED;
	imageCreateInfo.usage			= imageUsageFlags;
	imageCreateInfo.samples			= VK_SAMPLE_COUNT_1_BIT;	// Need to fix this if using multisampling
	imageCreateInfo.sharingMode		= imageSharingMode;			// When being attachments and multiple QueueFamilies require

	SLVK_AbstractGLFW::errorCheck(
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
		= SLVK_AbstractGLFW::findPhysicalDeviceMemoryPropertyIndex(gpuMemoryProperties, imageMemoryRequirements, requiredMemoryPropertyFlags);

	SLVK_AbstractGLFW::errorCheck(
		vkAllocateMemory(logicalDevice, &imageMemoryAllocateInfo, nullptr, &imageDeviceMemoryToAllocate),
		std::string("Failed to allocate reource image memory !!!")
	);

	vkBindImageMemory(logicalDevice, imageToCreate, imageDeviceMemoryToAllocate, 0);
}

void SLVK_AbstractGLFW::transitionResourceImageLayout(const VkImage& imageToTransitionLayout
	,const VkImageSubresourceRange& imageSubresourceRangeToTransition, const VkImageLayout& oldImageLayout, const VkImageLayout& newImageLayout
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

	if		(oldImageLayout == VK_IMAGE_LAYOUT_PREINITIALIZED && newImageLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;	// the first access scope includes command buffer submission (HOST_WRITE)
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	}else if (oldImageLayout == VK_IMAGE_LAYOUT_PREINITIALIZED && newImageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;	// the first access scope includes command buffer submission (HOST_WRITE)
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	}else if (oldImageLayout == VK_IMAGE_LAYOUT_PREINITIALIZED && newImageLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		imageMemoryBarrier.srcAccessMask = 0;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	}else if (oldImageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newImageLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	}else {
		throw std::invalid_argument("unsupported layout transition!");
	}

	VkCommandBuffer transitionImageLayoutCommandBuffer = VK_NULL_HANDLE;
	SLVK_AbstractGLFW::beginSingleTimeCommandBuffer(transitionImageLayoutCommandPool, logicalDevice, transitionImageLayoutCommandBuffer);
	vkCmdPipelineBarrier(
		transitionImageLayoutCommandBuffer,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,		// srcStageMask
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,		// dstStageMask
		0,						// dependencyFlags, framebuffer-global instead of framebuffer-local 
		0, nullptr,	0, nullptr, // VkMemoryBarrier or VkBufferMemoryBarrirer related
		1,						// imageMemoryBarrierCount
		&imageMemoryBarrier		// pImageMemoryBarriers
	);
	SLVK_AbstractGLFW::endSingleTimeCommandBuffer(transitionImageLayoutCommandPool, logicalDevice, imageMemoryTransferQueue, transitionImageLayoutCommandBuffer);
	transitionImageLayoutCommandBuffer = VK_NULL_HANDLE;
}

void SLVK_AbstractGLFW::transferResourceBufferToImage(const VkCommandPool& bufferToImageCommandPool, const VkQueue& bufferToImageTransferQueue
	, const VkDevice& logicalDevice, const VkBuffer& srcBuffer, const VkImage& dstImage, const uint32_t& imageWidth, const uint32_t& imageHeight
	, const uint32_t& layerCount = 1, const uint32_t& regionCount = 1, const VkBufferImageCopy* ptrBufferImageCopyRegionVec = nullptr) {

	VkCommandBuffer bufferToImageCommandBuffer = VK_NULL_HANDLE;
	SLVK_AbstractGLFW::beginSingleTimeCommandBuffer(bufferToImageCommandPool, logicalDevice, bufferToImageCommandBuffer);
	if (layerCount == 1) {
		VkBufferImageCopy bufferImageCopyRegion{};
		bufferImageCopyRegion.bufferRowLength = 0; // Means no padding bytes between rows of the image.
		bufferImageCopyRegion.bufferImageHeight = 0;
		bufferImageCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		bufferImageCopyRegion.imageSubresource.mipLevel = 0; // the mipmap level to copy from; not sure why there's no mipCount
		bufferImageCopyRegion.imageSubresource.baseArrayLayer = 0;
		bufferImageCopyRegion.imageSubresource.layerCount = 1;
		bufferImageCopyRegion.imageExtent = { imageWidth, imageHeight, 1 };
		// It's possible to specify an array of VkBufferImageCopy to perform many different copies from this buffer to the image in one operation.
		vkCmdCopyBufferToImage(bufferToImageCommandBuffer, srcBuffer, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferImageCopyRegion);
	}
	else
		vkCmdCopyBufferToImage(bufferToImageCommandBuffer, srcBuffer, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			regionCount, ptrBufferImageCopyRegionVec);

	SLVK_AbstractGLFW::endSingleTimeCommandBuffer(bufferToImageCommandPool, logicalDevice, bufferToImageTransferQueue, bufferToImageCommandBuffer);
	bufferToImageCommandBuffer = VK_NULL_HANDLE;
}

void SLVK_AbstractGLFW::createDeviceLocalTexture(const VkDevice& logicalDevice, const VkPhysicalDeviceMemoryProperties& gpuMemoryProperties
	,const char*& textureDiskAddress, const VkImageType& imageType,  int& textureWidth, int& textureHeight
	,VkImage& deviceLocalTextureToCreate, VkDeviceMemory& textureDeviceMemoryToAllocate, VkImageView& textureImageViewToCreate
	,const VkSharingMode& imageSharingMode, const VkCommandPool& tmpCommandBufferCommandPool, const VkQueue& imageMemoryTransferQueue)
{
	bool usingGliLibrary = false;
	if (std::string(textureDiskAddress).substr(std::string(textureDiskAddress).length() - 4, 4).compare(".ktx") == 0)
		usingGliLibrary = true;
	stbi_uc* ptrDiskTextureToUpload = nullptr;
	gli::texture2d tex2D;
	/*****************************************************************************************************************************************/
	if (usingGliLibrary) {
		tex2D = gli::texture2d(gli::load(textureDiskAddress));
		assert(!tex2D.empty());
		if (tex2D.empty()) { throw std::runtime_error("failed to load texture2D KTX image!"); }
		textureWidth = static_cast<uint32_t>(tex2D[0].extent().x);
		textureHeight = static_cast<uint32_t>(tex2D[0].extent().y);
	}else	{
		// The pointer ptrBackgroundTexture returned from stbi_load(...) is the first element in an array of pixel values.
		int actuallyTextureChannels	= 0;
		ptrDiskTextureToUpload = stbi_load(textureDiskAddress, &textureWidth, &textureHeight, &actuallyTextureChannels, STBI_rgb_alpha);
		if (!ptrDiskTextureToUpload) {
			throw std::runtime_error("failed to load texture image!");
		}
	}
	/***********************************************************************************************************************************************/
	/*************   Obsoleted   ( First:   Upload/MapMemory texture image file as linear stagingImage )      **************************************/
	//VkImage linearStagingImage;
	//VkDeviceMemory linearStagingImageDeviceMemory;
	//SLVK_AbstractGLFW::createResourceImage(logicalDevice, textureWidth, textureHeight, imageType,
	//	VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_LINEAR, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, linearStagingImage, linearStagingImageDeviceMemory,
	//	VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, imageSharingMode, gpuMemoryProperties);

	//// The graphics card may assume that one row of pixels is not imageWidth * 4 bytes wide, but rather texWidth * 4 + paddingBytes;
	//// To handle this for memcpy correctly, we need to query how bytes are arranged in our staging image using vkGetImageSubresourceLayout.
	//VkImageSubresource textureImageSubresource{};
	//textureImageSubresource.aspectMask	= VK_IMAGE_ASPECT_COLOR_BIT; // bitMask of color/depth/stencil/metadata
	//textureImageSubresource.mipLevel	= 0; // subresource's index of mipLevel, 0 means the first mipLevel, only 0 for a texture
	//textureImageSubresource.arrayLayer	= 0; // subresource's index of arrayLayer, 0 means the first arrayLayer, only 0 if not Textures (ArrayImage)

	//VkSubresourceLayout linearStagingImageSubresourceLayout;
	//vkGetImageSubresourceLayout(logicalDevice, linearStagingImage, &textureImageSubresource, &linearStagingImageSubresourceLayout);

	//void* ptrHostVisibleData;
	//VkDeviceSize hostVisibleImageDeviceSize = linearStagingImageSubresourceLayout.rowPitch * textureHeight ;
	//vkMapMemory(logicalDevice, linearStagingImageDeviceMemory, 0, hostVisibleImageDeviceSize, 0, &ptrHostVisibleData);

	//if (linearStagingImageSubresourceLayout.rowPitch == textureWidth * 4) {  // Channel == 4 due to STBI_rgb_alpha 
	//	// No padding bytes in rows if in this case; Usually with rowPitch == a power-of-2 size, e.g. 512 or 1024
	//	memcpy(ptrHostVisibleData, ptrDiskTextureToUpload, (size_t)hostVisibleImageDeviceSize);
	//}else	{
	//	// otherwise, have to copy the pixels row-by-row with the right offset based on SubresourceLayout.rowPitch
	//	uint8_t* ptrHostVisibleDataBytes = reinterpret_cast<uint8_t*>(ptrHostVisibleData);
	//	for (int row = 0; row < textureHeight; row++) {
	//		memcpy(	&ptrHostVisibleDataBytes[row * linearStagingImageSubresourceLayout.rowPitch],
	//				&ptrDiskTextureToUpload	[row * textureWidth * 4],
	//				textureWidth * 4);
	//	}
	//}

	//vkUnmapMemory(logicalDevice, linearStagingImageDeviceMemory);
	//stbi_image_free(ptrDiskTextureToUpload);
	/***********************************************************************************************************************************************/
	/**********     Obsoleted     ( Second: Transfer stagingImage to deviceLocalTextureImage with correct textureImageLayout )     *****************/
	//SLVK_AbstractGLFW::createResourceImage(logicalDevice, textureWidth, textureHeight, imageType,
	//	VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, deviceLocalTextureToCreate
	//	, textureDeviceMemoryToAllocate, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, imageSharingMode, gpuMemoryProperties);
	//
	//VkImageSubresourceRange textureImageSubresourceRange{};
	//textureImageSubresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
	//textureImageSubresourceRange.baseMipLevel	= 0;	// first mipMap level to start
	//textureImageSubresourceRange.levelCount		= 1;
	//textureImageSubresourceRange.baseArrayLayer	= 0;	// first arrayLayer to start
	//textureImageSubresourceRange.layerCount		= 1;

	//SLVK_AbstractGLFW::transitionResourceImageLayout(linearStagingImage, textureImageSubresourceRange, VK_IMAGE_LAYOUT_PREINITIALIZED,
	//	VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_FORMAT_R8G8B8A8_UNORM, logicalDevice, tmpCommandBufferCommandPool, imageMemoryTransferQueue);
	//SLVK_AbstractGLFW::transitionResourceImageLayout(deviceLocalTextureToCreate, textureImageSubresourceRange, VK_IMAGE_LAYOUT_PREINITIALIZED,
	//	VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_FORMAT_R8G8B8A8_UNORM, logicalDevice, tmpCommandBufferCommandPool, imageMemoryTransferQueue);

	//SLVK_AbstractGLFW::transferResourceImage(tmpCommandBufferCommandPool, logicalDevice, imageMemoryTransferQueue,
	//	linearStagingImage, deviceLocalTextureToCreate, textureWidth, textureHeight);
	//SLVK_AbstractGLFW::transitionResourceImageLayout(deviceLocalTextureToCreate, textureImageSubresourceRange, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	//	VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_FORMAT_R8G8B8A8_UNORM, logicalDevice, tmpCommandBufferCommandPool, imageMemoryTransferQueue);
	/***********************************************************************************************************************************************/
	/**********     Obsoleted    ( Third:  clean the staging Image, DeviceMemory )         *********************************************************/
	//vkDestroyImage(logicalDevice, linearStagingImage, nullptr);
	//vkFreeMemory(logicalDevice, linearStagingImageDeviceMemory, nullptr); 	// always try to destroy before free
	//linearStagingImage				= VK_NULL_HANDLE;
	//linearStagingImageDeviceMemory	= VK_NULL_HANDLE;
	
	/***********************************************************************************************************************************************/
	/*************      First:   Upload/MapMemory texture image file to texture StagingBuffer )         ********************************************/
	VkBuffer textureStagingBuffer;
	VkDeviceMemory textureStagingBufferDeviceMemory;
	VkDeviceSize hostVisibleTextureDeviceSize;
	if (usingGliLibrary)
			hostVisibleTextureDeviceSize = tex2D.size();
	else	hostVisibleTextureDeviceSize = textureWidth * textureHeight * 4;

	SLVK_AbstractGLFW::createResourceBuffer(logicalDevice, hostVisibleTextureDeviceSize, // 4 for RGBA
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE, gpuMemoryProperties,
		textureStagingBuffer, textureStagingBufferDeviceMemory, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	void* ptrHostVisibleData;
	vkMapMemory(logicalDevice, textureStagingBufferDeviceMemory, 0, hostVisibleTextureDeviceSize, 0, &ptrHostVisibleData);

	if (usingGliLibrary) {
		memcpy(ptrHostVisibleData, tex2D.data(), static_cast<size_t>(hostVisibleTextureDeviceSize));
	}else	{
		memcpy(ptrHostVisibleData, ptrDiskTextureToUpload, static_cast<size_t>(hostVisibleTextureDeviceSize));
		stbi_image_free(ptrDiskTextureToUpload);
	}
	vkUnmapMemory(logicalDevice, textureStagingBufferDeviceMemory);
	/***********************************************************************************************************************************************/
	/**********        Second: Transfer stagingImage to deviceLocalTextureImage with correct textureImageLayout )        ***************************/
	VkFormat textureFormat;
	if (usingGliLibrary) {
		gli::texture::format_type  formatType = tex2D.format();
		switch (formatType)
		{
			case gli::texture::format_type::FORMAT_RGBA_BP_UNORM_BLOCK16:
				textureFormat = VkFormat::VK_FORMAT_BC7_UNORM_BLOCK;			break;
			case gli::texture::format_type::FORMAT_RGBA_DXT3_UNORM_BLOCK16:
				textureFormat = VkFormat::VK_FORMAT_BC2_UNORM_BLOCK;			break;
			case gli::texture::format_type::FORMAT_RG_ATI2N_UNORM_BLOCK16:
				textureFormat = VkFormat::VK_FORMAT_BC5_UNORM_BLOCK;			break;
			case gli::texture::format_type::FORMAT_R_ATI1N_UNORM_BLOCK8:
				textureFormat = VkFormat::VK_FORMAT_BC4_UNORM_BLOCK;			break;
			case gli::texture::format_type::FORMAT_RGBA_DXT5_UNORM_BLOCK16:
				textureFormat = VkFormat::VK_FORMAT_BC3_UNORM_BLOCK;			break;
			case gli::texture::format_type::FORMAT_RGBA_DXT1_UNORM_BLOCK8:
				textureFormat = VkFormat::VK_FORMAT_BC1_RGBA_UNORM_BLOCK;		break;
			case gli::texture::format_type::FORMAT_R_EAC_UNORM_BLOCK8:
				textureFormat = VkFormat::VK_FORMAT_EAC_R11_UNORM_BLOCK;		break;

			default:
				throw std::runtime_error("May not support this KTX image format, check it out !!!");		break;
		}
	}
	else	textureFormat = VK_FORMAT_R8G8B8A8_UNORM;

	SLVK_AbstractGLFW::createResourceImage(logicalDevice, textureWidth, textureHeight, imageType,
		textureFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, deviceLocalTextureToCreate
		, textureDeviceMemoryToAllocate, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, imageSharingMode, gpuMemoryProperties);

	VkImageSubresourceRange textureImageSubresourceRange{};
	textureImageSubresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
	textureImageSubresourceRange.baseMipLevel	= 0;	// first mipMap level to start
	textureImageSubresourceRange.levelCount		= 1;
	textureImageSubresourceRange.baseArrayLayer	= 0;	// first arrayLayer to start
	textureImageSubresourceRange.layerCount		= 1;

	SLVK_AbstractGLFW::transitionResourceImageLayout(deviceLocalTextureToCreate, textureImageSubresourceRange, VK_IMAGE_LAYOUT_PREINITIALIZED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, logicalDevice, tmpCommandBufferCommandPool, imageMemoryTransferQueue);
	SLVK_AbstractGLFW::transferResourceBufferToImage(tmpCommandBufferCommandPool, imageMemoryTransferQueue,
		logicalDevice, textureStagingBuffer, deviceLocalTextureToCreate, textureWidth, textureHeight);
	SLVK_AbstractGLFW::transitionResourceImageLayout(deviceLocalTextureToCreate, textureImageSubresourceRange, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, logicalDevice, tmpCommandBufferCommandPool, imageMemoryTransferQueue);

	/***********************************************************************************************************************************************/
	/**********            Third:  clean the staging Buffer, DeviceMemory                 ***********************************************************/
	vkDestroyBuffer(logicalDevice, textureStagingBuffer, nullptr);
	vkFreeMemory(logicalDevice, textureStagingBufferDeviceMemory, nullptr);
	textureStagingBuffer				= VK_NULL_HANDLE;
	textureStagingBufferDeviceMemory	= VK_NULL_HANDLE;

	/***********************************************************************************************************************************************/
	/****************          Fourth:  create textureImageView       ******************************************************************************/
	VkImageViewCreateInfo textureImageViewCreateInfo{};
	textureImageViewCreateInfo.sType	= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	textureImageViewCreateInfo.image	= deviceLocalTextureToCreate;

	if (imageType == VK_IMAGE_TYPE_2D)
		textureImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	else 	throw std::runtime_error("No supported VkImageViewType set up for this imageType, Check it out !!!!\n");

	textureImageViewCreateInfo.format	= textureFormat;
	textureImageViewCreateInfo.subresourceRange = textureImageSubresourceRange;

	SLVK_AbstractGLFW::errorCheck(
		vkCreateImageView(logicalDevice, &textureImageViewCreateInfo, nullptr, &textureImageViewToCreate),
		std::string("Failed to create Resource Image View !!!")
	);
}

void SLVK_AbstractGLFW::createTextureSampler(const VkDevice& logicalDevice, VkSampler& textureSamplerToCreate) {
	VkSamplerCreateInfo textureSamplerCreateInfo{};
	textureSamplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	textureSamplerCreateInfo.magFilter = VK_FILTER_LINEAR; // oversampling
	textureSamplerCreateInfo.minFilter = VK_FILTER_LINEAR;	// undersampling
	textureSamplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	textureSamplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	textureSamplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	textureSamplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	textureSamplerCreateInfo.anisotropyEnable = VK_TRUE;
	textureSamplerCreateInfo.maxAnisotropy = 16.0; // physicalDeviceProperties.limits.maxSamplerAnisotropy
	textureSamplerCreateInfo.compareEnable = VK_FALSE; // for shadow maps (percentage-closer filtering)
	textureSamplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	textureSamplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	textureSamplerCreateInfo.unnormalizedCoordinates = VK_FALSE;

	SLVK_AbstractGLFW::errorCheck(
		vkCreateSampler(logicalDevice, &textureSamplerCreateInfo, nullptr, &textureSamplerToCreate),
		std::string("Failed to create texture sampler !!!")
	);
}


void SLVK_AbstractGLFW::transferResourceImage(const VkCommandPool& imageTransferCommandPool, const VkDevice& logicalDevice, const VkQueue& imageTransferQueue,
	const VkImage& srcImage, const VkImage& dstImage, const uint32_t& imageWidth, const uint32_t& imageHeight) {

	VkCommandBuffer imageCopyCommandBuffer = VK_NULL_HANDLE;
	SLVK_AbstractGLFW::beginSingleTimeCommandBuffer(imageTransferCommandPool, logicalDevice, imageCopyCommandBuffer);

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

	SLVK_AbstractGLFW::endSingleTimeCommandBuffer(imageTransferCommandPool, logicalDevice, imageTransferQueue, imageCopyCommandBuffer);
	imageCopyCommandBuffer = VK_NULL_HANDLE;
}

void SLVK_AbstractGLFW::createDeviceLocalTextureArray(const VkDevice& logicalDevice, const VkPhysicalDeviceMemoryProperties& gpuMemoryProperties
	, const std::vector<std::string> & texturesDiskAddressVector, const VkImageType& imageType
	, VkImage& deviceLocalTextureToCreate, VkDeviceMemory& textureDeviceMemoryToAllocate, VkImageView& textureImageViewToCreate
	, const VkSharingMode& imageSharingMode, const VkCommandPool& tmpCommandBufferCommandPool, const VkQueue& imageMemoryTransferQueue)
{
	bool usingGliLibrary = false;
	if (texturesDiskAddressVector.size() == 1
		&& texturesDiskAddressVector[0].substr(texturesDiskAddressVector[0].length() - 4, 4).compare(".ktx") == 0)
		usingGliLibrary = true;
	std::vector<stbi_uc*> ptrDiskTexToUploadVector;
	gli::texture2d_array tex2DArray;
	int textureArrayLayerCount, maxTextureWidth = 0, minTextureWidth = 999, maxTextureHeight = 0, minTextureHeight = 999;
	std::vector<VkDeviceSize> hostVisibleTexDeviceSizeVector;
	std::vector<int> textureWidthVector, textureHeightVector;
	VkDeviceSize totalHostVisibleTexDeviceSize = 0;
	/*****************************************************************************************************************************************/
	if (usingGliLibrary) {
		tex2DArray = gli::texture2d_array(gli::load(texturesDiskAddressVector[0]));
		assert(!tex2DArray.empty());
		if (tex2DArray.empty()) { throw std::runtime_error("failed to load texture2DArray KTX image!"); }
		maxTextureWidth = static_cast<uint32_t>(tex2DArray[0].extent().x);
		maxTextureHeight = static_cast<uint32_t>(tex2DArray[0].extent().y);
		textureArrayLayerCount = static_cast<int>(tex2DArray.layers());
		totalHostVisibleTexDeviceSize = tex2DArray.size();
	}
	else {
		textureArrayLayerCount = static_cast<int>(texturesDiskAddressVector.size());
		textureWidthVector.resize(textureArrayLayerCount);
		textureHeightVector.resize(textureArrayLayerCount);
		// The pointer ptrBackgroundTexture returned from stbi_load(...) is the first element in an array of pixel values.
		int actuallyTextureChannels;
		for (int i = 0; i < textureArrayLayerCount; i++) {
			stbi_uc* ptrDiskTextureToUpload = stbi_load(texturesDiskAddressVector[i].c_str(), &textureWidthVector[i], &textureHeightVector[i], &actuallyTextureChannels, STBI_rgb_alpha);
			if (!ptrDiskTextureToUpload) { throw std::runtime_error("failed to load one of the texture2DArray images!"); }
			ptrDiskTexToUploadVector.push_back(ptrDiskTextureToUpload);
			hostVisibleTexDeviceSizeVector.push_back(textureWidthVector[i] * textureHeightVector[i] * 4);
			totalHostVisibleTexDeviceSize += hostVisibleTexDeviceSizeVector[i];

			maxTextureWidth = maxTextureWidth > textureWidthVector[i] ? maxTextureWidth : textureWidthVector[i];
			minTextureWidth = minTextureWidth < textureWidthVector[i] ? minTextureWidth : textureWidthVector[i];
			maxTextureHeight = maxTextureHeight > textureHeightVector[i] ? maxTextureHeight : textureHeightVector[i];
			minTextureHeight = minTextureHeight < textureHeightVector[i] ? minTextureHeight : textureHeightVector[i];
		}
	}

	/***********************************************************************************************************************************************/
	/*************      First:   Upload/MapMemory texture image file to texture StagingBuffer )         ********************************************/
	VkBuffer textureStagingBuffer;
	VkDeviceMemory textureStagingBufferDeviceMemory;

	SLVK_AbstractGLFW::createResourceBuffer(logicalDevice, totalHostVisibleTexDeviceSize, // 4 for RGBA
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE, gpuMemoryProperties,
		textureStagingBuffer, textureStagingBufferDeviceMemory, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	void* ptrHostVisibleData;
	vkMapMemory(logicalDevice, textureStagingBufferDeviceMemory, 0, totalHostVisibleTexDeviceSize, 0, &ptrHostVisibleData);

	if (usingGliLibrary) {
		memcpy(ptrHostVisibleData, tex2DArray.data(), static_cast<size_t>(totalHostVisibleTexDeviceSize));
	}
	else {
		int offset = 0;
		for (int i = 0; i < textureArrayLayerCount; i++) {
			memcpy(static_cast<char*>(ptrHostVisibleData) + offset, ptrDiskTexToUploadVector[i], static_cast<size_t>(hostVisibleTexDeviceSizeVector[i]));
			stbi_image_free(ptrDiskTexToUploadVector[i]);
			offset += static_cast<int>(hostVisibleTexDeviceSizeVector[i]);
		}
	}
	vkUnmapMemory(logicalDevice, textureStagingBufferDeviceMemory);
	/***********************************************************************************************************************************************/
	/**********        Second: Transfer stagingImage to deviceLocalTextureImage with correct textureImageLayout )        ***************************/
	VkFormat textureFormat;
	if (usingGliLibrary) {
		gli::texture::format_type  formatType = tex2DArray.format();
		switch (formatType)
		{
		case gli::texture::format_type::FORMAT_RGBA_BP_UNORM_BLOCK16:
			textureFormat = VkFormat::VK_FORMAT_BC7_UNORM_BLOCK;			break;
		case gli::texture::format_type::FORMAT_RGBA_DXT3_UNORM_BLOCK16:
			textureFormat = VkFormat::VK_FORMAT_BC2_UNORM_BLOCK;			break;
		case gli::texture::format_type::FORMAT_RG_ATI2N_UNORM_BLOCK16:
			textureFormat = VkFormat::VK_FORMAT_BC5_UNORM_BLOCK;			break;
		case gli::texture::format_type::FORMAT_R_ATI1N_UNORM_BLOCK8:
			textureFormat = VkFormat::VK_FORMAT_BC4_UNORM_BLOCK;			break;
		case gli::texture::format_type::FORMAT_RGBA_DXT5_UNORM_BLOCK16:
			textureFormat = VkFormat::VK_FORMAT_BC3_UNORM_BLOCK;			break;
		case gli::texture::format_type::FORMAT_RGBA_DXT1_UNORM_BLOCK8:
			textureFormat = VkFormat::VK_FORMAT_BC1_RGBA_UNORM_BLOCK;		break;
		case gli::texture::format_type::FORMAT_R_EAC_UNORM_BLOCK8:
			textureFormat = VkFormat::VK_FORMAT_EAC_R11_UNORM_BLOCK;		break;
		default:
			throw std::runtime_error("May not support this KTX image format, check it out !!!");		break;
		}
	}
	else	textureFormat = VK_FORMAT_R8G8B8A8_UNORM;

	SLVK_AbstractGLFW::createResourceImage(logicalDevice, maxTextureWidth, maxTextureHeight, imageType,
		textureFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, deviceLocalTextureToCreate
		, textureDeviceMemoryToAllocate, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, imageSharingMode, gpuMemoryProperties, textureArrayLayerCount);

	VkImageSubresourceRange textureImageSubresourceRange{};
	textureImageSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	textureImageSubresourceRange.baseMipLevel = 0;	// first mipMap level to start
	textureImageSubresourceRange.levelCount = 1;
	textureImageSubresourceRange.baseArrayLayer = 0;	// first arrayLayer to start
	textureImageSubresourceRange.layerCount = textureArrayLayerCount;

	SLVK_AbstractGLFW::transitionResourceImageLayout(deviceLocalTextureToCreate, textureImageSubresourceRange, VK_IMAGE_LAYOUT_PREINITIALIZED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, logicalDevice, tmpCommandBufferCommandPool, imageMemoryTransferQueue);

	/******************************************************************************************************/
	/**********       Setup buffer copy regions for array layers      *************************************/
	std::vector<VkBufferImageCopy> bufferImageCopyRegionsVector;

	VkBufferImageCopy sameDemensionBufferImageCopyRegion{};
	sameDemensionBufferImageCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	sameDemensionBufferImageCopyRegion.imageSubresource.mipLevel = 0;
	sameDemensionBufferImageCopyRegion.imageSubresource.baseArrayLayer = 0;
	sameDemensionBufferImageCopyRegion.imageSubresource.layerCount = textureArrayLayerCount;
	sameDemensionBufferImageCopyRegion.imageExtent.width = maxTextureWidth;
	sameDemensionBufferImageCopyRegion.imageExtent.height = maxTextureHeight;
	sameDemensionBufferImageCopyRegion.imageExtent.depth = 1;

	if (usingGliLibrary) {
		// assume same dimensions for all layers
		bufferImageCopyRegionsVector.push_back(sameDemensionBufferImageCopyRegion);
	}
	else {
		// Check if all array layers have the same dimension
		if (maxTextureWidth == minTextureWidth && maxTextureHeight == minTextureHeight) {
			bufferImageCopyRegionsVector.push_back(sameDemensionBufferImageCopyRegion);
		}
		else { // not same dimension
			uint32_t offset = 0;
			// If dimensions differ, copy layer by layer and pass offsets
			for (int layerIndex = 0; layerIndex < textureArrayLayerCount; layerIndex++) {
				VkBufferImageCopy bufferImageCopyRegion{};
				bufferImageCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				bufferImageCopyRegion.imageSubresource.mipLevel = 0;
				bufferImageCopyRegion.imageSubresource.baseArrayLayer = layerIndex;
				bufferImageCopyRegion.imageSubresource.layerCount = 1;
				bufferImageCopyRegion.imageExtent.width = textureWidthVector[layerIndex];
				bufferImageCopyRegion.imageExtent.height = textureHeightVector[layerIndex];
				bufferImageCopyRegion.imageExtent.depth = 1;
				bufferImageCopyRegion.bufferOffset = offset;

				bufferImageCopyRegionsVector.push_back(bufferImageCopyRegion);
				offset += static_cast<int>(hostVisibleTexDeviceSizeVector[layerIndex]);
			}
		}
	}

	SLVK_AbstractGLFW::transferResourceBufferToImage(tmpCommandBufferCommandPool, imageMemoryTransferQueue,
		logicalDevice, textureStagingBuffer, deviceLocalTextureToCreate, static_cast<uint32_t>(maxTextureWidth)
		, static_cast<uint32_t>(maxTextureHeight), static_cast<uint32_t>(textureArrayLayerCount)
		, static_cast<uint32_t>(bufferImageCopyRegionsVector.size()), bufferImageCopyRegionsVector.data());

	SLVK_AbstractGLFW::transitionResourceImageLayout(deviceLocalTextureToCreate, textureImageSubresourceRange, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, logicalDevice, tmpCommandBufferCommandPool, imageMemoryTransferQueue);

	/***********************************************************************************************************************************************/
	/**********            Third:  clean the staging Buffer, DeviceMemory                 ***********************************************************/
	vkDestroyBuffer(logicalDevice, textureStagingBuffer, nullptr);
	vkFreeMemory(logicalDevice, textureStagingBufferDeviceMemory, nullptr);
	textureStagingBuffer = VK_NULL_HANDLE;
	textureStagingBufferDeviceMemory = VK_NULL_HANDLE;

	/***********************************************************************************************************************************************/
	/****************          Fourth:  create textureImageView       ******************************************************************************/
	VkImageViewCreateInfo textureImageViewCreateInfo{};
	textureImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	textureImageViewCreateInfo.image = deviceLocalTextureToCreate;

	if (imageType == VK_IMAGE_TYPE_2D)
		textureImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
	else 	throw std::runtime_error("No supported VkImageViewType set up for this imageType, Check it out !!!!\n");

	textureImageViewCreateInfo.format = textureFormat;
	textureImageViewCreateInfo.subresourceRange = textureImageSubresourceRange;

	SLVK_AbstractGLFW::errorCheck(
		vkCreateImageView(logicalDevice, &textureImageViewCreateInfo, nullptr, &textureImageViewToCreate),
		std::string("Failed to create Resource Image View !!!")
	);
}

/*---------------------------------------------------------------------------------------------------------------------------------*/
/*************    Public Static Functions Above      ********************************************************************************/
/*************    Public Static Functions Above      ********************************************************************************/
/*************    Public Static Functions Above      ********************************************************************************/
/***********************************************************************************************************************************/
/***********************************************************************************************************************************/
/***********************************************************************************************************************************/
/***********************************************************************************************************************************/

SLVK_AbstractGLFW::SLVK_AbstractGLFW()
{
	std::cout << "\nConstructor: SLVK_AbstractGLFW()\n";
	//showAllSupportedInstanceExtensions(); // Not Useful Functions
	//showAllSupportedInstanceLayers(); // Not Useful Functions
	//showAllSupportedExtensionsEachUnderInstanceLayer(); // Not Useful Functions

	m_WidgetWidth = DEFAULT_widgetWidth;
	m_WidgetHeight = DEFAULT_widgetHeight;

	strWindowName = "SL GLFW Vulkan Application";
}

SLVK_AbstractGLFW::~SLVK_AbstractGLFW()
{
	OutputDebugString("\n\t ~SLVK_AbstractGLFW()\n");
}

void SLVK_AbstractGLFW::showWidget()
{
	initGlfwVulkanDebugWSI();
	initVulkanApplication();
	// Game loop
	while (!glfwWindowShouldClose(widgetGLFW))
	{
		// Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
		glfwPollEvents();

		updateUniformBuffer();

		swapSwapchain();
	}

	// All of the operations in drawFrame are asynchronous, which means that when we exit the loop in mainLoop,
	//  drawing and presentation operations may still be going on, and cleaning up resources while that is happening is a bad idea;
	vkDeviceWaitIdle(m_LogicalDevice);
	// must finalize all objects after corresponding deviceWaitIdle
	finalizeWidget();
	finalizeAbstractGLFW();
	// Terminate GLFW, clearing any resources allocated by GLFW.
	glfwDestroyWindow(widgetGLFW);
	glfwTerminate();
}

/***********************************************************************************************************************************/
/***********************************************************************************************************************************/
/*************    Protected Functions       ********************************************************************************/
/*---------------------------------------------------------------------------------------------------------------------------------*/

void SLVK_AbstractGLFW::createColorAttachOnlyRenderPass() {
	/********************************************************************************************************************/
	/************    Setting AttachmentDescription:  Only colorAttachment is needed for Triangle      *******************/
	/********************************************************************************************************************/
	std::array<VkAttachmentDescription, 1> attachmentDescriptionArray{};
	attachmentDescriptionArray[0].format = m_SurfaceFormat.format;// swapChainImageFormat;
	attachmentDescriptionArray[0].samples = VK_SAMPLE_COUNT_1_BIT; // Not using multi-sampling
	attachmentDescriptionArray[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescriptionArray[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescriptionArray[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescriptionArray[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescriptionArray[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;       // layout before renderPass
	attachmentDescriptionArray[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // auto transition after renderPass

	/********************************************************************************************************************/
	/********    Setting Subpasses with Dependencies: One subpass is enough to paint the triangle     *******************/
	/********************************************************************************************************************/
	std::array<VkAttachmentReference, 1> colorAttachmentReferenceArray{};
	colorAttachmentReferenceArray[0].attachment = 0; // The colorAttachment index is 0
	colorAttachmentReferenceArray[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // auto transition during renderPass

	std::array<VkSubpassDescription, 1> subpassDescriptionArray{};
	subpassDescriptionArray[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescriptionArray[0].colorAttachmentCount = (uint32_t)colorAttachmentReferenceArray.size();	// Every subpass references one or more attachments
	subpassDescriptionArray[0].pColorAttachments = colorAttachmentReferenceArray.data();

	/******* There are two built-in dependencies that take care of the transition at the start of the render pass and at the end of the render pass,
	/////       but the former does not occur at the right time.
	/////       It assumes that the transition occurs at the start of the pipeline, but we haven't acquired the image yet at that point! ****/
	/******* There are two ways to deal with this problem.
	/////       We could change the waitStages for the imageAvailableSemaphore to VK_PIPELINE_STAGE_TOP_OF_PIPELINE_BIT
	/////        to ensure that the render passes don't begin until the image is available,
	/////       or we can make the render pass wait for the VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT stage. ****************************/
	std::vector<VkSubpassDependency> subpassDependencyVector;
	VkSubpassDependency headSubpassDependency{};
	headSubpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;		// subpassIndex, from external
	headSubpassDependency.dstSubpass = 0;						// subpassIndex, to the first subpass, which is also the only one
	headSubpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // specify the operations to wait on and the stages in which these operations occur.
	headSubpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	headSubpassDependency.srcAccessMask = 0;											 // specify the operations to wait on and the stages in which these operations occur.
	headSubpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT
		| VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	subpassDependencyVector.push_back(headSubpassDependency);

	/********************************************************************************************************************/
	/*********************    Create RenderPass for rendering       *****************************************************/
	/********************************************************************************************************************/
	VkRenderPassCreateInfo colorAttachOnlyRenderPassCreateInfo{};
	colorAttachOnlyRenderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	colorAttachOnlyRenderPassCreateInfo.attachmentCount = (uint32_t)attachmentDescriptionArray.size();
	colorAttachOnlyRenderPassCreateInfo.pAttachments = attachmentDescriptionArray.data();
	colorAttachOnlyRenderPassCreateInfo.subpassCount = (uint32_t)subpassDescriptionArray.size();
	colorAttachOnlyRenderPassCreateInfo.pSubpasses = subpassDescriptionArray.data();
	colorAttachOnlyRenderPassCreateInfo.dependencyCount = (uint32_t)subpassDependencyVector.size();
	colorAttachOnlyRenderPassCreateInfo.pDependencies = subpassDependencyVector.data();

	SLVK_AbstractGLFW::errorCheck(
		vkCreateRenderPass(m_LogicalDevice, &colorAttachOnlyRenderPassCreateInfo, nullptr, &m_ColorAttachOnlyRenderPass),
		std::string("Failed to create render pass !!")
	);
}

void SLVK_AbstractGLFW::createColorAttachOnlySwapchainFramebuffers() {
	/************************************************************************************************************/
	/*****************     Destroy old swapchainFramebuffers first, if there are      ***************************/
	/************************************************************************************************************/
	for (auto swapchainFramebuffer : m_SwapchainFramebufferVector) {
		vkDestroyFramebuffer(m_LogicalDevice, swapchainFramebuffer, nullptr);
	}
	m_SwapchainFramebufferVector.clear();
	m_SwapchainFramebufferVector.resize(m_SwapChain_ImagesCount);

	for (size_t i = 0; i < m_SwapChain_ImagesCount; i++) {

		std::array<VkImageView, 1> imageViewAttachmentArray{};
		imageViewAttachmentArray[0] = m_SwapchainImageViewsVector[i];

		VkFramebufferCreateInfo framebufferCreateInfo{};
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.renderPass = m_ColorAttachOnlyRenderPass;
		framebufferCreateInfo.attachmentCount = (uint32_t)imageViewAttachmentArray.size();
		framebufferCreateInfo.pAttachments = imageViewAttachmentArray.data();
		framebufferCreateInfo.width = m_WidgetWidth;
		framebufferCreateInfo.height = m_WidgetHeight;
		framebufferCreateInfo.layers = 1;

		SLVK_AbstractGLFW::errorCheck(
			vkCreateFramebuffer(m_LogicalDevice, &framebufferCreateInfo, nullptr, &m_SwapchainFramebufferVector[i]),
			std::string("Failed to create framebuffer !!!")
		);
	}
}

void SLVK_AbstractGLFW::createDefaultCommandPool() {

	VkCommandPoolCreateInfo commandPoolCreateInfo{};
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // allow commandBuffer to be individually reset
	commandPoolCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex;

	SLVK_AbstractGLFW::errorCheck(
		vkCreateCommandPool(m_LogicalDevice, &commandPoolCreateInfo, nullptr, &m_DefaultThreadCommandPool),
		std::string("Failed to create m_DefaultThreadCommandPool !!!")
	);

}

void SLVK_AbstractGLFW::createSingleRectIndexBuffer()
{
	uint16_t indices[] = { 0, 1, 2, 1, 2, 3 };
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
	/***************   Transfer from stagingBuffer to Optimal m_TriangleVertexBuffer   ********************************************************************/
	SLVK_AbstractGLFW::createResourceBuffer(m_LogicalDevice, indicesBufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE, m_PhysicalDeviceMemoryProperties,
		singleRectIndexBuffer, singleRectIndexBufferMemory, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	SLVK_AbstractGLFW::transferResourceBuffer(m_DefaultThreadCommandPool, m_LogicalDevice, m_GraphicsQueue, stagingBuffer,
		singleRectIndexBuffer, indicesBufferSize);

	vkDestroyBuffer(m_LogicalDevice, stagingBuffer, nullptr);
	vkFreeMemory(m_LogicalDevice, stagingBufferDeviceMemory, nullptr);	// always try to destroy before free
}

/****************************************************************************************************************************/
/****************************************************************************************************************************/
/****************************************************************************************************************************/
/***********************************************************************************************************************************/
/*************    Private Static Functions Below      ********************************************************************************/
/*************    Private Static Functions Below      ********************************************************************************/
/*************    Private Static Functions Below      ********************************************************************************/
/*---------------------------------------------------------------------------------------------------------------------------------*/
void SLVK_AbstractGLFW::onWidgetResized(GLFWwindow* widget, int width, int height) {
	if (width == 0 || height == 0) return;

	SLVK_AbstractGLFW* ptrAbstractWidget = reinterpret_cast<SLVK_AbstractGLFW*>(glfwGetWindowUserPointer(widget));
	ptrAbstractWidget->reInitPresentation();
	ptrAbstractWidget->reCreateRenderTarget();
}

void SLVK_AbstractGLFW::onKeyboardReaction(GLFWwindow* widget, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(widget, VK_TRUE);
}

void SLVK_AbstractGLFW::onKeyboardDetected(GLFWwindow* widget, int key, int scancode, int action, int mode)
{
	SLVK_AbstractGLFW* ptrAbstractWidget = reinterpret_cast<SLVK_AbstractGLFW*>(glfwGetWindowUserPointer(widget));
	ptrAbstractWidget->onKeyboardReaction(widget, key, scancode, action, mode);
}

VKAPI_ATTR VkBool32 VKAPI_CALL SLVK_AbstractGLFW::pfnDebugCallback(
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

/*---------------------------------------------------------------------------------------------------------------------------------*/
std::vector<char> SLVK_AbstractGLFW::readFileStream(const std::string& diskFileAddress, bool binary) {
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

void SLVK_AbstractGLFW::createShaderModuleFromSPIRV(const VkDevice& logicalDevice, const std::vector<char>& SPIRV_Vector, VkShaderModule & targetShaderModule)
{
	VkShaderModuleCreateInfo shaderModuleCreateInfo{};
	shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderModuleCreateInfo.codeSize = SPIRV_Vector.size();
	shaderModuleCreateInfo.pCode = (uint32_t*)SPIRV_Vector.data();

	SLVK_AbstractGLFW::errorCheck(
		vkCreateShaderModule(logicalDevice, &shaderModuleCreateInfo, nullptr, &targetShaderModule),
		std::string("Failed to create the shader module !!")
	);
}

// Compiles a shader to a SPIR-V binary. Returns the binary as a vector of 32-bit words.
std::vector<uint32_t> SLVK_AbstractGLFW::shadercToSPIRV(const std::string& source_name,
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

/*---------------------------------------------------------------------------------------------------------------------------------*/
/*************    Private Static Functions Above      ********************************************************************************/
/*************    Private Static Functions Above      ********************************************************************************/
/*************    Private Static Functions Above      ********************************************************************************/
/***********************************************************************************************************************************/
/***********************************************************************************************************************************/
/***********************************************************************************************************************************/
/***********************************************************************************************************************************/

/****************************************************************************************************************************/
/****************************************************************************************************************************/
/****************************************************************************************************************************/
/***********************************************************************************************************************************/
/*************         Private Functions Below      ********************************************************************************/
/*************         Private Functions Below      ********************************************************************************/
/*************         Private Functions Below      ********************************************************************************/
/*---------------------------------------------------------------------------------------------------------------------------------*/

void SLVK_AbstractGLFW::initGlfwVulkanDebugWSI()
{
	// Init GLFW
	glfwInit();
	// Set all the required options for GLFW
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); //tell GLFW to not create an OpenGL context 
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	// Create a GLFWwindow object that we can use for GLFW's functions
	widgetGLFW = glfwCreateWindow(m_WidgetWidth, m_WidgetHeight, strWindowName, nullptr, nullptr);
	glfwSetWindowPos(widgetGLFW, 400, 240);
	glfwMakeContextCurrent(widgetGLFW);

	// GLFW allows us to store an arbitrary pointer in the window object with glfwSetWindowUserPointer,
	//   so we can specify a static class member and get the original class instance back with glfwGetWindowUserPointer;
	// We can then proceed to call recreateSwapChain, but only if the size of the window is non - zero;
	//   This case occurs when the window is minimized and it will cause swap chain creation to fail.
	glfwSetWindowUserPointer(widgetGLFW, this);
	glfwSetWindowSizeCallback(widgetGLFW, SLVK_AbstractGLFW::onWidgetResized);
	glfwSetKeyCallback(widgetGLFW, SLVK_AbstractGLFW::onKeyboardDetected);

	/*****************************************************************************************************************************/
	// Set the required callback functions
	if (DEBUG_LAYERS_ENABLED) {
		initDebugLayers();
	}
	initExtensions();
	createInstance();
	if (DEBUG_LAYERS_ENABLED) {
		initDebugReportCallback(); // Need created Instance
	}

	/*******************************************************************************************************************************/
	/********* The window surface needs to be created right after the instance creation, *******************************************/
	/********* because the check of "surface" support will influence the physical m_LogicalDevice selection.     ****************************/
	createSurface(); // m_Surface == default framebuffer to draw
	pickPhysicalDevice();
	//showPhysicalDeviceSupportedLayersAndExtensions(m_PhysicalDevice);// only show m_PhysicalDevice after pickPhysicalDevice()
	createDefaultLogicalDevice();
	collectSwapchainFeatures();
	createSwapchain();
	createSynchronizationPrimitives(); // has to be after createSwapchain() for the correct m_SwapChain_ImagesCount

	std::cout << "\n Finish  SLVK_AbstractGLFW::initGlfwVulkanDebugWSI()\n";
}

/*******************************************************************
* 1. Call initDebugLayers() before createInstance() to track the instance creation procedure.
* Sum:
*********************************************************************/
void SLVK_AbstractGLFW::initDebugLayers()
{
	// choose layers
	debugInstanceLayersVector.push_back("VK_LAYER_LUNARG_standard_validation");
	debugDeviceLayersVector.push_back("VK_LAYER_LUNARG_standard_validation");	// depricated, but still recommended

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


void SLVK_AbstractGLFW::initExtensions()
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
	if (DEBUG_LAYERS_ENABLED) {
		debugInstanceExtensionsVector.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	}

	/*****************************************************************************************************************************/
	/*************  For Physical Device Extensions  ******************************************************************************/
	/*****************************************************************************************************************************/
	debugDeviceExtensionsVector.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME); //There is no quote here
}

/*******************************************************************
* 1. vkCreateInstance requires VkInstanceCreateInfo
* 2. VkInstanceCreateInfo requires VkApplicationInfo, setup of extensions (instance and m_LogicalDevice) and layers
* Sum: createInstance requires basic appInfo, InstanceExtensionInfo, DeviceExtensionInfo and LayerInfo
*********************************************************************/
void SLVK_AbstractGLFW::createInstance()
{
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "SL Vulkan";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "SLVK Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);

	VkInstanceCreateInfo instanceCreateInfo = {};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pApplicationInfo = &appInfo;

	instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(debugInstanceExtensionsVector.size());
	instanceCreateInfo.ppEnabledExtensionNames = debugInstanceExtensionsVector.data();

	if (DEBUG_LAYERS_ENABLED) {
		instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(debugInstanceLayersVector.size());
		instanceCreateInfo.ppEnabledLayerNames = debugInstanceLayersVector.data();
		instanceCreateInfo.pNext = &debugReportCallbackCreateInfo;
	}

	SLVK_AbstractGLFW::errorCheck(
		vkCreateInstance(&instanceCreateInfo, nullptr, &instance),
		std::string("Failed to create instance! \t Error:\t")
	);
}

void SLVK_AbstractGLFW::initDebugReportCallback()
{
	fetch_vkCreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT"));
	fetch_vkDestroyDebugReportCallbackEXT = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT"));

	if (nullptr == fetch_vkCreateDebugReportCallbackEXT || nullptr == fetch_vkDestroyDebugReportCallbackEXT) {
		throw std::runtime_error("Vulkan Error: Can't fetch debug function pointers.");
	}

	SLVK_AbstractGLFW::errorCheck(
		fetch_vkCreateDebugReportCallbackEXT(instance, &debugReportCallbackCreateInfo, nullptr, &debugReportCallback),
		std::string("Create debugReportCallback Error!")
	);
}

/*---------------------------------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------------------------------*/
void SLVK_AbstractGLFW::createSurface()
{
	SLVK_AbstractGLFW::errorCheck(
		glfwCreateWindowSurface(instance, widgetGLFW, nullptr, &m_Surface),
		std::string("Failed to create window m_Surface!")
	);
}

void SLVK_AbstractGLFW::showPhysicalDeviceInfo(const VkPhysicalDevice & gpuToCheck)
{
	if (VK_NULL_HANDLE == gpuToCheck) 		throw std::runtime_error("Wrong GPU!");

	VkPhysicalDeviceProperties physicalDeviceProperties{};
	vkGetPhysicalDeviceProperties(gpuToCheck, &physicalDeviceProperties);
	std::ostringstream stream;
	stream << "\tGPU Name: [" << physicalDeviceProperties.deviceName << "]\t\tType: \t\"";
	switch (physicalDeviceProperties.deviceType) {
	case 0:			stream << "VK_PHYSICAL_DEVICE_TYPE_OTHER\"\n";				break;
	case 1:			stream << "VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU\"\n";	break;
	case 2:			stream << "VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU\"\n";		break;
	case 3:			stream << "VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU\"\n";		break;
	case 4:			stream << "VK_PHYSICAL_DEVICE_TYPE_CPU\"\n";				break;
	default:		stream << "Unrecognized GPU Property.deviceType! \n";		break;
	}
	std::cout << stream.str();
}

bool SLVK_AbstractGLFW::isPhysicalDeviceSuitable(const VkPhysicalDevice& gpuToCheck, int32_t& graphicsQueueIndex, int32_t& presentQueueIndex)
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
			vkGetPhysicalDeviceSurfaceSupportKHR(gpuToCheck, i, m_Surface, &presentSupport);
			if (presentSupport) {
				presentQueueIndex = i;
			}
		}

		if (graphicsQueueIndex >= 0 && presentQueueIndex >= 0)
			break;
	}

	return graphicsQueueIndex >= 0 && presentQueueIndex >= 0;
}

int SLVK_AbstractGLFW::ratePhysicalDevice(const VkPhysicalDevice & gpuToCheck, int32_t& graphicsQueueIndex, int32_t& presentQueueIndex)
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

	//	1. Get the number of Queues supported by the PhysicalDevice
	//	2. Get the properties each Queue type or Queue Family
	//			There could be 4 Queue type or Queue families supported by PhysicalDevice - 
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
			vkGetPhysicalDeviceSurfaceSupportKHR(gpuToCheck, i, m_Surface, &presentSupport);
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
		score += 100000;	// Discrete GPUs have a significant performance advantage
	}
	score += physicalDeviceProperties.limits.maxImageDimension2D;// Maximum possible size of textures affects graphics quality

	return score;
}

void SLVK_AbstractGLFW::pickPhysicalDevice()
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
	//		m_PhysicalDevice = detectedGPU;
	//		break;
	//	}
	//}

	//if (VK_NULL_HANDLE == m_PhysicalDevice) {
	//	throw std::runtime_error("failed to find a suitable GPU!");
	//}

	/******************************************************************************************************/
	/******** Rate all available PhysicalDevices and  Pick the best suitable one  *************************/
	/******** Use an ordered map to automatically sort candidates by increasing score *********************/

	std::multimap<int, VkPhysicalDevice> physicalDevicesScoredMap;
	std::cout << "All Detected GPUs Properties: \n";
	for (int i = 0; i < physicalDevicesVector.size(); i++) {
		int score = ratePhysicalDevice(physicalDevicesVector[i], graphicsQueueFamilyIndex, presentQueueFamilyIndex);// Primary check function in this block
		showPhysicalDeviceInfo(physicalDevicesVector[i]);
		physicalDevicesScoredMap.insert(std::make_pair(score, physicalDevicesVector[i]));
		std::cout << "\t\t\t\tGraphics QueueFamily Index = \t" << graphicsQueueFamilyIndex << std::endl;
		std::cout << "\t\t\t\tPresent  QueueFamily Index = \t" << presentQueueFamilyIndex << std::endl;
		std::cout << "\t\t\t\tRated Score = \t\t" << score << std::endl;
	}
	// Check if the best candidate is suitable at all
	if (physicalDevicesScoredMap.rbegin()->first > 0) {
		m_PhysicalDevice = physicalDevicesScoredMap.rbegin()->second;
	}
	else {
		throw std::runtime_error("failed to find a suitable GPU!");
	}
	/******************************************************************************************************/
	/*****  Acquire the m_PhysicalDeviceMemoryProperties of the selected GPU   *****/
	vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &m_PhysicalDeviceMemoryProperties);
	std::cout << "\n\nSelected GPU Properties:\n";	showPhysicalDeviceInfo(m_PhysicalDevice);
	std::cout << std::endl;
}

void SLVK_AbstractGLFW::createDefaultLogicalDevice()
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

	VkPhysicalDeviceFeatures			physicalDeviceFeatures{};
	vkGetPhysicalDeviceFeatures(m_PhysicalDevice, &physicalDeviceFeatures);

	VkDeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(deviceQueuesCreateInfosVector.size());
	deviceCreateInfo.pQueueCreateInfos = deviceQueuesCreateInfosVector.data();
	deviceCreateInfo.pEnabledFeatures = &physicalDeviceFeatures;

	if (DEBUG_LAYERS_ENABLED) {
		deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(debugDeviceLayersVector.size());   // depricated
		deviceCreateInfo.ppEnabledLayerNames = debugDeviceLayersVector.data();				// depricated
	}
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(debugDeviceExtensionsVector.size());
	deviceCreateInfo.ppEnabledExtensionNames = debugDeviceExtensionsVector.data();

	SLVK_AbstractGLFW::errorCheck(
		vkCreateDevice(m_PhysicalDevice, &deviceCreateInfo, nullptr, &m_LogicalDevice),
		std::string("Fail at Create Logical Device!")
	);

	// Retrieve queue handles for each queue family
	vkGetDeviceQueue(m_LogicalDevice, graphicsQueueFamilyIndex, 0, &m_GraphicsQueue);
	vkGetDeviceQueue(m_LogicalDevice, presentQueueFamilyIndex, 0, &m_SwapchainPresentQueue); // We only need 1 queue, so the third parameter (index) we give is 0.
}

/*---------------------------------------------------------------------------------------------------------------------------------*/
void SLVK_AbstractGLFW::collectSwapchainFeatures()
{
	/****************************************************************************************************************************/
	/********** Getting Surface Capabilities first to support SwapChain. ********************************************************/
	/*********** Could not do this right after surface creation, because GPU had not been seleted at that time ******************/
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_PhysicalDevice, m_Surface, &m_SurfaceCapabilities);

	// Do the check & assignment below because what we surface size we got may not equal to what we set
	// Make sure the size of swapchain match the size of surface
	if (m_SurfaceCapabilities.currentExtent.width < UINT32_MAX) {
		m_WidgetWidth = m_SurfaceCapabilities.currentExtent.width;
		m_WidgetHeight = m_SurfaceCapabilities.currentExtent.height;
	}
	else {
		m_WidgetWidth = (std::max)(m_SurfaceCapabilities.minImageExtent.width, (std::min)(m_SurfaceCapabilities.maxImageExtent.width, static_cast<uint32_t>(m_WidgetWidth)));
		m_WidgetHeight = (std::max)(m_SurfaceCapabilities.minImageExtent.height, (std::min)(m_SurfaceCapabilities.maxImageExtent.height, static_cast<uint32_t>(m_WidgetHeight)));
	}

	/****************************************************************************************************************************/
	/**************************** Reserve swapchain minImageCount ***************************************************************/
	/****************************************************************************************************************************/
	// For best performance, possibly at the price of some latency, the minImageCount should be set to at least 3 if supported;
	// maxImageCount can actually be zero in which case the amount of swapchain images do not have an upper limit other than available memory. 
	// It's also possible that the swapchain image amount is locked to a certain value on certain systems. The code below takes into consideration both of these possibilities.
	if (m_SwapChain_ImagesCount < m_SurfaceCapabilities.minImageCount + 1) m_SwapChain_ImagesCount = m_SurfaceCapabilities.minImageCount + 1;
	if (m_SurfaceCapabilities.maxImageCount > 0) {
		if (m_SwapChain_ImagesCount > m_SurfaceCapabilities.maxImageCount) m_SwapChain_ImagesCount = m_SurfaceCapabilities.maxImageCount;
	}

	/****************************************************************************************************************************/
	/********** Reserve swapchain imageFormat and imageColorSpace ***************************************************************/
	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(m_PhysicalDevice, m_Surface, &formatCount, nullptr);
	if (formatCount == 0) {
		throw std::runtime_error("No SurfaceFormat found, not a suitable GPU!");
	}
	std::vector<VkSurfaceFormatKHR> surfaceFormatVector;
	surfaceFormatVector.resize(formatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(m_PhysicalDevice, m_Surface, &formatCount, surfaceFormatVector.data());
	if (surfaceFormatVector[0].format == VK_FORMAT_UNDEFINED) { // the prasentation layer (WSI) doesnot care about the format
		m_SurfaceFormat.format = VK_FORMAT_B8G8R8A8_UNORM;
		m_SurfaceFormat.colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
	}
	else {
		m_SurfaceFormat = surfaceFormatVector[0];
	}

	/****************************************************************************************************************************/
	/**********                Reserve m_SwapchainPresentMode                ***************************************************************/
	/****************************************************************************************************************************/
	uint32_t presentModeCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(m_PhysicalDevice, m_Surface, &presentModeCount, nullptr);
	std::vector<VkPresentModeKHR> presentModeVector(presentModeCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(m_PhysicalDevice, m_Surface, &presentModeCount, presentModeVector.data());
	for (auto m : presentModeVector) {
		// VK_PRESENT_MODE_MAILBOX_KHR is good for gaming, but can only get full advantage of MailBox PresentMode with more than 2 buffers,
		// which means triple-buffering
		if (m == VK_PRESENT_MODE_MAILBOX_KHR) {
			m_SwapchainPresentMode = m;
			break;
		}
	}
}

void SLVK_AbstractGLFW::createSwapchain() {
	VkSwapchainCreateInfoKHR swapchainCreateInfo{};
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.surface = m_Surface;
	swapchainCreateInfo.minImageCount = m_SwapChain_ImagesCount; // This is only to set the min value, instead of the actual imageCount after swapchain creation
	swapchainCreateInfo.imageFormat = m_SurfaceFormat.format;
	swapchainCreateInfo.imageColorSpace = m_SurfaceFormat.colorSpace;
	swapchainCreateInfo.imageExtent.width = m_WidgetWidth;
	swapchainCreateInfo.imageExtent.height = m_WidgetHeight;
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
		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;  // share between QueueFamilies or not
		swapchainCreateInfo.queueFamilyIndexCount = 0;// no QueueFamily share
		swapchainCreateInfo.pQueueFamilyIndices = nullptr;
	}

	swapchainCreateInfo.preTransform = m_SurfaceCapabilities.currentTransform; // Rotate of Mirror before presentation (VkSurfaceTransformFlagBitsKHR)
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfo.presentMode = m_SwapchainPresentMode;
	swapchainCreateInfo.clipped = VK_TRUE;	// Typically always set this true, such that Vulkan never render the invisible (out of visible range) image 

	swapchainCreateInfo.oldSwapchain = m_SwapChain;// resize window
	VkSwapchainKHR newSwapChain;
	SLVK_AbstractGLFW::errorCheck(
		vkCreateSwapchainKHR(m_LogicalDevice, &swapchainCreateInfo, nullptr, &newSwapChain),
		std::string("Fail to Create SwapChain !")
	);
	m_SwapChain = newSwapChain;

	// Get actual amount/count of swapchain images
	vkGetSwapchainImagesKHR(m_LogicalDevice, m_SwapChain, &m_SwapChain_ImagesCount, nullptr);

	// Get swapChainImages, instead of asking for real count.
	std::vector<VkImage>			swapchainImagesVector;
	swapchainImagesVector.resize(m_SwapChain_ImagesCount);
	m_SwapchainImageViewsVector.resize(m_SwapChain_ImagesCount);
	SLVK_AbstractGLFW::errorCheck(
		vkGetSwapchainImagesKHR(m_LogicalDevice, m_SwapChain, &m_SwapChain_ImagesCount, swapchainImagesVector.data()),
		std::string("Failed to get SwapChain Images")
	);

	/**************************************************************************************************************************/
	/****************       Create corresponding m_SwapchainImageViewsVector       ***********************************************/
	/**************************************************************************************************************************/
	for (uint32_t i = 0; i < m_SwapChain_ImagesCount; ++i) {
		VkImageViewCreateInfo swapchainImageViewCreateInfo{};
		swapchainImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		swapchainImageViewCreateInfo.image = swapchainImagesVector[i];
		swapchainImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; // handling 2D image
		swapchainImageViewCreateInfo.format = m_SurfaceFormat.format;
		swapchainImageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		swapchainImageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		swapchainImageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		swapchainImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		swapchainImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // color/depth/stencil/metadata
		swapchainImageViewCreateInfo.subresourceRange.baseMipLevel = 0; // first mipmap level accessible to the view
		swapchainImageViewCreateInfo.subresourceRange.levelCount = 1; // amount of mipmaps
		swapchainImageViewCreateInfo.subresourceRange.baseArrayLayer = 0; // first array layer accessible to the view
		swapchainImageViewCreateInfo.subresourceRange.layerCount = 1; // if larger than 1, .viewType needs to be array

		SLVK_AbstractGLFW::errorCheck(
			vkCreateImageView(m_LogicalDevice, &swapchainImageViewCreateInfo, nullptr, &m_SwapchainImageViewsVector[i]),
			std::string("Failed to create SwapChan ImageViews !!")
		);
	}
}

void SLVK_AbstractGLFW::cleanUpSwapChain() {
	cleanUpDepthStencil();

	if (VK_NULL_HANDLE != m_SwapChain) {
		vkDestroySwapchainKHR(m_LogicalDevice, m_SwapChain, nullptr);
		// swapChainImages will be handled by the destroy of swapchain
		// But swapchainImageViews need to be dstroyed first, before the destroy of swapchain.
		for (auto swapchainImageView : m_SwapchainImageViewsVector) {
			vkDestroyImageView(m_LogicalDevice, swapchainImageView, nullptr);
		}
		m_SwapchainImageViewsVector.clear();

		m_SwapChain = VK_NULL_HANDLE;
		// The memory of m_SwapChain images is not managed by programmer (No allocation, nor free)
		// It may not be freed until the window is destroyed, or another swapchain is created for the window.
		/************************************************************************************************************/
		/*********************           Destroy m_SwapchainFramebufferVector         ***************************************/
		/************************************************************************************************************/
		for (auto swapchainFramebuffer : m_SwapchainFramebufferVector) {
			vkDestroyFramebuffer(m_LogicalDevice, swapchainFramebuffer, nullptr);
		}
		m_SwapchainFramebufferVector.clear();
	}
}

void SLVK_AbstractGLFW::createSynchronizationPrimitives() {
	VkSemaphoreCreateInfo semaphoreCreateInfo{};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	SLVK_AbstractGLFW::errorCheck(
		vkCreateSemaphore(m_LogicalDevice, &semaphoreCreateInfo, nullptr, &m_SC_ImageAcquiredSemaphore),
		std::string("Failed to create m_SC_ImageAcquiredSemaphore !!!")
	);
	SLVK_AbstractGLFW::errorCheck(
		vkCreateSemaphore(m_LogicalDevice, &semaphoreCreateInfo, nullptr, &m_SC_PaintReadyToPresentSemaphore),
		std::string("Failed to create m_SC_PaintReadyToPresentSemaphore !!!")
	);

	VkFenceCreateInfo fenceCreateInfo{};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;	// Signaled state, no wait for the first render of each command buffer
	m_SC_WaitCommandBufferCompleteFencesVector.resize(m_SwapChain_ImagesCount);
	for (auto& fence : m_SC_WaitCommandBufferCompleteFencesVector)
	{
		SLVK_AbstractGLFW::errorCheck(
			vkCreateFence(m_LogicalDevice, &fenceCreateInfo, nullptr, &fence),
			std::string("Failed to create waitCommandBufferCompleteFences !!!")
		);
	}
}

/* Draw frames by acquiring images, submitting the right draw command buffer and returning the images back to the swap chain.
	1. vkAcquireNextImageKHR:	Acquire an image from the SwapChain;
	2. vkQueueSubmit:			Select the appropriate command buffer for that image and execute it;  m_SwapchainPresentQueue
	3. vkQueuePresentKHR:		Return the image to the swap chain for presentation to the screen.
*/
void SLVK_AbstractGLFW::swapSwapchain()
{
	/*******************************************************************************************************************************/
	/*********         1. vkAcquireNextImageKHR:	Acquire an image from the SwapChain;     ***************************************/
	/*-----------------------------------------------------------------------------------------------------------------------------*/
	// Use of a presentable image must occur only after the image is returned by vkAcquireNextImageKHR, and before it is presented by vkQueuePresentKHR.
	// This includes transitioning the image layout and rendering commands.
	uint32_t swapchainImageIndex;
	VkResult result = vkAcquireNextImageKHR(m_LogicalDevice, m_SwapChain,
		UINT64_MAX,							// timeout for this Image Acquire command, i.e., (std::numeric_limits<uint64_t>::max)(),
		m_SC_ImageAcquiredSemaphore,	// semaphore to signal
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

	// Use a fence to wait until the m_SwapchainCommandBufferVector[swapchainImageIndex] has finished last execution before using it again
	SLVK_AbstractGLFW::errorCheck(
		vkWaitForFences(m_LogicalDevice, 1, &m_SC_WaitCommandBufferCompleteFencesVector[swapchainImageIndex], VK_TRUE, UINT64_MAX),
		std::string("Failed to vkWaitForFences m_SC_WaitCommandBufferCompleteFencesVector[swapchainImageIndex] !!")
	);
	vkResetFences(m_LogicalDevice, 1, &m_SC_WaitCommandBufferCompleteFencesVector[swapchainImageIndex]);

	/*******************************************************************************************************************************/
	/*********       2. vkQueueSubmit:			Select the appropriate command buffer for that image and execute it    *************/
	/*-----------------------------------------------------------------------------------------------------------------------------*/
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	std::vector<VkSemaphore> submitInfoWaitSemaphoresVecotr;
	submitInfoWaitSemaphoresVecotr.push_back(m_SC_ImageAcquiredSemaphore);
	// Commands before this submitInfoWaitDstStageMaskArray stage could be executed before semaphore signaled
	VkPipelineStageFlags submitInfoWaitDstStageMaskArray[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = (uint32_t)submitInfoWaitSemaphoresVecotr.size();
	submitInfo.pWaitSemaphores = submitInfoWaitSemaphoresVecotr.data();
	submitInfo.pWaitDstStageMask = submitInfoWaitDstStageMaskArray;

	submitInfo.commandBufferCount = 1;	// wait for submitInfoCommandBuffersVecotr to be created
	submitInfo.pCommandBuffers = &m_SwapchainCommandBufferVector[swapchainImageIndex];

	std::vector<VkSemaphore> submitInfoSignalSemaphoresVector;
	submitInfoSignalSemaphoresVector.push_back(m_SC_PaintReadyToPresentSemaphore);
	submitInfo.signalSemaphoreCount = (uint32_t)submitInfoSignalSemaphoresVector.size();
	submitInfo.pSignalSemaphores = submitInfoSignalSemaphoresVector.data();

	SLVK_AbstractGLFW::errorCheck(
		vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, m_SC_WaitCommandBufferCompleteFencesVector[swapchainImageIndex]),
		std::string("Failed to submit draw command buffer !!!")
	);

	/*******************************************************************************************************************************/
	/**  3. m_SwapchainPresentQueue		vkQueuePresentKHR:	Return the image to the swap chain for presentation to the screen.   ***/
	/*-----------------------------------------------------------------------------------------------------------------------------*/
	std::vector<VkSemaphore> presentInfoWaitSemaphoresVector;
	presentInfoWaitSemaphoresVector.push_back(m_SC_PaintReadyToPresentSemaphore);
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = (uint32_t)presentInfoWaitSemaphoresVector.size();
	presentInfo.pWaitSemaphores = presentInfoWaitSemaphoresVector.data();

	VkSwapchainKHR swapChainsArray[] = { m_SwapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChainsArray;
	presentInfo.pImageIndices = &swapchainImageIndex;

	result = vkQueuePresentKHR(m_SwapchainPresentQueue, &presentInfo);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
		reCreateRenderTarget();
	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to present swap chain image !!!");
	}
}

/*---------------------------------------------------------------------------------------------------------------------------------*/
void SLVK_AbstractGLFW::reInitPresentation()
{
	// Call vkDeviceWaitIdle() here, because we shouldn't touch resources that may still be in use. 
	vkDeviceWaitIdle(m_LogicalDevice);

	// Have to use this 3 commands to get currentExtent
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_PhysicalDevice, m_Surface, &m_SurfaceCapabilities);
	if (m_SurfaceCapabilities.currentExtent.width < UINT32_MAX) {
		m_WidgetWidth = m_SurfaceCapabilities.currentExtent.width;
		m_WidgetHeight = m_SurfaceCapabilities.currentExtent.height;
	}
	else {
		glfwGetWindowSize(widgetGLFW, &m_WidgetWidth, &m_WidgetHeight);
	}

	m_SwapchainResize_Viewport.width = static_cast<float>(m_WidgetWidth);
	m_SwapchainResize_Viewport.height = static_cast<float>(m_WidgetHeight);
	m_SwapchainResize_ScissorRect2D.extent.width = static_cast<uint32_t>(m_WidgetWidth);
	m_SwapchainResize_ScissorRect2D.extent.height = static_cast<uint32_t>(m_WidgetHeight);

	cleanUpSwapChain();
	createSwapchain();
}

void SLVK_AbstractGLFW::finalizeAbstractGLFW() {
	/************************************************************************************************************/
	/******************     Destroy depthStencil Memory, ImageView, Image     ***********************************/
	/************************************************************************************************************/
	//if (VK_NULL_HANDLE != depthStencilImage) {
	//	vkDestroyImage(m_LogicalDevice, depthStencilImage, nullptr);
	//	vkDestroyImageView(m_LogicalDevice, depthStencilImageView, nullptr);
	//	vkFreeMemory(m_LogicalDevice, depthStencilImageDeviceMemory, nullptr); 	// always try to destroy before free

	//	depthStencilImage = VK_NULL_HANDLE;
	//	depthStencilImageDeviceMemory = VK_NULL_HANDLE;
	//	depthStencilImageView = VK_NULL_HANDLE;
	//}
	/************************************************************************************************************/
	/*********************           Destroy common RenderPass if not VK_NULL_HANDLE        *********************/
	/************************************************************************************************************/
	if (VK_NULL_HANDLE != m_ColorAttachOnlyRenderPass) {
		vkDestroyRenderPass(m_LogicalDevice, m_ColorAttachOnlyRenderPass, nullptr);
		m_ColorAttachOnlyRenderPass = VK_NULL_HANDLE;
	}
	/************************************************************************************************************/
	/*********************           Destroy m_DefaultThreadCommandPool         ***********************************/
	/************************************************************************************************************/
	if (VK_NULL_HANDLE != m_DefaultThreadCommandPool) {
		vkDestroyCommandPool(m_LogicalDevice, m_DefaultThreadCommandPool, nullptr);
		m_DefaultThreadCommandPool = VK_NULL_HANDLE;

		m_SwapchainCommandBufferVector.clear();
	}
	/************************************************************************************************************/
	/******************     Destroy VertexBuffer, VertexBufferMemory     ****************************************/
	/************************************************************************************************************/
	if (VK_NULL_HANDLE != singleRectIndexBuffer) {
		vkDestroyBuffer(m_LogicalDevice, singleRectIndexBuffer, nullptr);
		vkFreeMemory(m_LogicalDevice, singleRectIndexBufferMemory, nullptr);	// always try to destroy before free

		singleRectIndexBuffer = VK_NULL_HANDLE;
		singleRectIndexBufferMemory = VK_NULL_HANDLE;
	}
	//if (VK_NULL_HANDLE != mvpUniformStagingBuffer) {
	//	vkDestroyBuffer(m_LogicalDevice, mvpUniformStagingBuffer, nullptr);
	//	vkFreeMemory(m_LogicalDevice, mvpUniformStagingBufferDeviceMemory, nullptr);	// always try to destroy before free

	//	mvpUniformStagingBuffer				= VK_NULL_HANDLE;
	//	mvpUniformStagingBufferDeviceMemory = VK_NULL_HANDLE;
	//}
	//if (VK_NULL_HANDLE != mvpOptimalUniformBuffer) {
	//	vkDestroyBuffer(m_LogicalDevice, mvpOptimalUniformBuffer, nullptr);
	//	vkFreeMemory(m_LogicalDevice, mvpOptimalUniformBufferMemory, nullptr);	// always try to destroy before free

	//	mvpOptimalUniformBuffer			= VK_NULL_HANDLE;
	//	mvpOptimalUniformBufferMemory	= VK_NULL_HANDLE;
	//}
	/************************************************************************************************************/
	/*****  SwapChain is a child of Logical Device, must be destroyed before Logical Device  ********************/
	/****************   A surface must outlive any swapchains targeting it    ***********************************/
	cleanUpSwapChain();

	/************************************************************************************************************/
	/*********************           Destroy Synchronization Items             **********************************/
	/************************************************************************************************************/
	if (VK_NULL_HANDLE != m_SC_ImageAcquiredSemaphore) {
		vkDestroySemaphore(m_LogicalDevice, m_SC_ImageAcquiredSemaphore, nullptr);
		m_SC_ImageAcquiredSemaphore = VK_NULL_HANDLE;
	}
	if (VK_NULL_HANDLE != m_SC_PaintReadyToPresentSemaphore) {
		vkDestroySemaphore(m_LogicalDevice, m_SC_PaintReadyToPresentSemaphore, nullptr);
		m_SC_PaintReadyToPresentSemaphore = VK_NULL_HANDLE;
	}
	for (auto& fence : m_SC_WaitCommandBufferCompleteFencesVector) {
		vkDestroyFence(m_LogicalDevice, fence, nullptr);
	}
	m_SC_WaitCommandBufferCompleteFencesVector.clear();

	/************************************************************************************************************/
	/*********************           Destroy logical m_LogicalDevice                **************************************/
	/************************************************************************************************************/
	if (VK_NULL_HANDLE != m_LogicalDevice) {
		vkDestroyDevice(m_LogicalDevice, VK_NULL_HANDLE);

		// Device queues are implicitly cleaned up when the m_LogicalDevice is destroyed
		if (VK_NULL_HANDLE != m_GraphicsQueue) { m_GraphicsQueue = VK_NULL_HANDLE; }
		if (VK_NULL_HANDLE != m_SwapchainPresentQueue) { m_SwapchainPresentQueue = VK_NULL_HANDLE; }

		m_LogicalDevice = VK_NULL_HANDLE;
	}

	/************************************************************************************************************/
	/*********************  Must destroy debugReportCallback before destroy instance   **************************/
	/************************************************************************************************************/
	if (DEBUG_LAYERS_ENABLED) {
		if (VK_NULL_HANDLE != debugReportCallback) {
			fetch_vkDestroyDebugReportCallbackEXT(instance, debugReportCallback, VK_NULL_HANDLE);
			debugReportCallback = VK_NULL_HANDLE;
		}
	}

	/************************************************************************************************************/
	/*************  Destroy window surface, Note that this is a native Vulkan API function  *********************/
	/*****  Surface survives longer than m_LogicalDevice than swapchain, and depends only on Instance, or platform  ******/
	/************************************************************************************************************/
	if (VK_NULL_HANDLE != m_Surface) {
		vkDestroySurfaceKHR(instance, m_Surface, nullptr);	//  m_Surface was created with GLFW function
		m_Surface = VK_NULL_HANDLE;
	}

	/************************************************************************************************************/
	/*********************           Destroy Instance                ********************************************/
	/************************************************************************************************************/
	if (VK_NULL_HANDLE != instance) {
		vkDestroyInstance(instance, VK_NULL_HANDLE); 	instance = VK_NULL_HANDLE;
	}

	OutputDebugString("\n\tFinish  SLVK_AbstractGLFW::finalizeAbstractGLFW()\n");
}

bool SLVK_AbstractGLFW::checkInstanceLayersSupport(std::vector<const char*> layersVector) {
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

/***============================================================================================================*********/
/*************************************       Enumerate All Vulkan      *************************************************/
/*---------------------------------------------------------------------------------------------------------------------------------*/
void SLVK_AbstractGLFW::showAllSupportedInstanceExtensions()
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

void SLVK_AbstractGLFW::showAllSupportedInstanceLayers()
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

void SLVK_AbstractGLFW::showAllSupportedExtensionsEachUnderInstanceLayer()
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
		SLVK_AbstractGLFW::errorCheck(vkEnumerateInstanceExtensionProperties(instanceLayersVector[i].layerName, &extensionsCount, NULL), std::string("Extension Count under Layer Error!"));
		std::vector<VkExtensionProperties> extensionsPropVec(extensionsCount);
		SLVK_AbstractGLFW::errorCheck(vkEnumerateInstanceExtensionProperties(instanceLayersVector[i].layerName, &extensionsCount, extensionsPropVec.data()), std::string("Extension Data() under Layer Error!"));

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

void SLVK_AbstractGLFW::showPhysicalDeviceSupportedLayersAndExtensions(const VkPhysicalDevice & gpuToCheck)
{
	/************************************************************************************************************/
	/******* Check PhysicalDevice Layers with each supported Extensions under Layer******************************/
	/************************************************************************************************************/
	std::ostringstream stream;
	stream << "\nAll Supported PysicalDevice Extensions: \n----------------------------------------------------\n";
	uint32_t gpuExtensionsCount = 0;
	SLVK_AbstractGLFW::errorCheck(vkEnumerateDeviceExtensionProperties(gpuToCheck, nullptr, &gpuExtensionsCount, nullptr), std::string("GPU Extension Count under Layer Error!"));
	std::vector<VkExtensionProperties> gpuExtensionsPropVec(gpuExtensionsCount);
	SLVK_AbstractGLFW::errorCheck(vkEnumerateDeviceExtensionProperties(gpuToCheck, nullptr, &gpuExtensionsCount, gpuExtensionsPropVec.data()), std::string("GPU Extension Data() under Layer Error!"));
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
		SLVK_AbstractGLFW::errorCheck(vkEnumerateDeviceExtensionProperties(gpuToCheck, physicalDeviceLayersVector[i].layerName, &gpuExtensionsCount, NULL), std::string("GPU Extension Count under Layer Error!"));
		std::vector<VkExtensionProperties> gpuExtensionsPropVec(gpuExtensionsCount);
		SLVK_AbstractGLFW::errorCheck(vkEnumerateDeviceExtensionProperties(gpuToCheck, physicalDeviceLayersVector[i].layerName, &gpuExtensionsCount, gpuExtensionsPropVec.data()), std::string("GPU Extension Data() under Layer Error!"));

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

/*---------------------------------------------------------------------------------------------------------------------------------*/
/*************         Private Functions Above      ********************************************************************************/
/*************         Private Functions Above      ********************************************************************************/
/*************         Private Functions Above      ********************************************************************************/
/***********************************************************************************************************************************/
/***********************************************************************************************************************************/
/***********************************************************************************************************************************/
/***********************************************************************************************************************************/



/*****************************************************************************************************************/
/*-----------     Necessary Structures for Resources Descrition       -------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------------*/
void SLVK_AbstractGLFW::createMvpUniformBuffers() {
	VkDeviceSize mvpUboUniformBufferDeviceSize = sizeof(MvpUniformBufferObject);

	SLVK_AbstractGLFW::createResourceBuffer(m_LogicalDevice, mvpUboUniformBufferDeviceSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE, m_PhysicalDeviceMemoryProperties,
		mvpUniformStagingBuffer, mvpUniformStagingBufferDeviceMemory, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	SLVK_AbstractGLFW::createResourceBuffer(m_LogicalDevice, mvpUboUniformBufferDeviceSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE, m_PhysicalDeviceMemoryProperties,
		mvpOptimalUniformBuffer, mvpOptimalUniformBufferMemory, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

}

/*****************************************************************************************************************/
/*-----------             Depth Test FrameBuffer related            ---------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------------*/

void SLVK_AbstractGLFW::createDepthTestAttachment()
{
	/********************************************************************************************************************/
	/******    If first time (not resize):  Check depthTestImage Format,  Initial depthTestImageSubresourceRange     ****/
	if (depthTestFormat == VK_FORMAT_UNDEFINED) {
		VkFormatProperties commonFormatProperties{};
		vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, VK_FORMAT_D32_SFLOAT, &commonFormatProperties);
		if (commonFormatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
			depthTestFormat = VK_FORMAT_D32_SFLOAT; // VK_FORMAT_D32_SFLOAT is extremely common for depthTest
		else {
			for (auto f : SLVK_AbstractGLFW::depthStencilSupportCheckFormatsVector) {
				VkFormatProperties formatProperties{};
				vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, f, &formatProperties);
				if (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
					depthTestFormat = f;
					break;
				}
			}
			if (depthTestFormat == VK_FORMAT_UNDEFINED) {
				throw std::runtime_error("Depth stencil format not selected.");
			}
			hasStencil = SLVK_AbstractGLFW::hasStencilComponent(depthTestFormat);
		}

		depthTestImageSubresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | (hasStencil ? VK_IMAGE_ASPECT_STENCIL_BIT : 0);
		depthTestImageSubresourceRange.baseMipLevel = 0;	// first mipMap level to start
		depthTestImageSubresourceRange.levelCount = 1;
		depthTestImageSubresourceRange.baseArrayLayer = 0;	// first arrayLayer to start
		depthTestImageSubresourceRange.layerCount = 1;
	}
	/********************************************************************************************************************/
	/***************************     Create depthTest Image     *********************************************************/
	SLVK_AbstractGLFW::createResourceImage(m_LogicalDevice, m_WidgetWidth, m_WidgetHeight, VK_IMAGE_TYPE_2D,  // depthTestImage is also a 2D image
		depthTestFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, depthTestImage
		, depthTestImageDeviceMemory, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_SHARING_MODE_EXCLUSIVE, m_PhysicalDeviceMemoryProperties);

	/********************************************************************************************************************/
	/******************************     Create depthTest Image View    **************************************************/
	VkImageViewCreateInfo depthTestImageViewCreateInfo{};
	depthTestImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	depthTestImageViewCreateInfo.image = depthTestImage;
	depthTestImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	depthTestImageViewCreateInfo.format = depthTestFormat;
	depthTestImageViewCreateInfo.subresourceRange = depthTestImageSubresourceRange;

	vkCreateImageView(m_LogicalDevice, &depthTestImageViewCreateInfo, nullptr, &depthTestImageView);
	/********************************************************************************************************************/
	/******************************     Transition depthTest ImageLayout     ********************************************/
	SLVK_AbstractGLFW::transitionResourceImageLayout(depthTestImage, depthTestImageSubresourceRange, VK_IMAGE_LAYOUT_PREINITIALIZED,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, m_LogicalDevice, m_DefaultThreadCommandPool, m_GraphicsQueue);
}

void SLVK_AbstractGLFW::createDepthTestRenderPass()
{
	/********************************************************************************************************************/
	/************    Setting AttachmentDescription:  colorAttachment + depthTestAttachment      *************************/
	/********************************************************************************************************************/
	VkAttachmentDescription colorAttachmentDescription{};
	colorAttachmentDescription.format = m_SurfaceFormat.format;// swapChainImageFormat;
	colorAttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT; // Not using multi-sampling
	colorAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;       // layout before renderPass
	colorAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // auto transition after renderPass

	VkAttachmentDescription depthTestAttachmentDescription{};
	depthTestAttachmentDescription.format = depthTestFormat;
	depthTestAttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT; // Not using multi-sampling
	depthTestAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthTestAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // don't care storing because depth data will not be used after drawing has finished. 
	depthTestAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthTestAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthTestAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // bug happen if preinitialized, not sure why
	depthTestAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

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
	subpassDescriptionArray[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescriptionArray[0].colorAttachmentCount = (uint32_t)colorAttachmentReferenceArray.size();	// Every subpass references one or more attachments
	subpassDescriptionArray[0].pColorAttachments = colorAttachmentReferenceArray.data();
	subpassDescriptionArray[0].pDepthStencilAttachment = &depthTestAttachmentReferenceArray;

	/******* There are two built-in dependencies that take care of the transition at the start of the render pass and at the end of the render pass,
	/////       but the former does not occur at the right time.
	/////       It assumes that the transition occurs at the start of the pipeline, but we haven't acquired the image yet at that point! ****/
	/******* There are two ways to deal with this problem.
	/////       We could change the waitStages for the imageAvailableSemaphore to VK_PIPELINE_STAGE_TOP_OF_PIPELINE_BIT
	/////        to ensure that the render passes don't begin until the image is available,
	/////       or we can make the render pass wait for the VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT stage. ****************************/
	std::vector<VkSubpassDependency> subpassDependencyVector;
	VkSubpassDependency headSubpassDependency{};
	headSubpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;		// subpassIndex, from external
	headSubpassDependency.dstSubpass = 0;						// subpassIndex, to the first subpass, which is also the only one
	headSubpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // specify the operations to wait on and the stages in which these operations occur.
	headSubpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	headSubpassDependency.srcAccessMask = 0;											 // specify the operations to wait on and the stages in which these operations occur.
	headSubpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT
		| VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	subpassDependencyVector.push_back(headSubpassDependency);

	/********************************************************************************************************************/
	/*********************    Create RenderPass for rendering triangle      *********************************************/
	/********************************************************************************************************************/
	VkRenderPassCreateInfo depthTestRenderPassCreateInfo{};
	depthTestRenderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	depthTestRenderPassCreateInfo.attachmentCount = (uint32_t)attachmentDescriptionVector.size();
	depthTestRenderPassCreateInfo.pAttachments = attachmentDescriptionVector.data();
	depthTestRenderPassCreateInfo.subpassCount = (uint32_t)subpassDescriptionArray.size();
	depthTestRenderPassCreateInfo.pSubpasses = subpassDescriptionArray.data();
	depthTestRenderPassCreateInfo.dependencyCount = (uint32_t)subpassDependencyVector.size();
	depthTestRenderPassCreateInfo.pDependencies = subpassDependencyVector.data();

	SLVK_AbstractGLFW::errorCheck(
		vkCreateRenderPass(m_LogicalDevice, &depthTestRenderPassCreateInfo, nullptr, &depthTestRenderPass),
		std::string("Failed to create render pass !!")
	);
}

void SLVK_AbstractGLFW::createDepthTestSwapchainFramebuffers()
{
	/************************************************************************************************************/
	/*********     Destroy old swapchainFramebuffers first for widgetRezie, if there are      *******************/
	/************************************************************************************************************/
	for (auto swapchainFramebuffer : m_SwapchainFramebufferVector) {
		vkDestroyFramebuffer(m_LogicalDevice, swapchainFramebuffer, nullptr);
	}
	m_SwapchainFramebufferVector.clear();
	m_SwapchainFramebufferVector.resize(m_SwapChain_ImagesCount);

	for (size_t i = 0; i < m_SwapChain_ImagesCount; i++) {
		std::array<VkImageView, 2> imageViewAttachmentArray = {
			m_SwapchainImageViewsVector[i],
			// The same depth image can be used by all of them,
			// because only a single subpass is running at the same time in this example due to the semaphores.
			depthTestImageView
		};

		VkFramebufferCreateInfo framebufferCreateInfo{};
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.renderPass = depthTestRenderPass;
		framebufferCreateInfo.attachmentCount = (uint32_t)imageViewAttachmentArray.size();
		framebufferCreateInfo.pAttachments = imageViewAttachmentArray.data();
		framebufferCreateInfo.width = m_WidgetWidth;
		framebufferCreateInfo.height = m_WidgetHeight;
		framebufferCreateInfo.layers = 1;

		SLVK_AbstractGLFW::errorCheck(
			vkCreateFramebuffer(m_LogicalDevice, &framebufferCreateInfo, nullptr, &m_SwapchainFramebufferVector[i]),
			std::string("Failed to create framebuffer !!!")
		);
	}
}


/*****************************************************************************************************************/
/*****************************************************************************************************************/
/*-----------             Depth Stencil FrameBuffer related, Not Applicable Yet    ------------------------------*/
/*---------------------------------------------------------------------------------------------------------------*/

///* A helper function that records memory barrirers using the vkCmdPipelineBarrier() command */
void SLVK_AbstractGLFW::setImageMemoryBarrier(VkImage image, VkImageAspectFlags imageAspectFlags
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

void SLVK_AbstractGLFW::createDepthStencilAttachment()
{
	/********************************************************************************************************************/
	/******************************  Check Image Format *****************************************************************/
	for (auto f : SLVK_AbstractGLFW::depthStencilSupportCheckFormatsVector) {
		VkFormatProperties formatProperties{};
		vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, f, &formatProperties);
		if (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
			depthStencilFormat = f;
			break;
		}
	}
	if (depthStencilFormat == VK_FORMAT_UNDEFINED) {
		throw std::runtime_error("Depth stencil format not selected.");
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
	depthStencilImageCreateInfo.extent.width = m_WidgetWidth;
	depthStencilImageCreateInfo.extent.height = m_WidgetHeight;
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

	vkCreateImage(m_LogicalDevice, &depthStencilImageCreateInfo, nullptr, &depthStencilImage);

	/******************************************************************************************************************************************************/
	/***************************  Allocate & Bind memory for depthStencil Image using the created handle *********************************************************/
	VkMemoryRequirements imageMemoryRequirements{};
	vkGetImageMemoryRequirements(m_LogicalDevice, depthStencilImage, &imageMemoryRequirements);

	uint32_t gpuMemoryTypeIndex = SLVK_AbstractGLFW::findPhysicalDeviceMemoryPropertyIndex(
		m_PhysicalDeviceMemoryProperties,
		imageMemoryRequirements,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT  // Set the resource to reside on GPU itself
	);

	VkMemoryAllocateInfo depthStencilImageMemoryAllocateInfo{};
	depthStencilImageMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	depthStencilImageMemoryAllocateInfo.allocationSize = imageMemoryRequirements.size;
	depthStencilImageMemoryAllocateInfo.memoryTypeIndex = gpuMemoryTypeIndex;

	vkAllocateMemory(m_LogicalDevice, &depthStencilImageMemoryAllocateInfo, nullptr, &depthStencilImageDeviceMemory);
	vkBindImageMemory(m_LogicalDevice, depthStencilImage, depthStencilImageDeviceMemory, 0);

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

	vkCreateImageView(m_LogicalDevice, &depthStencilImageViewCreateInfo, nullptr, &depthStencilImageView);


	//// Use command buffer to create the depth image. This includes -
	//// Command buffer allocation, recording with begin/end scope and submission.
	//CommandBufferMgr::allocCommandBuffer(&deviceObj->m_LogicalDevice, cmdPool, &cmdDepthImageCommandBuffer);
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
	//result = vkCreateImageView(deviceObj->m_LogicalDevice, &depthImageViewCreateInfo, NULL, &FormatImageMemoryViewDepthStruct.view);
	//assert(result == VK_SUCCESS);
}

void SLVK_AbstractGLFW::createDepthStencilRenderPass()
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
	attachmentDescriptionsArray[1].format = m_SurfaceFormat.format;
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

	SLVK_AbstractGLFW::errorCheck(
		vkCreateRenderPass(m_LogicalDevice, &renderPassCreateInfo, nullptr, &depthStencilRenderPass),
		std::string("Failed to create colorDepthStencil render pass !!")
	);
}

void SLVK_AbstractGLFW::createDepthStencilGraphicsPipeline()
{
}