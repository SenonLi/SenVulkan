#include "SenAbstractGLFW.h"

SenAbstractGLFW::SenAbstractGLFW()
//:xRot(0), yRot(0), aspect(1.0)
{
	//showAllSupportedInstanceExtensions(); // Not Useful Functions
	//showAllSupportedInstanceLayers(); // Not Useful Functions
	//showAllSupportedExtensionsEachUnderInstanceLayer(); // Not Useful Functions

	strWindowName = "Sen GLFW Vulkan Application";

	widgetWidth = DEFAULT_widgetWidth;
	widgetHeight = DEFAULT_widgetHeight;
}

SenAbstractGLFW::~SenAbstractGLFW()
{
	OutputDebugString("\n ~SenAbstractGLWidget()\n");
}

//void SenAbstractGLFW::paintVulkan(void)
//{
//	// Define the viewport dimensions
//	glfwGetFramebufferSize(widgetGLFW, &widgetWidth, &widgetHeight);
//	glViewport(0, 0, widgetWidth, widgetHeight);
//}

void SenAbstractGLFW::initGlfwVulkan()
{
	// Init GLFW
	glfwInit();
	// Set all the required options for GLFW
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); //tell GLFW to not create an OpenGL context 
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	// Create a GLFWwindow object that we can use for GLFW's functions
	widgetGLFW = glfwCreateWindow(widgetWidth, widgetHeight, strWindowName, nullptr, nullptr);
	glfwSetWindowPos(widgetGLFW, 400, 240);
	glfwMakeContextCurrent(widgetGLFW);

	/*****************************************************************************************************************************/

	// Set the required callback functions
	//keyboardRegister();

	if (layersEnabled) {
		initDebugLayers();
	}
	initExtensions();
	createInstance();
	if (layersEnabled) {
		initDebugReportCallback();
	}

	// Clear the colorbuffer
	//glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	//glViewport(0, 0, widgetWidth, widgetHeight);
	//glEnable(GL_DEPTH_TEST);

	std::cout << "\n Finish Initial initGlfwVulkan\n";
}

void SenAbstractGLFW::showWidget()
{
	initGlfwVulkan();

	// Game loop
	while (!glfwWindowShouldClose(widgetGLFW))
	{
		// Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
		glfwPollEvents();

		// Render
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//paintVulkan();

		// Swap the screen buffers
		//glfwSwapBuffers(widgetGLFW);
	}


	finalize();// all the clean up works

	// Terminate GLFW, clearing any resources allocated by GLFW.
	glfwDestroyWindow(widgetGLFW);
	glfwTerminate();
}


VKAPI_ATTR VkBool32 VKAPI_CALL SenAbstractGLFW::pfnDebugCallback(VkFlags msgFlags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject, size_t location, int32_t msgCode, const char * layerPrefix, const char * msg, void * userData)
{
	std::ostringstream stream;
	if (msgFlags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) { stream << "INFO:\t"; }
	if (msgFlags & VK_DEBUG_REPORT_WARNING_BIT_EXT) { stream << "WARNING:\t"; }
	if (msgFlags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) { stream << "PERFORMANCE:\t"; }
	if (msgFlags & VK_DEBUG_REPORT_ERROR_BIT_EXT) { stream << "ERROR:\t"; }
	if (msgFlags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) { stream << "DEBUG:\t"; }
	stream << "@[" << layerPrefix << "]: \t";
	stream << msg << std::endl;
	std::cout << stream.str();

#ifdef _WIN32
	if (msgFlags & VK_DEBUG_REPORT_ERROR_BIT_EXT)	MessageBox(NULL, stream.str().c_str(), "Vulkan Error!", 0);
#endif

	return VK_FALSE;
}

