
#include "SenRenderer.h"
#include "SenWindow.h"
#include "Shared.h"

SenRenderer::SenRenderer()
{
	_SetupDebug();
	_InitInstance();
	_InitDebug();
	_InitDevice();
}


SenRenderer::~SenRenderer()
{
}

void SenRenderer::closeSenWindow()
{
	if (_window) {
		delete _window;
		_window = nullptr;
	}
}

void SenRenderer::finalize()
{
	_DeInitDevice();
	_DeInitDebug();
	_DeInitInstance();
}

SenWindow * SenRenderer::openSenWindow(uint32_t size_X, uint32_t size_Y, std::string name)
{
	_window = new SenWindow(this, size_X, size_Y, name);
	return		_window;
}

bool SenRenderer::run()
{
	if (nullptr != _window) {
		return _window->updateSenWindow();
	}
	return true;
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
	instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(_instanceLayersList.size());
	instanceCreateInfo.ppEnabledLayerNames = _instanceLayersList.data();

	instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(_instanceExtensionsList.size());
	instanceCreateInfo.ppEnabledExtensionNames = _instanceExtensionsList.data();

	instanceCreateInfo.pNext = &debugCallbackCreateInfo;

	ErrorCheck(vkCreateInstance(&instanceCreateInfo, nullptr, &_instance));
	//if (VK_SUCCESS != error) {
	//	assert(0 && "Vulkan ERROR: Create instance failed.");
	//	std::exit(-1);
	//}
}

void SenRenderer::_DeInitInstance()
{
	if (nullptr != _instance) {
		vkDestroyInstance(_instance, nullptr);
		_instance = nullptr;
	}
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
		vkGetPhysicalDeviceMemoryProperties(_gpu, &_gpuMemoryProperties);
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

	//deviceCreateInfo.enabledLayerCount = _deviceLayersList.size();   				// depricated
	//deviceCreateInfo.ppEnabledLayerNames = _deviceLayersList.data();				// depricated

	deviceCreateInfo.enabledExtensionCount = (uint32_t)_deviceExtensionsList.size();
	deviceCreateInfo.ppEnabledExtensionNames = _deviceExtensionsList.data();

	ErrorCheck(vkCreateDevice(_gpu, &deviceCreateInfo, nullptr, &_device));


	vkGetDeviceQueue(_device, _graphicsFamilyIndex, 0, &_queue);
}

void SenRenderer::_DeInitDevice()
{
	if (nullptr != _device)
	{
		vkDestroyDevice(_device, nullptr);
		_device = nullptr;
	}
	if (nullptr != _device)
	{
		vkDestroyDevice(_device, nullptr);
		_device = nullptr;
	}
}

#if _SenENABLE_VULKAN_DEBUG

VKAPI_ATTR VkBool32 VKAPI_CALL
VulkanDebugCallback(
	VkDebugReportFlagsEXT		msgFlags,
	VkDebugReportObjectTypeEXT	objType,
	uint64_t					srcObj,
	size_t						location,
	int32_t						msgCode,
	const char *				layerPrefix,
	const char *				msg,
	void *						userData	)
{
	std::ostringstream stream;
	if (msgFlags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)		{			stream << "INFO:\t";		}
	if (msgFlags & VK_DEBUG_REPORT_WARNING_BIT_EXT)			{			stream << "WARNING:\t";		}
	if (msgFlags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {		stream << "PERFORMANCE:\t";	}
	if (msgFlags & VK_DEBUG_REPORT_ERROR_BIT_EXT)			{			stream << "ERROR:\t";		}
	if (msgFlags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)			{			stream << "DEBUG:\t";		}
	stream << "@[" << layerPrefix << "]: \t";
	stream << msg << std::endl;
	std::cout << stream.str();

#ifdef _WIN32
	if(msgFlags & VK_DEBUG_REPORT_ERROR_BIT_EXT)	MessageBox(NULL, stream.str().c_str(), "Vulkan Error!", 0);
#endif

	return VK_FALSE;
}


void SenRenderer::_SetupDebug()
{
	debugCallbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
	debugCallbackCreateInfo.pfnCallback = VulkanDebugCallback;
	debugCallbackCreateInfo.flags =
		//VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
		VK_DEBUG_REPORT_WARNING_BIT_EXT |
		VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
		VK_DEBUG_REPORT_ERROR_BIT_EXT |
		//VK_DEBUG_REPORT_DEBUG_BIT_EXT |
		0;

	_instanceLayersList.push_back("VK_LAYER_LUNARG_standard_validation");

	_instanceExtensionsList.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

#if defined( _WIN32 )	// on Windows OS
	_instanceExtensionsList.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);

#elif defined( __linux ) // on Linux ( Via XCB library )
	_instanceExtensionsList.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#endif


	_instanceExtensionsList.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME); //There is no quote here

	_deviceLayersList.push_back("VK_LAYER_LUNARG_standard_validation");

	_deviceExtensionsList.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME); //There is no quote here
}

//PFN_vkCreateDebugReportCallbackEXT	fetch_vkCreateDebugReportCallbackEXT = nullptr;
//PFN_vkDestroyDebugReportCallbackEXT	fetch_vkDestroyDebugReportCallbackEXT = nullptr;

void SenRenderer::_InitDebug()
{
	fetch_vkCreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(_instance, "vkCreateDebugReportCallbackEXT"));
	fetch_vkDestroyDebugReportCallbackEXT = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr(_instance, "vkDestroyDebugReportCallbackEXT"));
	
	if (nullptr == fetch_vkCreateDebugReportCallbackEXT || nullptr == fetch_vkDestroyDebugReportCallbackEXT) {
		assert(0 && "Vulkan Error: Can't fetch debug function pointers.");
		std::exit(-1);
	}
	
	fetch_vkCreateDebugReportCallbackEXT(_instance, &debugCallbackCreateInfo, nullptr, &_debugReport);
}

void SenRenderer::_DeInitDebug()
{
	if (VK_NULL_HANDLE != _debugReport) {
		fetch_vkDestroyDebugReportCallbackEXT(_instance, _debugReport, nullptr);
		_debugReport = VK_NULL_HANDLE;
	}
}

#else

void SenRenderer::_SetupDebug() {};
void SenRenderer::_InitDebug() {};
void SenRenderer::_DeInitDebug() {};

#endif // _SenENABLE_VULKAN_DEBUG