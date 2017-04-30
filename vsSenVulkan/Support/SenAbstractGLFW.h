#pragma once

#ifndef __SenAbstractGLFW__
#define __SenAbstractGLFW__

#if defined( _WIN32 )		// on Windows OS

#include <Windows.h>		// for OutputDebugString() function
//#define VK_USE_PLATFORM_WIN32_KHR 1 // For Vulkan Surface if not using GLFW

#elif defined( __linux )	// on Linux ( Via XCB library )
// xcb seems like a popular and well supported option on X11, until wayland and mir take over
#define VK_USE_PLATFORM_XCB_KHR 1
#include <xcb/xcb.h>
#endif

#include <stdexcept>// for propagating errors 
#include <stdlib.h>	// for const error catch
#include <stdio.h>  
#include <iostream> // for cout, cin
#include <fstream>	// for readFileBinaryStream function
#include <sstream>  // for quick chunch cout
#include <map>		// for ratePhysicalDevice to pick the best
#include <set>		// for unique queue family <==> unique queue
#include <vector>
#include <array>
#include <string>


#define GLFW_INCLUDE_VULKAN // Let GLFW know Vulkan is utilized, must be in front of vulkan.h
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.h>
#include <algorithm>		// std::max, std::min

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>

//#include "LoadShaders.h"

class SenAbstractGLFW
{
public:
	SenAbstractGLFW();
	virtual ~SenAbstractGLFW();

	void showWidget();

	static VKAPI_ATTR VkBool32 VKAPI_CALL pfnDebugCallback(VkFlags, VkDebugReportObjectTypeEXT, uint64_t, size_t, int32_t, const char*, const char*, void *);
	static void onWidgetResized(GLFWwindow* widget, int width, int height);
	static std::vector<char> readFileBinaryStream(const std::string& filename);
	static uint32_t findPhysicalDeviceMemoryPropertyIndex(
		const VkPhysicalDeviceMemoryProperties& gpuMemoryProperties,
		const VkMemoryRequirements& memoryRequirements,
		const VkMemoryPropertyFlags& requiredMemoryPropertyFlags
	);
	static void errorCheck(VkResult result, std::string msg);
	static void createResourceBuffer(const VkDevice& logicalDevice, const VkDeviceSize& bufferDeviceSize,
		const VkBufferUsageFlags& bufferUsageFlags, const VkSharingMode& bufferSharingMode, const VkPhysicalDeviceMemoryProperties& gpuMemoryProperties,
		VkBuffer& bufferToCreate, VkDeviceMemory& bufferDeviceMemoryToAllocate, const VkMemoryPropertyFlags& requiredMemoryPropertyFlags);
	static void transferResourceBuffer(const VkCommandPool& bufferTransferCommandPool, const VkDevice& logicalDevice, const VkQueue& bufferTransferQueue,
		const VkBuffer& srcBuffer, const VkBuffer& dstBuffer, const VkDeviceSize& size);

//	void _protectedKeyDetection(GLFWwindow* widget, int key, int scancode, int action, int mode) { 
//		keyDetection(widget, key, scancode, action, mode);
//	}

protected:
	GLFWwindow* widgetGLFW;
	int widgetWidth, widgetHeight;
	char* strWindowName;

	VkInstance						instance					= VK_NULL_HANDLE;

	/*******************************************************************************************************************************/
	/********* VkSurfaceKHR object that represents an abstract type of surface to present rendered images to. **********************/
	/********* The surface in our program will be backed by the window that we've already opened with GLFW.   **********************/
	VkSurfaceKHR					surface						= VK_NULL_HANDLE; // VK_KHR_surface Instance Extension
	std::vector<VkSurfaceFormatKHR> surfaceFormatVector;
	VkSurfaceFormatKHR				surfaceFormat{};
	VkSurfaceCapabilitiesKHR		surfaceCapabilities{};
	
	VkPhysicalDevice				physicalDevice				= VK_NULL_HANDLE;
	VkPhysicalDeviceProperties		physicalDeviceProperties{};			// GPU name, type (discrete)
	int32_t							graphicsQueueFamilyIndex	= -1;	// Index of Graphics QueueFamily of GPU that we will choose to 
	int32_t							presentQueueFamilyIndex		= -1;	// The Graphics (Drawing) QueueFamily may not support presentation (WSI)

