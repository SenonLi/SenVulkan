#pragma once

#ifndef __Sen_221_Cube__
#define __Sen_221_Cube__

#include "../Support/SenAbstractGLFW.h"

class Sen_221_Cube :	public SenAbstractGLFW
{
public:
	Sen_221_Cube();
	virtual ~Sen_221_Cube();

protected:
	void initVulkanApplication();
	void reCreateRenderTarget(); // for resize window
	void finalizeWidget();

	void cleanUpDepthStencil();
	void updateUniformBuffer();

private:
	void createDepthTestIndexBuffer();
	void createDepthTestVertexBuffer();
	void createDepthTestCommandBuffers();
	void createDepthTestPipeline();
	void initBackgroundTextureImage();
	void createTextureAppDescriptorPool();
	void createTextureAppDescriptorSetLayout();
	void createTextureAppDescriptorSet();

	VkImage							backgroundTextureImage				= VK_NULL_HANDLE;
	VkDeviceMemory					backgroundTextureImageDeviceMemory	= VK_NULL_HANDLE;
	VkImageView						backgroundTextureImageView			= VK_NULL_HANDLE;
	VkSampler						texture2DSampler					= VK_NULL_HANDLE;


	VkBuffer						depthTestVertexBuffer				= VK_NULL_HANDLE;
	VkDeviceMemory					depthTestVertexBufferMemory			= VK_NULL_HANDLE;
	VkPipeline						depthTestPipeline					= VK_NULL_HANDLE;

	VkPipelineLayout				textureAppPipelineLayout			= VK_NULL_HANDLE;

	int backgroundTextureWidth, backgroundTextureHeight;
	const char* backgroundTextureDiskAddress;
};


#endif // !__Sen_221_Cube__

