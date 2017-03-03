#include "SenRenderer.h"

SenRenderer::SenRenderer()
{
	_InitInstance();
}


SenRenderer::~SenRenderer()
{
	_DeInitInstance();
}

void SenRenderer::_InitInstance()
{
	VkApplicationInfo applicationInfo{};
	applicationInfo.sType					= VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.apiVersion				= VK_MAKE_VERSION(1, 0, 0);
	applicationInfo.applicationVersion		= VK_MAKE_VERSION(0, 1, 0);
	applicationInfo.pApplicationName = "Sen Learn Vulkan";

	VkInstanceCreateInfo instanceCreateInfo {};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pApplicationInfo = &applicationInfo;

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
		std::vector<VkPhysicalDevice> gpu_list( gpu_count);
		vkEnumeratePhysicalDevices(_instance, &gpu_count, gpu_list.data());
		_gpu = gpu_list[0];

	}

	VkDeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.

	vkCreateDevice( _gpu, ,nullptr, &_device);
}