/*******************************************************************
* 1. Call initDebugLayers() before createInstance() to track the instance creation procedure.
* Sum:
*********************************************************************/
void SenAbstractGLFW::initDebugLayers()
{
	// choose layers
	debugInstanceLayersVector.push_back("VK_LAYER_LUNARG_standard_validation");
	debugDeviceLayersVector.push_back("VK_LAYER_LUNARG_standard_validation");				// depricated

	// check layer support
	if (!checkInstanceLayersSupport(debugInstanceLayersVector)) {
		throw std::runtime_error("Layer Support Error!");
	}

	// init Debug report callback
	debugReportCallbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
	debugReportCallbackCreateInfo.pfnCallback = pfnDebugCallback;
	debugReportCallbackCreateInfo.flags =
		//VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
		VK_DEBUG_REPORT_WARNING_BIT_EXT |
		VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
		VK_DEBUG_REPORT_ERROR_BIT_EXT |
		//VK_DEBUG_REPORT_DEBUG_BIT_EXT |
		0;
}


void SenAbstractGLFW::initExtensions()
{
	uint32_t glfwInstanceExtensionsCount = 0;
	const char** glfwInstanceExtensions;

	glfwInstanceExtensions = glfwGetRequiredInstanceExtensions(&glfwInstanceExtensionsCount);

	//OutputDebugString("\nGLFW required Vulkan Instance Extensions: \n");
	for (uint32_t i = 0; i < glfwInstanceExtensionsCount; i++) {
		debugInstanceExtensionsVector.push_back(glfwInstanceExtensions[i]);
		//std::string strExtension = std::to_string(i) + ". " + std::string(glfwInstanceExtensions[i]) + "\n";
		//OutputDebugString(strExtension.c_str());
	}

	if (layersEnabled) {
		debugInstanceExtensionsVector.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	}

	debugDeviceExtensionsVector.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME); //There is no quote here
}

/*******************************************************************
* 1. vkCreateInstance requires VkInstanceCreateInfo
* 2. VkInstanceCreateInfo requires VkApplicationInfo, setup of extensions (instance and device) and layers
* Sum: createInstance requires basic appInfo, InstanceExtensionInfo, DeviceExtensionInfo and LayerInfo
*********************************************************************/
void SenAbstractGLFW::createInstance()
{
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Sen Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);

	VkInstanceCreateInfo instanceCreateInfo = {};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pApplicationInfo = &appInfo;

	instanceCreateInfo.enabledExtensionCount = (uint32_t)debugInstanceExtensionsVector.size();
	instanceCreateInfo.ppEnabledExtensionNames = debugInstanceExtensionsVector.data();

	if (layersEnabled) {
		instanceCreateInfo.enabledLayerCount = (uint32_t)debugInstanceLayersVector.size();
		instanceCreateInfo.ppEnabledLayerNames = debugInstanceLayersVector.data();
		instanceCreateInfo.pNext = &debugReportCallbackCreateInfo;
	}

	errorCheck(
		vkCreateInstance(&instanceCreateInfo, nullptr, &instance),
		std::string("Failed to create instance! \t Error:\t")
	);

	//if (vkCreateInstance(&instanceCreateInfo, nullptr, &instance) != VK_SUCCESS) {
	//	std::string errString = std::string("Failed to create instance!");
	//	throw std::runtime_error(errString);
	//}
}

void SenAbstractGLFW::initDebugReportCallback()
{
	fetch_vkCreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT"));
	fetch_vkDestroyDebugReportCallbackEXT = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT"));

	if (nullptr == fetch_vkCreateDebugReportCallbackEXT || nullptr == fetch_vkDestroyDebugReportCallbackEXT) {
		throw std::runtime_error("Vulkan Error: Can't fetch debug function pointers.");
		std::exit(-1);
	}

	errorCheck(
		fetch_vkCreateDebugReportCallbackEXT(instance, &debugReportCallbackCreateInfo, nullptr, &debugReportCallback),
		std::string("Create debugReportCallback Error!")
	);

}

void SenAbstractGLFW::finalize() {
	if (layersEnabled) {
		//Must destroy debugReportCallback before destroy instance
		fetch_vkDestroyDebugReportCallbackEXT(instance, debugReportCallback, nullptr);
		debugReportCallback = VK_NULL_HANDLE;
	}

	// Destroy Instance
	vkDestroyInstance(instance, nullptr); 	instance = nullptr;
}



