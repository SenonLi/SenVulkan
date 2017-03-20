#pragma once

#include "Shared.h"
#include "SenRenderer.h"
//#include "SenWindow.h"


#define GLFW_INCLUDE_VULKAN // Let GLFW know Vulkan is utilized
#include <GLFW/glfw3.h>

class SenVulkanAPI_Widget
{
public:
	SenVulkanAPI_Widget();
	virtual ~SenVulkanAPI_Widget();

	SenRenderer renderer;
	VkDevice device;
	VkQueue queue;

	VkFence fence;
	VkFenceCreateInfo fenceCreateInfo{};
	VkSemaphore semaphore;
	VkCommandPool commandPool;


	GLFWwindow* widgetGLFW;
	int widgetWidth, widgetHeight;
	char* strWindowName;


	void showWidget();


	virtual void initGlfwVulkan();
	//	virtual void paintVulkan();
	virtual void finalize();

	void initCommandBuffers();




};

