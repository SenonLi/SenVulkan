#pragma once

#ifndef __Sen_22_DepthTest__
#define __Sen_22_DepthTest__

#include "../Support/SenAbstractGLFW.h"

class Sen_22_DepthTest :	public SenAbstractGLFW
{
public:
	Sen_22_DepthTest();
	virtual ~Sen_22_DepthTest();

protected:
	void initVulkanApplication();
	void reCreateRenderTarget(); // for resize window
	void finalizeWidget();

	void cleanUpDepthStencil();
	void updateUniformBuffer();

private:
	void createDepthTestIndexBuffer();
	void createDepthTestVertexBuffer();
	void createDepthResources();

	void createTextureAppPipeline();
	void initBackgroundTextureImage();
	void createTextureAppDescriptorPool();
	void createTextureAppDescriptorSetLayout();
	void createTextureAppDescriptorSet();
	void createTextureAppCommandBuffers();


	VkRenderPass					depthTestRenderPass					= VK_NULL_HANDLE;
	VkPipelineLayout				depthTestPipelineLayout				= VK_NULL_HANDLE;
	VkPipeline						depthTestPipeline					= VK_NULL_HANDLE;

	VkPipelineLayout				textureAppPipelineLayout			= VK_NULL_HANDLE;
	VkPipeline						textureAppPipeline					= VK_NULL_HANDLE;
	
	VkBuffer						textureAppVertexBuffer				= VK_NULL_HANDLE;
	VkDeviceMemory					textureAppVertexBufferMemory		= VK_NULL_HANDLE;

	int backgroundTextureWidth, backgroundTextureHeight;
	const char* backgroundTextureDiskAddress;
	VkImage							backgroundTextureImage				= VK_NULL_HANDLE;
	VkDeviceMemory					backgroundTextureImageDeviceMemory	= VK_NULL_HANDLE;
	VkImageView						backgroundTextureImageView			= VK_NULL_HANDLE;
	VkSampler						texture2DSampler					= VK_NULL_HANDLE;

	VkImage							depthTestImage						= VK_NULL_HANDLE;
	VkDeviceMemory					depthTestImageDeviceMemory			= VK_NULL_HANDLE;
	VkImageView						depthTestImageView					= VK_NULL_HANDLE;
	VkFormat						depthTestFormat						= VK_FORMAT_UNDEFINED;
};


#endif // !__Sen_07_Texture__

