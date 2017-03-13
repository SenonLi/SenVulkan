#include "SenAbstractGLFW.h"

SenAbstractGLFW::SenAbstractGLFW()
	//:xRot(0), yRot(0), aspect(1.0)
{
	strWindowName = "Sen GLFW Vulkan Application";

	widgetWidth = DEFAULT_widgetWidth;
	widgetHeight = DEFAULT_widgetHeight;

	expectedInstanceLayersVector.push_back("VK_LAYER_LUNARG_standard_validation");
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
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, SenGL_MajorVersion);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, SenGL_MinorVersion);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); //tell GLFW to not create an OpenGL context 
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	// Create a GLFWwindow object that we can use for GLFW's functions
	widgetGLFW = glfwCreateWindow(widgetWidth, widgetHeight, strWindowName, nullptr, nullptr);
	glfwSetWindowPos(widgetGLFW, 400, 240);
	glfwMakeContextCurrent(widgetGLFW);


#ifdef _DEBUG
	// comment this line in a release build! 
#endif

	// Set the required callback functions
	//keyboardRegister();

	createInstance();


	// Clear the colorbuffer
	//glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	//glViewport(0, 0, widgetWidth, widgetHeight);
	//glEnable(GL_DEPTH_TEST);

	std::cout << "\n Finish Initial initGlfwVulkan\n";
}

void SenAbstractGLFW::showWidget()
{
	//showVulkanSupportedInstanceExtensions(); // Not Useful Functions

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

/*******************************************************************
* 1. vkCreateInstance requires VkInstanceCreateInfo
* 2. VkInstanceCreateInfo requires VkApplicationInfo, setup of extensions (instance and device) and layers
* Sum: createInstance requires basic appInfo, InstanceExtensionInfo, DeviceExtensionInfo and LayerInfo
*********************************************************************/
void SenAbstractGLFW::createInstance()
{
	if (instanceLayersEnabled && !checkInstanceLayersSupport()) {
		throw std::runtime_error("Validation layers requested, but not available!");
	}

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Sen Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	
	std::vector<const char*> instanceExtensionsVector = getRequiredInstanceExtensions(); // Combine InstanceExtensions in need
	createInfo.enabledExtensionCount = (uint32_t)instanceExtensionsVector.size();
	createInfo.ppEnabledExtensionNames = instanceExtensionsVector.data();

	if (instanceLayersEnabled) {
		createInfo.enabledLayerCount = (uint32_t)expectedInstanceLayersVector.size();
		createInfo.ppEnabledLayerNames = expectedInstanceLayersVector.data();
	}
	
	if (vkCreateInstance(&createInfo, nullptr, &_instance) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create instance!");
	}
}

bool SenAbstractGLFW::checkInstanceLayersSupport() {
	uint32_t layersCount;
	vkEnumerateInstanceLayerProperties(&layersCount, nullptr);

	std::vector<VkLayerProperties> availableInstanceLayers(layersCount);
	vkEnumerateInstanceLayerProperties(&layersCount, availableInstanceLayers.data());

	for (const char* expectedLayerName : expectedInstanceLayersVector) {
		bool layerFound = false;

		for (const auto& availableLayerProperties : availableInstanceLayers) {
			if (strcmp(expectedLayerName, availableLayerProperties.layerName) == 0) {
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

/*******************************************************************
* 1. GLFW for Vulkan creating window
* 2. glfw offer "const char**", we create and return vector<const char*>
* Sum: other instanceExtensions could also be added into this vector
*********************************************************************/
std::vector<const char *> SenAbstractGLFW::getRequiredInstanceExtensions()
{
	std::vector<const char*> instanceExtensionsVector;

	uint32_t glfwInstanceExtensionsCount = 0;
	const char** glfwInstanceExtensions;
	
	glfwInstanceExtensions = glfwGetRequiredInstanceExtensions(&glfwInstanceExtensionsCount);

	//OutputDebugString("\nGLFW required Vulkan Instance Extensions: \n");
	for (uint32_t i = 0; i < glfwInstanceExtensionsCount; i++) {
		instanceExtensionsVector.push_back(glfwInstanceExtensions[i]);
		//std::string strExtension = std::to_string(i) + ". " + std::string(glfwInstanceExtensions[i]) + "\n";
		//OutputDebugString(strExtension.c_str());
	}

	if (instanceLayersEnabled) {
		instanceExtensionsVector.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	}

	return instanceExtensionsVector;
}


void SenAbstractGLFW::finalize() {
	vkDestroyInstance(_instance, nullptr); //	_instance = nullptr;
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









//*************************************************/
//*************                       *************/
//*************  Not Useful Functions *************/
//*************       Below           *************/
//*************************************************/
void SenAbstractGLFW::showVulkanSupportedInstanceExtensions()
{
	uint32_t extensionsCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, nullptr);

	std::vector<VkExtensionProperties> instanceExtensions(extensionsCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, instanceExtensions.data());

	std::cout << "\nVulkan Supported Instance Extensions: \n";
	for (uint32_t i = 0; i < extensionsCount; i++) {
		std::string strExtension = std::to_string(i) + ". " + std::string(instanceExtensions[i].extensionName) + "\n";
		std::cout << strExtension.c_str();
	}
}
