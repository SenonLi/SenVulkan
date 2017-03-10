#include "SenRenderer.h"

SenRenderer::SenRenderer()
{
	_SetupDebug();
	_InitInstance();
	_InitDevice();
}


SenRenderer::~SenRenderer()
{
	_DeInitDevice();

	_DeInitInstance();
}

void SenRenderer::_InitInstance()
{
	VkApplicationInfo applicationInfo={};
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);
	applicationInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
	applicationInfo.pApplicationName = "Sen Learn Vulkan";

	VkInstanceCreateInfo instanceCreateInfo{};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pApplicationInfo = &applicationInfo;
	instanceCreateInfo.enabledLayerCount = _instanceLayersList.size();
	instanceCreateInfo.ppEnabledLayerNames = _instanceLayersList.data();

	//instanceCreateInfo.enabledExtensionCount = _instanceExtensionsList.size();
	//instanceCreateInfo.ppEnabledExtensionNames = _instanceExtensionsList.data();

	auto error = vkCreateInstance(&instanceCreateInfo, nullptr, &_instance);
	if (VK_SUCCESS != error) {
		assert(0 && "Vulkan ERROR: Create instance failed.");
		std::exit(-1);
	}
}

void SenRenderer::_DeInitInstance()
{
	vkDestroyInstance(_instance, nullptr);
	_instance = nullptr;
}

void SenRenderer::_InitDevice()
{
	{
		uint32_t gpu_count = 0;
		vkEnumeratePhysicalDevices(_instance, &gpu_count, nullptr);
		std::vector<VkPhysicalDevice> gpu_list(gpu_count);
		vkEnumeratePhysicalDevices(_instance, &gpu_count, gpu_list.data());
		_gpu = gpu_list[0];
		vkGetPhysicalDeviceProperties(_gpu, &_gpuProperties);
	}
	{
		uint32_t familyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(_gpu, &familyCount, nullptr);
		std::vector<VkQueueFamilyProperties> familyPropertyList(familyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(_gpu, &familyCount, familyPropertyList.data());

		bool found = false;
		for (uint32_t i = 0; i < familyCount; i++) {
			if (familyPropertyList[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				found = true;
				_graphicsFamilyIndex = i;
			}
		}

		if (!found) {
			assert(0 && "Vulkan Error: Queue family supporting graphics not found.");
			std::exit(-1);
		}

	}

	{
		uint32_t layerCount = 0;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		std::vector<VkLayerProperties> layerPropertyList(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, layerPropertyList.data());

		std::cout << "Instance Layers: \n";
		for (auto &i : layerPropertyList) {
			std::cout << " " << i.layerName << "\t\t | " << i.description << std::endl;
		}
		std::cout << std::endl;
	}

	{
		uint32_t layerCount = 0;
		vkEnumerateDeviceLayerProperties(_gpu, &layerCount, nullptr);
		std::vector<VkLayerProperties> layerPropertyList(layerCount);
		vkEnumerateDeviceLayerProperties(_gpu, &layerCount, layerPropertyList.data());

		std::cout << "Device Layers: \n";
		for (auto &i : layerPropertyList) {
			std::cout << " " << i.layerName << "\t\t | " << i.description << std::endl;
		}
		std::cout << std::endl;
	}

	float queuePriorities[]{ 1.0f };
	VkDeviceQueueCreateInfo deviceQueueCreateInfo{};
	deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	deviceQueueCreateInfo.queueFamilyIndex = _graphicsFamilyIndex;
	deviceQueueCreateInfo.queueCount = 1;
	deviceQueueCreateInfo.pQueuePriorities = queuePriorities;

	VkDeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;

	deviceCreateInfo.enabledLayerCount = _deviceLayersList.size();
	deviceCreateInfo.ppEnabledLayerNames = _deviceLayersList.data();

	deviceCreateInfo.enabledExtensionCount = _deviceExtensionsList.size();
	deviceCreateInfo.ppEnabledExtensionNames = _deviceExtensionsList.data();

	auto error = vkCreateDevice(_gpu, &deviceCreateInfo, nullptr, &_device);
	if (VK_SUCCESS != error) {
		assert(0 && "Vulkan Error:Device creation failed.");
		std::exit(-1);
	}
}

void SenRenderer::_DeInitDevice()
{
	vkDestroyDevice(_device, nullptr);
	_device = nullptr;
}

void SenRenderer::_SetupDebug()
{
	_instanceLayersList.push_back("VK_LAYER_LUNARG_standard_validation");

	_instanceExtensionsList.push_back("VK_EXT_DEBUG_REPORT_EXTENSION_NAME");

	_deviceLayersList.push_back("VK_LAYER_LUNARG_standard_validation");
}

void SenRenderer::_InitDebug()
{
	//vkCreateDebugReportCallbackEXT(_instance, nullptr, nullptr, nullptr);
}
