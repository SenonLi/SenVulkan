#pragma once

#ifndef __Sen_06_Triangle__
#define __Sen_06_Triangle__

#include "../Support/SenAbstractGLFW.h"

class Sen_06_Triangle : public SenAbstractGLFW
{
public:
	Sen_06_Triangle();
	virtual ~Sen_06_Triangle();
	
protected:
	void initVulkanApplication();
	void reCreateRenderTarget();// for resize window

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
	VkBuffer							triangleVertexBuffer		= VK_NULL_HANDLE;
	VkDeviceMemory						triangleVertexBufferMemory	= VK_NULL_HANDLE;
};




#endif // !__Sen_06_Triangle__

