#pragma once

#ifndef __SenVulkanAPI_Shared__
#define __SenVulkanAPI_Shared__



#define _SenENABLE_VULKAN_DEBUG				1
#define _SenENABLE_VULKAN_RUNTIME_DEBU		1



#if defined( _WIN32 )	// on Windows OS

#include <Windows.h>	// for OutputDebugString() function
#define VK_USE_PLATFORM_WIN32_KHR 1 // For Vulkan Surface if not using GLFW

#elif defined( __linux ) // on Linux ( Via XCB library )
// xcb seems like a popular and well supported option on X11, until wayland and mir take over
#define VK_USE_PLATFORM_XCB_KHR 1
#include <xcb/xcb.h>
#endif


#include <iostream>
#include <assert.h>
#include <vulkan/vulkan.h>

void ErrorCheck(VkResult result);




#endif // !__SenVulkanAPI_Shared__