bool SenAbstractGLFW::checkInstanceLayersSupport(std::vector<const char*> layersVector) {
	uint32_t layersCount;
	vkEnumerateInstanceLayerProperties(&layersCount, nullptr);

	std::vector<VkLayerProperties> supportedInstanceLayersVector(layersCount);
	vkEnumerateInstanceLayerProperties(&layersCount, supportedInstanceLayersVector.data());

	for (const char* layerNameToTest : layersVector) {
		bool layerFound = false;

		for (const auto& supportedLayerProperties : supportedInstanceLayersVector) {
			if (std::strcmp(layerNameToTest, supportedLayerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			return false;
		}
	}

	return true;
}




void SenAbstractGLFW::errorCheck(VkResult result, std::string msg)
{
	if (result != VK_SUCCESS) {

		std::string errString;
		switch (result)
		{
		case VK_NOT_READY:
			errString = "VK_NOT_READY    \n";
			break;
		case VK_TIMEOUT:
			errString = "VK_TIMEOUT    \n";
			break;
		case VK_EVENT_SET:
			errString = "VK_EVENT_SET    \n";
			break;
		case VK_EVENT_RESET:
			errString = "VK_EVENT_RESET    \n";
			break;
		case VK_INCOMPLETE:
			errString = "VK_INCOMPLETE    \n";
			break;

		case VK_ERROR_OUT_OF_HOST_MEMORY:
			errString = "VK_ERROR_OUT_OF_HOST_MEMORY    \n";
			break;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:
			errString = "VK_ERROR_OUT_OF_DEVICE_MEMORY    \n";
			break;
		case VK_ERROR_INITIALIZATION_FAILED:
			errString = "VK_ERROR_INITIALIZATION_FAILED    \n";
			break;
		case VK_ERROR_DEVICE_LOST:
			errString = "VK_ERROR_DEVICE_LOST    \n";
			break;
		case VK_ERROR_MEMORY_MAP_FAILED:
			errString = "VK_ERROR_MEMORY_MAP_FAILED    \n";
			break;
		case VK_ERROR_LAYER_NOT_PRESENT:
			errString = "VK_ERROR_LAYER_NOT_PRESENT    \n";
			break;
		case VK_ERROR_EXTENSION_NOT_PRESENT:
			errString = "VK_ERROR_EXTENSION_NOT_PRESENT    \n";
			break;
		case VK_ERROR_FEATURE_NOT_PRESENT:
			errString = "VK_ERROR_FEATURE_NOT_PRESENT    \n";
			break;
		case VK_ERROR_INCOMPATIBLE_DRIVER:
			errString = "VK_ERROR_INCOMPATIBLE_DRIVER    \n";
			break;
		case VK_ERROR_TOO_MANY_OBJECTS:
			errString = "VK_ERROR_TOO_MANY_OBJECTS    \n";
			break;
		case VK_ERROR_FORMAT_NOT_SUPPORTED:
			errString = "VK_ERROR_FORMAT_NOT_SUPPORTED    \n";
			break;
		case VK_ERROR_FRAGMENTED_POOL:
			errString = "VK_ERROR_FRAGMENTED_POOL    \n";
			break;
		case VK_ERROR_SURFACE_LOST_KHR:
			errString = "VK_ERROR_SURFACE_LOST_KHR    \n";
			break;
		case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
			errString = "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR    \n";
			break;
		case VK_SUBOPTIMAL_KHR:
			errString = "VK_SUBOPTIMAL_KHR    \n";
			break;
		case VK_ERROR_OUT_OF_DATE_KHR:
			errString = "VK_ERROR_OUT_OF_DATE_KHR    \n";
			break;
		case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
			errString = "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR    \n";
			break;
		case VK_ERROR_VALIDATION_FAILED_EXT:
			errString = "VK_ERROR_VALIDATION_FAILED_EXT    \n";
			break;
		case VK_ERROR_INVALID_SHADER_NV:
			errString = "VK_ERROR_INVALID_SHADER_NV    \n";
			break;
		case VK_ERROR_OUT_OF_POOL_MEMORY_KHR:
			errString = "VK_ERROR_OUT_OF_POOL_MEMORY_KHR    \n";
			break;
		case VK_ERROR_INVALID_EXTERNAL_HANDLE_KHX:
			errString = "VK_ERROR_INVALID_EXTERNAL_HANDLE_KHX    \n";
			break;
		default:
			std::cout << result << std::endl;
			errString = "\n\n Cannot tell error type, check std::cout!    \n";
			break;
		}

		throw std::runtime_error(msg + errString);
	}
}


//*************************************************/
//*************                       *************/
//*************  Not Useful Functions *************/
//*************       Below           *************/
//*************************************************/
void SenAbstractGLFW::showAllSupportedInstanceExtensions()
{
	uint32_t extensionsCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, nullptr);

	std::vector<VkExtensionProperties> instanceExtensionsVector(extensionsCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, instanceExtensionsVector.data());

	std::ostringstream stream;
	stream << "\nAll Supported Instance Extensions: \n";
	for (uint32_t i = 0; i < extensionsCount; i++) {
		stream << "\t" << i+1 << ". " << instanceExtensionsVector[i].extensionName << "\n";
	}	
	std::cout << stream.str();
}

void SenAbstractGLFW::showAllSupportedInstanceLayers()
{
	uint32_t layersCount = 0;
	vkEnumerateInstanceLayerProperties(&layersCount, nullptr);

	std::vector<VkLayerProperties> instanceLayersVector(layersCount);
	vkEnumerateInstanceLayerProperties(&layersCount, instanceLayersVector.data());

	std::ostringstream stream;
	stream << "\nAll Supported Instance Layers: \n";
	for (uint32_t i = 0; i < layersCount; i++) {
		stream << "\t" << i+1 << ". " << instanceLayersVector[i].layerName << "\n";
	}
	std::cout << stream.str();
}

void SenAbstractGLFW::showAllSupportedExtensionsEachUnderInstanceLayer()
{
	uint32_t layersCount = 0;
	vkEnumerateInstanceLayerProperties(&layersCount, nullptr);

	std::vector<VkLayerProperties> instanceLayersVector(layersCount);
	vkEnumerateInstanceLayerProperties(&layersCount, instanceLayersVector.data());

	std::ostringstream stream;
	stream << "\nAll Supported Instance Extensions under Layers: \n";
	for (uint32_t i = 0; i < layersCount; i++) {
		stream << "\t" << i << ". " << instanceLayersVector[i].description << "\t\t\t\t [\t" << instanceLayersVector[i].layerName << "\t]\n";

		uint32_t extensionsCount = 0;
		errorCheck(vkEnumerateInstanceExtensionProperties(instanceLayersVector[i].layerName, &extensionsCount, NULL), std::string("Extension Count under Layer Error!"));
		std::vector<VkExtensionProperties> extensionsPropVec(extensionsCount);
		errorCheck(vkEnumerateInstanceExtensionProperties(instanceLayersVector[i].layerName, &extensionsCount, extensionsPropVec.data()), std::string("Extension Data() under Layer Error!"));

		if (extensionsPropVec.size()) {
			for (int j = 0; j < extensionsPropVec.size(); j++) {
				stream << "\t\t\t|-- [ " << j+1 << " ] . " << extensionsPropVec[j].extensionName << "\n";
			}
		}
		else {
			stream << "\t\t\t|-- [ 0 ] .  None  \n";
		}
		stream << std::endl;
	}
	std::cout << stream.str();
}




//SenAbstractGLFW* currentInstance;
//
//extern "C" void _KeyDetection(GLFWwindow* widget, int key, int scancode, int action, int mode)
//{
//	currentInstance->_protectedKeyDetection(widget, key, scancode, action, mode);
//}
//
//void SenAbstractGLFW::keyboardRegister()
//{
//	::currentInstance = this;
//	::glfwSetKeyCallback(widgetGLFW, _KeyDetection);
//
//}
//
//void SenAbstractGLFW::keyDetection(GLFWwindow* widget, int key, int scancode, int action, int mode)
//{
//	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
//		glfwSetWindowShouldClose(widget, GL_TRUE);
//}
//
//
