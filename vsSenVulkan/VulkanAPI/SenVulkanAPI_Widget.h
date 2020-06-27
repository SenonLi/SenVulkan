#pragma once

#include "Shared.h"
#include "SenRenderer.h"
#include "SenWindow.h"


#define GLFW_INCLUDE_VULKAN // Let GLFW know Vulkan is utilized
#include <GLFW/glfw3.h>

#include <chrono>
#include <array>

constexpr double PI = 3.14159265358979323846;
constexpr double CIRCLE_RAD = PI * 2;
constexpr double CIRCLE_THIRD = CIRCLE_RAD / 3.0;
constexpr double CIRCLE_THIRD_1 = 0;
constexpr double CIRCLE_THIRD_2 = CIRCLE_THIRD;
constexpr double CIRCLE_THIRD_3 = CIRCLE_THIRD * 2;

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
	const char* strWindowName;


	void showWidget();


	virtual void initGlfwVulkanDebugWSI();
	//	virtual void paintVulkan();
	virtual void finalize();

	void initCommandBuffers();




};

