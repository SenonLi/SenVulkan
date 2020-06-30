#pragma once

#ifndef __Sen_06_Triangle__
#define __Sen_06_Triangle__

#include "../Support/SLVK_AbstractGLFW.h"

class Sen_06_Triangle : public SLVK_AbstractGLFW
{
public:
	Sen_06_Triangle();
	virtual ~Sen_06_Triangle();
	
protected:
	void initVulkanApplication();
	void reCreateRenderTarget();// for resize window

	void cleanUpDepthStencil() { ; }
	void finalizeWidget();
	void updateUniformBuffer();
	
private:
	void createTriangleDescriptorSetLayout();
	void createTriangleDescriptorPool();
	void createTriangleDescriptorSet(); // need to be after createTriangleDescriptorPool
	void createTrianglePipeline();
	void createTriangleVertexBuffer();
	void createTriangleCommandBuffers();

	VkPipelineLayout					trianglePipelineLayout		= VK_NULL_HANDLE;
	VkPipeline							trianglePipeline			= VK_NULL_HANDLE;

	/*****************************************************************************************************************/
	/*------------------------     For Resources Descrition       ---------------------------------------------------*/
	/*---------------------------------------------------------------------------------------------------------------*/
	/* uniform values need to be specified during pipeline creation by creating a VkPipelineLayout object */
	VkDescriptorPool					m_DescriptorPool			= VK_NULL_HANDLE;
	VkDescriptorSetLayout				m_Default_DSL				= VK_NULL_HANDLE;
	VkDescriptorSet						m_Default_DS				= VK_NULL_HANDLE;

	VkBuffer							triangleVertexBuffer		= VK_NULL_HANDLE;
	VkDeviceMemory						triangleVertexBufferMemory	= VK_NULL_HANDLE;
};




#endif // !__Sen_06_Triangle__

