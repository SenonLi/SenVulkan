//#include "VulkanAPI/SenVulkanAPI_Widget.h" // Video Tutorial
//
//int main() 
//{
//	SenVulkanAPI_Widget widget;
//	widget.showWidget();
//
//	return 0;
//}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "SenVulkanTutorial/Sen_06_Triangle.h"
#include "SenVulkanTutorial/Sen_07_Texture.h"
#include "SenVulkanTutorial/Sen_072_TextureArray.h"
#include "SenVulkanTutorial/Sen_22_DepthTest.h"
#include "SenVulkanTutorial/Sen_221_Cube.h"
//#include <functional>

SenAbstractGLFW* widget;
int main() {
	widget = new Sen_072_TextureArray();
	try {
		widget->showWidget();
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

	delete(widget);
	return EXIT_SUCCESS;
}