	VkDevice						device						= VK_NULL_HANDLE;
	VkQueue							graphicsQueue				= VK_NULL_HANDLE;			// Handle to the graphics queue
	VkQueue							presentQueue				= VK_NULL_HANDLE;			// Since presentQueueFamilyIndex may not == graphicsQueueFamilyIndex, make two queue
	VkPresentModeKHR				presentMode					= VK_PRESENT_MODE_FIFO_KHR; // VK_PRESENT_MODE_FIFO_KHR is always available.

	VkSwapchainKHR					swapChain					= VK_NULL_HANDLE;
	uint32_t						swapchainImagesCount		= 2;
	std::vector<VkImage>			swapchainImagesVector;
	std::vector<VkImageView>		swapchainImageViewsVector;
	std::vector<VkFramebuffer>		swapchainFramebufferVector;
	std::vector<VkCommandBuffer>	swapchainCommandBufferVector;

	VkCommandPool						defaultThreadCommandPool;

	VkRenderPass						triangleRenderPass				= VK_NULL_HANDLE;
	VkPipelineLayout					trianglePipelineLayout			= VK_NULL_HANDLE;
	VkPipeline							trianglePipeline				= VK_NULL_HANDLE;
	VkBuffer							triangleVertexBuffer			= VK_NULL_HANDLE;
	VkDeviceMemory						triangleVertexBufferMemory		= VK_NULL_HANDLE;
	VkBuffer							triangleIndexBuffer				= VK_NULL_HANDLE;
	VkDeviceMemory						triangleIndexBufferMemory		= VK_NULL_HANDLE;
	// It should be noted that in a real world application, you're not supposed to actually call vkAllocateMemory for every individual buffer.
	// The maximum number of simultaneous memory allocations is limited by the maxMemoryAllocationCount physical device limit, 
	//		which may be as low as 4096 even on high end hardware like an NVIDIA GTX 1080.
	// The right way to allocate memory for a large number of objects at the same time is to 
	//		create a custom allocator that splits up a single allocation among many different objects by using the offset parameters
	//		that we've seen in many functions.

	// As mentioned above that you should allocate multiple resources like buffers from a single memory allocation, 
	//		in fact, you should go a step further.
	//	Driver developers recommend that you also store multiple buffers, like the vertex and index buffer,
	//		into a single VkBuffer and use offsets in commands like vkCmdBindVertexBuffers.
	//	The advantage is that your data is more cache friendly in that case, because it's closer together.
	//	It is even possible to reuse the same chunk of memory for multiple resources if they are not used during the same render operations,
	//		provided that their data is refreshed, of course.
	// This is known as aliasing and some Vulkan functions have explicit flags to specify that you want to do this.
	
	VkSemaphore swapchainImageAcquiredSemaphore;// wait for SWI, from VK_IMAGE_LAYOUT_PRESENT_SRC_KHR to VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	VkSemaphore paintReadyToPresentSemaphore;	// wait for GPU, from VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL to VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
	

	struct MvpUniformBufferObject {
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 projection;
	};
	VkDescriptorSetLayout			mvpUboDescriptorSetLayout					= VK_NULL_HANDLE;
	VkBuffer						mvpUniformStagingBuffer				= VK_NULL_HANDLE;
	VkDeviceMemory					mvpUniformStagingBufferDeviceMemory	= VK_NULL_HANDLE;
	VkBuffer						mvpOptimalUniformBuffer				= VK_NULL_HANDLE;
	VkDeviceMemory					mvpOptimalUniformBufferMemory		= VK_NULL_HANDLE;
	VkDescriptorPool				descriptorPool						= VK_NULL_HANDLE;
	VkDescriptorSet					mvpUboDescriptorSet					= VK_NULL_HANDLE;


	VkRenderPass						depthTestRenderPass				= VK_NULL_HANDLE;
	VkPipelineLayout					depthTestPipelineLayout			= VK_NULL_HANDLE;
	VkPipeline							depthTestPipeline				= VK_NULL_HANDLE;

