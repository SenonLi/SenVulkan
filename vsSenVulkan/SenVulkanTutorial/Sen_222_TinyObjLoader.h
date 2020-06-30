#pragma once

#ifndef __Sen_222_TinyObjLoader__
#define __Sen_222_TinyObjLoader__

#include "../Support/SLVK_AbstractGLFW.h"
#include "../Support/SenTinyObjLoader.h"

class Sen_222_TinyObjLoader :	public SLVK_AbstractGLFW
{
public:
	Sen_222_TinyObjLoader();
	virtual ~Sen_222_TinyObjLoader();

protected:
	void initVulkanApplication();
	void reCreateRenderTarget(); // for resize window
	void finalizeWidget();

	void cleanUpDepthStencil();
	void updateUniformBuffer();

private:
	void createMeshLinkModelndexBuffer();
	void createMeshLinkModeVertexBuffer();
	void createTinyObjLoaderCommandBuffers();

	void initTinyObjCompleteTextureImage();
	void createTinyObjLoaderPipeline();
	void createTextureAppDescriptorPool();
	void createTextureAppDescriptorSetLayout();
	void createTextureAppDescriptorSet();

	/*****************************************************************************************************************/
	/*------------------------     For Resources Descrition       ---------------------------------------------------*/
	/*---------------------------------------------------------------------------------------------------------------*/
	/* uniform values need to be specified during pipeline creation by creating a VkPipelineLayout object */
	VkDescriptorPool				m_DescriptorPool					= VK_NULL_HANDLE;
	VkDescriptorSetLayout			m_Default_DSL						= VK_NULL_HANDLE;
	VkDescriptorSet					m_Default_DS						= VK_NULL_HANDLE;

	const int						m_COMBINED_IMAGE_SAMPLER_DS_Index	= 1;
	VkImage							tinyObjCompleteImage				= VK_NULL_HANDLE;
	VkDeviceMemory					tinyObjCompleteImageDeviceMemory	= VK_NULL_HANDLE;
	VkImageView						tinyObjCompleteImageView			= VK_NULL_HANDLE;
	VkSampler						texture2DSampler					= VK_NULL_HANDLE;


	VkBuffer						tinyMeshLinkModelVertexBuffer		= VK_NULL_HANDLE;
	VkDeviceMemory					tinyMeshLinkModelVertexBufferMemory	= VK_NULL_HANDLE;
	VkBuffer						tinyMeshLinkModelIndexBuffer		= VK_NULL_HANDLE;
	VkDeviceMemory					tinyMeshLinkModelIndexBufferMemory	= VK_NULL_HANDLE;

	VkPipeline						tinyObjLoaderPipeline				= VK_NULL_HANDLE;

	VkPipelineLayout				tinyObjLoaderPipelineLayout			= VK_NULL_HANDLE;

	int tinyObjCompleteTextureWidth, tinyObjCompleteTextureHeight;
	const char* tinyObjCompleteTextureDiskAddress;
	const char* tinyObjectDiskAddress;

	std::vector<VertexStruct>	vertexStructVector;
	std::vector<uint32_t>		indexVector;
};


#endif // !__Sen_222_TinyObjLoader__

