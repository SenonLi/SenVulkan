#pragma once

#ifndef __SenRenderer__
#define __SenRenderer__

#include <cstdlib>
#include <assert.h>
#include <vector>
#include <iostream>
#include <sstream>

#include "Shared.h"

#include <vulkan/vulkan.h>

class SenWindow;

class SenRenderer
{
public:
	SenRenderer();
	virtual ~SenRenderer();

	void finalize();
	void closeSenWindow();
	SenWindow*	openSenWindow(uint32_t size_X, uint32_t size_Y, std::string name);
	bool		run();

	VkInstance			_instance	= VK_NULL_HANDLE;
	VkPhysicalDevice	_gpu		= VK_NULL_HANDLE;
	VkDevice			_device		= VK_NULL_HANDLE;
	VkQueue				_queue		= VK_NULL_HANDLE;

	VkPhysicalDeviceProperties			_gpuProperties = {};
	VkPhysicalDeviceMemoryProperties	_gpuMemoryProperties = {};
	uint32_t							_graphicsFamilyIndex = 0;

	SenWindow					*_window = nullptr;

	const VkInstance						getInstance()	const						{	return _instance; }
	const VkPhysicalDevice					getPhysicalDevice() const					{	return _gpu; }
	const VkDevice							getDevice() const							{	return _device; }
	const VkQueue							getQueue() const							{	return _queue; }
	const uint32_t							getGraphicsQueueFamilyIndex() const			{	return _graphicsFamilyIndex; }
	const VkPhysicalDeviceProperties&		getPhysicalDeviceProperties() const			{	return _gpuProperties; }
	const VkPhysicalDeviceMemoryProperties&	getPhysicalDeviceMemoryProperties() const	{	return _gpuMemoryProperties; }

private:
	void _InitInstance();
	void _DeInitInstance();

	void _InitDevice();
	void _DeInitDevice();

	void _SetupDebug();
	void _InitDebug();
	void _DeInitDebug();

	std::vector<const char*>	_instanceLayersList;
	std::vector<const char*>	_instanceExtensionsList;
	std::vector<const char*>	_deviceLayersList;
	std::vector<const char*>	_deviceExtensionsList;

	PFN_vkCreateDebugReportCallbackEXT	fetch_vkCreateDebugReportCallbackEXT = VK_NULL_HANDLE;
	PFN_vkDestroyDebugReportCallbackEXT	fetch_vkDestroyDebugReportCallbackEXT = VK_NULL_HANDLE;

	VkDebugReportCallbackEXT	_debugReport = VK_NULL_HANDLE;
	VkDebugReportCallbackCreateInfoEXT debugCallbackCreateInfo = {};
};

#endif // __SenRenderer__