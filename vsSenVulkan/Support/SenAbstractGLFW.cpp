#include "SenAbstractGLFW.h"

SenAbstractGLFW::SenAbstractGLFW()
	//:xRot(0), yRot(0), aspect(1.0)
{
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




	// Clear the colorbuffer
	//glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	//glViewport(0, 0, widgetWidth, widgetHeight);
	//glEnable(GL_DEPTH_TEST);

	OutputDebugString("\n Finish Initial initGlfwVulkan\n");
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