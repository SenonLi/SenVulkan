#pragma once

#ifndef __SenRenderer__
#define __SenRenderer__


#include <vulkan/vulkan.h>

#include <cstdlib>
#include <assert.h>
#include <vector>
#include <iostream>
#include <sstream>

#ifdef _WIN32
#include <Windows.h>
#endif

#include "Shared.h"


class SenRenderer
{
public:
	SenRenderer();
	virtual ~SenRenderer();

private:
public:
	void _InitInstance();
	void _DeInitInstance();

	void _InitDevice();
	void _DeInitDevice();

	void _SetupDebug();
	void _InitDebug();
	void _DeInitDebug();

	VkInstance			_instance = VK_NULL_HANDLE;
	VkPhysicalDevice	_gpu = VK_NULL_HANDLE;
	VkDevice			_device = VK_NULL_HANDLE;
	VkPhysicalDeviceProperties   _gpuProperties = {};

	uint32_t			_graphicsFamilyIndex = 0;

	std::vector<const char*> _instanceLayersList;
	std::vector<const char*>	_instanceExtensionsList;
	std::vector<const char*>	_deviceLayersList;
	std::vector<const char*>	_deviceExtensionsList;

	PFN_vkCreateDebugReportCallbackEXT	fetch_vkCreateDebugReportCallbackEXT = VK_NULL_HANDLE;
	PFN_vkDestroyDebugReportCallbackEXT	fetch_vkDestroyDebugReportCallbackEXT = VK_NULL_HANDLE;

	VkDebugReportCallbackEXT	_debugReport = VK_NULL_HANDLE;
	VkDebugReportCallbackCreateInfoEXT debugCallbackCreateInfo = {};
};

#endif // __SenRenderer__