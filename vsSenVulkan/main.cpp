//
//#include "VulkanAPI/SenRenderer.h" // Video Tutorial
//
//int main() 
//{
//	SenRenderer renderer;
//
//	//VkCommandPool cmdPool;
//	//VkCommandPoolCreateInfo createInfo{};
//	//createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
//	//createInfo.queueFamilyIndex = renderer._graphicsFamilyIndex;
//
//	//vkCreateCommandPool(renderer._device, &createInfo, nullptr, &cmdPool);
//
//	return 0;
//}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Support/SenAbstractGLFW.h"
#include <functional>


int main() {
	SenAbstractGLFW widget;

	try {
		widget.showWidget();
	}
	catch (const std::runtime_error& e) {
		std::cerr << e.what() << std::endl;

#ifdef _DEBUG
		OutputDebugString("\n\n");
		OutputDebugString(e.what());
		MessageBox(NULL, e.what(), "Vulkan Error!", 0);
#endif

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}


//#include "SenVulkanTutorial/Tutorial_Triangle.h"
//
//int main() {
//	Tutorial_Triangle widget;
//
//	try {
//		widget.run();
//	}
//	catch (const std::runtime_error& e) {
//		std::cerr << e.what() << std::endl;
//		return EXIT_FAILURE;
//	}
//
//	return EXIT_SUCCESS;
//}