	VkImage								depthStencilImage				= VK_NULL_HANDLE;
	VkPhysicalDeviceMemoryProperties	physicalDeviceMemoryProperties{};
	VkDeviceMemory						depthStencilImageDeviceMemory	= VK_NULL_HANDLE;
	VkImageView							depthStencilImageView			= VK_NULL_HANDLE;
	VkFormat							depthStencilFormat				= VK_FORMAT_UNDEFINED;
	bool								stencilAvailable				= false;

	virtual void initGlfwVulkan();
	virtual void paintVulkan();
	virtual void reCreateTriangleSwapchain(); // for resize window
	virtual void finalize();

//	virtual void keyDetection(GLFWwindow* widget, int key, int scancode, int action, int mode);


	const int DEFAULT_widgetWidth = 800;	// 640;
	const int DEFAULT_widgetHeight = 600; // 640;

#ifdef _DEBUG
	const bool layersEnabled = true;
#else
	const bool layersEnabled = false;
#endif

private:
	std::vector<const char*> debugInstanceLayersVector;
	std::vector<const char*> debugInstanceExtensionsVector;
	std::vector<const char*> debugDeviceLayersVector; 		// depricated, but still recommended
	std::vector<const char*> debugDeviceExtensionsVector;

	VkDebugReportCallbackCreateInfoEXT debugReportCallbackCreateInfo{}; // important for creations of both instance and debugReportCallback
	VkDebugReportCallbackEXT	debugReportCallback = VK_NULL_HANDLE;
	PFN_vkCreateDebugReportCallbackEXT	fetch_vkCreateDebugReportCallbackEXT = VK_NULL_HANDLE;
	PFN_vkDestroyDebugReportCallbackEXT	fetch_vkDestroyDebugReportCallbackEXT = VK_NULL_HANDLE;

	void initDebugLayers();
	void initExtensions();
	void createInstance();
	void initDebugReportCallback();

	/*******************************************************************************************************************************/
	/********* The window surface needs to be created right after the instance creation, *******************************************/
	/********* because it can actually influence the physical device selection.          *******************************************/
	void createSurface(); // surface == default framebuffer to draw

	void showPhysicalDeviceInfo(const VkPhysicalDevice& gpuToCheck);
	bool isPhysicalDeviceSuitable(const VkPhysicalDevice& gpuToCheck, int32_t& graphicsQueueIndex, int32_t& presentQueueIndex);
	int  ratePhysicalDevice(const VkPhysicalDevice& gpuToCheck, int32_t& graphicsQueueIndex, int32_t& presentQueueIndex);
	void pickPhysicalDevice();
	void createLogicalDevice();
	
	void createShaderModule(const VkDevice& logicalDevice, const std::vector<char>& SPIRV_Vector, VkShaderModule& shaderModule);
	void collectSwapchainFeatures();
	void createSwapchain();
	void createSwapchainImageViews();

	void createTriangleRenderPass();
	
	void createTrianglePipeline();
	void createSwapchainFramebuffers();
	void createCommandPool();
	void createTriangleVertexBuffer();
	void createTriangleIndexBuffer();

	void createTriangleCommandBuffers();
	void createSemaphores();

	void createDescriptorSetLayout();
	void createUniformBuffer();
	void createDescriptorPool();
	void createDescriptorSet();
	void updateUniformBuffer();


	void setImageMemoryBarrier(VkImage image, VkImageAspectFlags imageAspectFlags
		, VkImageLayout oldImageLayout, VkImageLayout newImageLayout
		, VkAccessFlagBits srcAccessFlagBits, const VkCommandBuffer& imageLayoutTransitionCommandBuffer);
	void createDepthStencilAttachment();
	void createDepthStencilRenderPass();
	void createDepthStencilGraphicsPipeline();
	//void createDepthStencilFramebuffers();


//	void keyboardRegister();


	bool checkInstanceLayersSupport(std::vector<const char*> layersVector);

// Enumerate All
	void showAllSupportedInstanceExtensions();
	void showAllSupportedInstanceLayers();
	void showAllSupportedExtensionsEachUnderInstanceLayer();
	void showPhysicalDeviceSupportedLayersAndExtensions(const VkPhysicalDevice& gpuToCheck);
};


#endif //__SenAbstractGLFW__