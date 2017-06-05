#pragma once

#ifndef __Sen_072_TextureArray__
#define __Sen_072_TextureArray__

#include "../Support/SenAbstractGLFW.h"

class Sen_072_TextureArray :	public SenAbstractGLFW
{
public:
	Sen_072_TextureArray();
	virtual ~Sen_072_TextureArray();

protected:
	void initVulkanApplication();
	void reCreateRenderTarget(); // for resize window
	void finalizeWidget();

	void updateUniformBuffer();
	void cleanUpDepthStencil() { ; }

	void createDeviceLocalTextureKTX_2D(const VkDevice& logicalDevice, const VkPhysicalDeviceMemoryProperties& gpuMemoryProperties
		, const char*& textureDiskAddress, const VkImageType& imageType, int& textureWidth, int& textureHeight
		, VkImage& deviceLocalTextureToCreate, VkDeviceMemory& textureDeviceMemoryToAllocate, VkImageView& textureImageViewToCreate
		, const VkSharingMode& imageSharingMode, const VkCommandPool& tmpCommandBufferCommandPool, const VkQueue& imageMemoryTransferQueue);

private:
	void createTextureAppPipeline();
	void createTextureAppVertexBuffer();

	void initBackgroundTextureImage();
	void createTextureAppDescriptorPool();
	void createTextureAppDescriptorSetLayout();
	void createTextureAppDescriptorSet();
	void createTextureAppCommandBuffers();

	VkPipelineLayout					textureAppPipelineLayout		= VK_NULL_HANDLE;
	VkPipeline							textureAppPipeline				= VK_NULL_HANDLE;
	VkBuffer							textureAppVertexBuffer			= VK_NULL_HANDLE;
	VkDeviceMemory						textureAppVertexBufferMemory	= VK_NULL_HANDLE;

	int backgroundTextureWidth, backgroundTextureHeight;
	const char* backgroundTextureDiskAddress;
	VkImage backgroundTextureImage						= VK_NULL_HANDLE;
	VkDeviceMemory backgroundTextureImageDeviceMemory	= VK_NULL_HANDLE;
	VkImageView backgroundTextureImageView				= VK_NULL_HANDLE;

	VkSampler texture2DSampler							= VK_NULL_HANDLE;
};


#endif // !__Sen_07_Texture__

