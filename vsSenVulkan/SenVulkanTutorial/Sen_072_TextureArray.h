#pragma once

#ifndef __Sen_072_TextureArray__
#define __Sen_072_TextureArray__

#include "../Support/SLVK_AbstractGLFW.h"

class Sen_072_TextureArray :	public SLVK_AbstractGLFW
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

private:
	void createTextureAppPipeline();
	void createTextureAppVertexBuffer();

	void initTex2DArrayImage();

	void createTextureAppDescriptorPool();
	void createTextureAppDescriptorSetLayout();
	void createTextureAppDescriptorSet();

	void createTex2DArrayCommandBuffers();

	VkPipelineLayout					textureAppPipelineLayout		= VK_NULL_HANDLE;
	VkPipeline							textureAppPipeline				= VK_NULL_HANDLE;

	/*****************************************************************************************************************/
	/*------------------------     For Resources Descrition       ---------------------------------------------------*/
	/*---------------------------------------------------------------------------------------------------------------*/
	/* uniform values need to be specified during pipeline creation by creating a VkPipelineLayout object */
	VkDescriptorPool					m_DescriptorPool					= VK_NULL_HANDLE;
	VkDescriptorSetLayout				m_Default_DSL						= VK_NULL_HANDLE;
	VkDescriptorSet						m_Default_DS						= VK_NULL_HANDLE;

	const int							m_COMB_IMA_SAMPLER_DS_BindingIndex	= 3;
	VkBuffer							textureAppVertexBuffer				= VK_NULL_HANDLE;
	VkDeviceMemory						textureAppVertexBufferMemory		= VK_NULL_HANDLE;

	int backgroundTextureWidth, backgroundTextureHeight;
	const char* backgroundTextureDiskAddress;
	VkImage backgroundTextureImage						= VK_NULL_HANDLE;
	VkDeviceMemory backgroundTextureImageDeviceMemory	= VK_NULL_HANDLE;
	VkImageView backgroundTextureImageView				= VK_NULL_HANDLE;

	VkSampler texture2DSampler							= VK_NULL_HANDLE;
};


#endif // !__Sen_07_Texture__

