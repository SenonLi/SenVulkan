#pragma once

#ifndef __SenAbstractGLFW__
#define __SenAbstractGLFW__

#if defined( _WIN32 )  // on Windows OS

#include <Windows.h> // for OutputDebugString() function
//#define VK_USE_PLATFORM_WIN32_KHR 1 // For Vulkan Surface if not using GLFW

#elif defined( __linux ) // on Linux ( Via XCB library )
// xcb seems like a popular and well supported option on X11, until wayland and mir take over
#define VK_USE_PLATFORM_XCB_KHR 1
#include <xcb/xcb.h>
#endif


#include <stdexcept> // for propagating errors 
#include <stdlib.h> // for const error catch
#include <stdio.h>  

#include <vector>
#include <string>
#include <iostream> // for cout, cin
#include <sstream>  // for quick chunch cout
#include <map>		// for ratePhysicalDevice to pick the best
#include <set>		// for unique queue family <==> unique queue

#define GLFW_INCLUDE_VULKAN // Let GLFW know Vulkan is utilized, must be in front of vulkan.h
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.h>

//#include "LoadShaders.h"

class SenAbstractGLFW
{
public:
	SenAbstractGLFW();
	virtual ~SenAbstractGLFW();

	void showWidget();

//	void _protectedKeyDetection(GLFWwindow* widget, int key, int scancode, int action, int mode) { 
//		keyDetection(widget, key, scancode, action, mode);
//	}

protected:
	GLFWwindow* widgetGLFW;
	int widgetWidth, widgetHeight;
	char* strWindowName;

	VkInstance						instance = VK_NULL_HANDLE;

	/*******************************************************************************************************************************/
	/********* VkSurfaceKHR object that represents an abstract type of surface to present rendered images to. **********************/
	/********* The surface in our program will be backed by the window that we've already opened with GLFW.   **********************/
	VkSurfaceKHR					surface						= VK_NULL_HANDLE; // VK_KHR_surface Instance Extension
	std::vector<VkSurfaceFormatKHR> surfaceFormatVector;
	VkSurfaceFormatKHR				surfaceFormat				= {};
	VkSurfaceCapabilitiesKHR		surfaceCapabilities			= {};
	
	VkPhysicalDevice				physicalDevice				= VK_NULL_HANDLE;
	VkPhysicalDeviceProperties		physicalDeviceProperties	= {};	// GPU name, type (discrete)
	int32_t							graphicsQueueFamilyIndex	= -1;  // Index of Graphics QueueFamily of GPU that we will choose to 
	int32_t							presentQueueFamilyIndex		= -1;  // The Graphics (Drawing) QueueFamily may not support presentation (WSI)

	VkDevice						device						= VK_NULL_HANDLE;
	VkQueue							graphicsQueue				= VK_NULL_HANDLE; // Handle to the graphics queue
	VkQueue							presentQueue				= VK_NULL_HANDLE; // Since presentQueueFamilyIndex may not == graphicsQueueFamilyIndex, make two queue

	VkSwapchainKHR					swapChain					= VK_NULL_HANDLE;
	uint32_t						swapchainImagesCount		= 2;
	std::vector<VkImage>			swapchainImagesVector;
	std::vector<VkImageView>		swapchainImageViewsVector;

	VkImage								depthStencilImage				= VK_NULL_HANDLE;
	VkPhysicalDeviceMemoryProperties	physicalDeviceMemoryProperties	= {};
	VkDeviceMemory						depthStencilImageDeviceMemory	= VK_NULL_HANDLE;
	VkImageView							depthStencilImageView			= VK_NULL_HANDLE;
	VkFormat							depthStencilFormat				= VK_FORMAT_UNDEFINED;
	bool								stencilAvailable				= false;

	//VkRenderPass renderPass;
	//VkPipelineLayout pipelineLayout;
	//VkPipeline graphicsPipeline;

	//VkCommandPool commandPool;
	//std::vector<VkCommandBuffer> commandBuffers;

	//VkSemaphore imageAvailableSemaphore;
	//VkSemaphore renderFinishedSemaphore;

//	float xRot, yRot;
//	float aspect;

	virtual void initGlfwVulkan();
	virtual void paintVulkan();
	virtual void finalize();

//	virtual void keyDetection(GLFWwindow* widget, int key, int scancode, int action, int mode);

//	int modelMatrixLocation;
//	int projectionMatrixLocation;

	const int DEFAULT_widgetWidth = 800;	// 640;
	const int DEFAULT_widgetHeight = 600; // 640;

#ifdef _DEBUG
	const bool layersEnabled = true;
#else
	const bool layersEnabled = false;
#endif

private:
	std::vector<const char*> debugInstanceLayersVector;
	std::vector<const char*> debugInstanceExtensionsVector;
	std::vector<const char*> debugDeviceLayersVector; 				// depricated, but still recommended
	std::vector<const char*> debugDeviceExtensionsVector;

	VkDebugReportCallbackCreateInfoEXT debugReportCallbackCreateInfo = {}; // important for creations of both instance and debugReportCallback
	VkDebugReportCallbackEXT	debugReportCallback = VK_NULL_HANDLE;
	PFN_vkCreateDebugReportCallbackEXT	fetch_vkCreateDebugReportCallbackEXT = VK_NULL_HANDLE;
	PFN_vkDestroyDebugReportCallbackEXT	fetch_vkDestroyDebugReportCallbackEXT = VK_NULL_HANDLE;

	static VKAPI_ATTR VkBool32 VKAPI_CALL pfnDebugCallback(VkFlags,VkDebugReportObjectTypeEXT,uint64_t,size_t,int32_t,const char*,const char*,void *);

	void initDebugLayers();
	void initExtensions();
	void createInstance();
	void initDebugReportCallback();

	/*******************************************************************************************************************************/
	/********* The window surface needs to be created right after the instance creation, *******************************************/
	/********* because it can actually influence the physical device selection.          *******************************************/
	void createSurface(); // surface == default framebuffer to draw

	void showPhysicalDeviceInfo(const VkPhysicalDevice& gpuToCheck);
	bool isPhysicalDeviceSuitable(const VkPhysicalDevice& gpuToCheck, int32_t& graphicsQueueIndex, int32_t& presentQueueIndex);
	int  ratePhysicalDevice(const VkPhysicalDevice& gpuToCheck, int32_t& graphicsQueueIndex, int32_t& presentQueueIndex);
	void pickPhysicalDevice();
	void createLogicalDevice();
	
	void createSwapChain();
	void createSwapChainImageViews();

	uint32_t findPhysicalDeviceMemoryPropertyIndex(
		const VkPhysicalDeviceMemoryProperties& gpuMemoryProperties,
		const VkMemoryRequirements& memoryRequirements, 
		const VkMemoryPropertyFlags& requiredMemoryPropertyFlags
	);
	void createDepthStencilAttachment();
	//void createRenderPass();
	//void createGraphicsPipeline();
	//void createFramebuffers();
	//void createCommandPool();
	//void createCommandBuffers();
	//void createSemaphores();

//	void keyboardRegister();



	void errorCheck(VkResult result, std::string msg);
	bool checkInstanceLayersSupport(std::vector<const char*> layersVector);

// Enumerate All
	void showAllSupportedInstanceExtensions();
	void showAllSupportedInstanceLayers();
	void showAllSupportedExtensionsEachUnderInstanceLayer();
	void showPhysicalDeviceSupportedLayersAndExtensions(const VkPhysicalDevice& gpuToCheck);
};


#endif //__SenAbstractGLFW__