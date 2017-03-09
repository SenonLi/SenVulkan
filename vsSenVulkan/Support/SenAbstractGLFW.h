#pragma once

#ifndef __SenAbstractGLFW__
#define __SenAbstractGLFW__


#include <iostream> // for cout, cin
#include <stdexcept> // for propagating errors 
#include <stdlib.h> // for const error catch
#include <stdio.h>  
#include <Windows.h> // for OutputDebugString() function
#include <vector>
#include <string>

#define GLFW_INCLUDE_VULKAN // Let GLFW know Vulkan is utilized
#include <GLFW/glfw3.h>

//#include "LoadShaders.h"

class SenAbstractGLFW
{
public:
	SenAbstractGLFW();
	virtual ~SenAbstractGLFW();

	void showWidget();

//	void _protectedKeyDetection(GLFWwindow* widget, int key, int scancode, int action, int mode) { 
//		keyDetection(widget, key, scancode, action, mode);
//	}

protected:
	GLFWwindow* widgetGLFW;
	int widgetWidth, widgetHeight;
	char* strWindowName;

	VkInstance			_instance = nullptr;

//	float xRot, yRot;
//	float aspect;

	virtual void initGlfwVulkan();
//	virtual void paintVulkan();
	virtual void finalize();

//	virtual void keyDetection(GLFWwindow* widget, int key, int scancode, int action, int mode);

//	// shader info variables
//	GLuint programA, programB;
//	GLuint defaultTextureID;
//	int textureLocation;

//	GLuint verArrObjArray[2];
//	GLuint verBufferObjArray[2];
//	GLuint verIndicesObjArray[2];

//	int modelMatrixLocation;
//	int projectionMatrixLocation;

	const int DEFAULT_widgetWidth = 800;	// 640;
	const int DEFAULT_widgetHeight = 600; // 640;

private:

	void createInstance();


	std::vector<const char*> getInstanceExtensions();




	void showVulkanSupportedInstanceExtensions();





//	void keyboardRegister();
//
};


#endif //__SenAbstractGLFW__