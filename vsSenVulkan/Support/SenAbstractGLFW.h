#pragma once

#ifndef __SenAbstractGLFW__
#define __SenAbstractGLFW__

#include <stdexcept> // for propagating errors 
#include <stdlib.h> // for const error catch
#include <stdio.h>  
#include <Windows.h> // for OutputDebugString() function

#include <vector>
#include <string>
#include <iostream> // for cout, cin
#include <sstream>  // for quick chunch cout
#include <map>

#define GLFW_INCLUDE_VULKAN // Let GLFW know Vulkan is utilized
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

	VkInstance						instance		= VK_NULL_HANDLE;
	
	VkPhysicalDevice				physicalDevice	= VK_NULL_HANDLE;
	VkPhysicalDeviceProperties		physicalDeviceProperties = {};	// GPU name, type (discrete)
	int32_t							graphicsQueueFamilyIndex = -1;  // Index of Graphics QueueFamily of GPU that we will choose to 

	VkDevice						device			= VK_NULL_HANDLE;
	VkQueue							graphicsQueue	= VK_NULL_HANDLE;

//	float xRot, yRot;
//	float aspect;

	virtual void initGlfwVulkan();
//	virtual void paintVulkan();
	virtual void finalize();

//	virtual void keyDetection(GLFWwindow* widget, int key, int scancode, int action, int mode);

//	// shader info variables
//	GLuint programA, programB;
//	GLuint defaultTextureID;
//	int textureLocation;

//	GLuint verArrObjArray[2];
//	GLuint verBufferObjArray[2];
//	GLuint verIndicesObjArray[2];

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
	
	void showPhysicalDeviceInfo(const VkPhysicalDevice& gpuToCheck);
	bool isPhysicalDeviceSuitable(const VkPhysicalDevice& gpuToCheck);
	int  ratePhysicalDevice(const VkPhysicalDevice& gpuToCheck);
	void pickPhysicalDevice();

	void createLogicalDevice();

//	void keyboardRegister();


	//errorCheck(
	//	vkCreateInstance(&instanceCreateInfo, nullptr, &instance),
	//	std::string("Failed to create instance! \t Error:\t")
	//);

	void errorCheck(VkResult result, std::string msg);
	bool checkInstanceLayersSupport(std::vector<const char*> layersVector);

// Enumerate All
	void showAllSupportedInstanceExtensions();
	void showAllSupportedInstanceLayers();
	void showAllSupportedExtensionsEachUnderInstanceLayer();
	void showPhysicalDeviceSupportedLayersAndExtensions(const VkPhysicalDevice& gpuToCheck);
};


#endif //__SenAbstractGLFW__