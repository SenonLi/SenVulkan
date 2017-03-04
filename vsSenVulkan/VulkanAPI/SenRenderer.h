#pragma once

#ifndef __SenRenderer__
#define __SenRenderer__


#include <vulkan/vulkan.h>

#include <cstdlib>
#include <assert.h>
#include <vector>

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

	VkInstance			_instance			= nullptr;
	VkPhysicalDevice	_gpu				= nullptr;
	VkDevice			_device				= nullptr;
	VkPhysicalDeviceProperties   _gpuProperties = {};

	uint32_t			_graphicsFamilyIndex = 0;
};

#endif // __SenRenderer__