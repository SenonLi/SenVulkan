#pragma once

#ifndef __SenRenderer__
#define __SenRenderer__


#include <vulkan/vulkan.h>

#include <cstdlib>
#include <assert.h>
#include <vector>
#include <iostream>

class SenRenderer
{
public:
	SenRenderer();
	virtual ~SenRenderer();

private:
	void _InitInstance();
	void _DeInitInstance();

	void _InitDevice();
	void _DeInitDevice();

	void _SetupDebug();
	void _InitDebug();
	void _DeInitDebug();

	VkInstance			_instance = nullptr;
	VkPhysicalDevice	_gpu = nullptr;
	VkDevice			_device = nullptr;
	VkPhysicalDeviceProperties   _gpuProperties = {};

	uint32_t			_graphicsFamilyIndex = 0;

	std::vector<const char*> _instanceLayersList;
	std::vector<const char*>	_instanceExtensionsList;
	std::vector<const char*>	_deviceLayersList;
	std::vector<const char*>	_deviceExtensionsList;
};

#endif // __SenRenderer__