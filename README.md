# SenVulkan
SenBegin with Vulkan 1.042, step by step

* Fixed Release build issue. (cannot ignore MVCT lib)
* Add Validation Layer (as Instance Layer) check function


### Use VS2015 to directly push to github
## Compile three folders (cmake, vs2015) before using VulkanSDK:
* glslang
* spirv-tools
* Samples

## Questions
* the resource delete management VDeleter templete ? 
* namespace and extern in C++?
* Staging buffer? 
* Asynchronous transfer?
* 
std::unique_ptr<VulkanApplication> VulkanApplication::instance;
std::once_flag VulkanApplication::onlyOnce;

extern std::vector<const char *> instanceExtensionNames;
extern std::vector<const char *> layerNames;
extern std::vector<const char *> deviceExtensionNames;

// Returns the Single ton object of VulkanApplication
VulkanApplication* VulkanApplication::GetInstance(){
    std::call_once(onlyOnce, [](){instance.reset(new VulkanApplication()); });
    return instance.get();
}

* Difference between std::multimap and std::map?
* also  std::set, std::unordered_set and std::unordered_map, std::multimap
* constexpr specifier (since C++11)?
* What is the difference between buffer objects and uniform buffer objects?
* What is suballocation, and why can it make a good performance?

## Terminologies
Vulkan is a layered architecture, made up of (The Vulkan Application, The Vulkan Loader, Vulkan Layers and Installable Client Drivers)
* ICDs: Installable Client Drivers (talked in conjunction with Vulkan Loader and Vulkan Layers)
* WSI: Window System Integration, which provides a platform-independent way to implement windowing or surface management.
* Tiling: Arrangement of data elements in memory, telling how an image is saved in GPU memory
* Optical tiling: an image is saved in GPU memory with an arrangement based on group of pixels that can offer fastest GPU access speed.
	it's arrangement pattern is uncertain that we won't know.
*  IHVs: Independent Hardware Vendors

## Tips
### Vulkan
* 1. Extensions are to support vulkan applications, and layers are for runtime debugging that may even cover the extensions debugging at runtime;
in other words, each of the layers means to support some extensions, which means the supportable extentions could be found enumerated based on the layerName.
* 2. Physical device will be implicitly destroyed when the VkInstance is destroyed, so we don't need to, and there is no command to destroy a phisicalDevice.
* 3. Queues are automatically created when a logical device object is created; 
* 4. Vulkan lets you VkDeviceQueueCreateInfo.pQueuePriorities to queues to influence the scheduling of command buffer execution using floating point numbers between 0.0 and 1.0. This is required even if there is only a single queue.
* 5. For an image object, a subresource means a combined view from a single texture with a desired mipmap lever;
						  its format basically means how many channels to represent color (RGBA)
						  its tiling refers to the GPU alignment of textels for and Image Object, where the optimal tiling gives optimal memory access;
						  its layout state is per-(image subrecource), and seperate subresources of the same image can be in different layouts at the same time except that depth and stencil aspects of a given image subrecource must always be in the same layout.
* 6. Presentation capability is a per-queueFamily feature ( One Physical Device has several QueueFamilies );
	presentation capability is a feature of physicalDevice;
	surface survives even longer, and depends only on instance and platform;
* 7. VkSwapchainKHR is a child of the device and is affected by the lost state (Hardware Problem); it must be destroyed before destroying the VkDevice.
	 Based on the API function vkCreateSharedSwapchainsKHR(), one logical device could own multiple SwapChains;
	 However, VkSurfaceKHR is not a child of any VkDevice and is not otherwise affected by the lost device.
	 After successfully recreating a VkDevice, the same VkSurfaceKHR can be used to create a new VkSwapchainKHR, provided the previous one was destroyed；
* 8. Multiple Queues with same QueueFamilyIndex can only be created using one deviceQueueCreateInfo;
	 Different QueueFamilyIndex's Queues need to be created using a vector of DeviceQueueCreateInfos;
* 9. Display Planes (Overlay Planes) are with respect to the displays that are attached to physicalDevices;
* 10. It is the physicalDevice, instead of physical display, that performs composition operations to merge information from the planes into a single image;
* 11. Before an image can be presented, it must be in the correct layout;
* 12. ☆☆☆☆☆  An application can acquire use of a presentable image with vkAcquireNextImageKHR. 
		After acquiring a presentable image and before modifying it, the application must use a synchronization primitive to ensure that the presentation engine has finished reading from the image. 
		The application can then transition the image layout, queue rendering commands to it, etc.
		Finally, the application presents the image with vkQueuePresentKHR, which releases the acquisition of the image;
		It allows the application to generate command buffers referencing all of the images in the swapchain at initializationtime, rather than in its main loop;
* 13. The native window referred to by surface must not already be associated with a swapchain other than oldSwapchain, or with a non-Vulkan graphics API surface.
* 14. The graphics pipeline in Vulkan is almost completely immutable, so you must recreate the pipeline from scratch if you want to change shaders, bind different framebuffers or change the blend function. 
* 15. The VkShaderModule object is just a dumb wrapper around the bytecode buffer, they are only required during the pipeline creation process;
* 16. A limited amount of the state that we've specified in the previous structs can actually be changed without recreating the pipeline:
		viewport, line width, blend constants ...
		This will cause the configuration of these values to be ignored and you will be required to specify the data at drawing time.;
* 17. The uniform values need to be specified during pipeline creation by creating a VkPipelineLayout object:
		transformation matrix, texture samplers ...
* 18. You can only use a framebuffer with the render passes that it is compatible with, which roughly means that they use the same number and type of attachments;
* 19. A render pass  groups dependent operations and contains a number of subpasses, which describe access to attachments (subpasses are subsequent rendering operations);
* 20. CommandPools are externally synchronized, meaning that a commandPool must not be used concurrently in multiple threads;
* 21. If the command buffer was already recorded once, then a call to vkBeginCommandBuffer will implicitly reset it;
* 22. Fences are mainly designed to synchronize your application itself with rendering operation,
	  semaphores are used to synchronize operations within or across command queues;
* 23. Framebuffers and graphics pipelines are created based on a specific render pass object. They must only be used with that render pass object, or one compatible with it.
* 24. A render pass and a framebuffer define the complete render target state for one or more subpasses as well as the algorithmic dependencies between the subpasses.
* 25. Image subresources used as attachments must not be used via any non-attachment usage for the duration of a render pass instance;
		This restriction means that the render pass has full knowledge of all uses of all of the attachments,
		so that the implementation is able to make correct decisions about 
		when and how to perform layout transitions,	when to overlap execution of subpasses, etc;
* 

### V++ / Debug
* 0. nullptr is special NULL in C++ for solving Overriding problem <br>
* 1. OutputDebugString <br>
#include <string>
std::string strExtension = std::to_string(i) + ". " + std::string(glfwInstanceExtensions[i]) + "\n";
OutputDebugString(strExtension.c_str());

* 2. Use reinterpret_cast to do cast between two unrelated types
* 3. Even though nullptr can handle the overloaded function problem of NULL, it cannot handle the 64bit to 32bit Vulkan handle transfer problem.
VK_NULL_HANDLE should be used to initial a Vulkan object handle instead of nullptr.
* 4. std::array  :
	What are the advantages of using std::array over usual ones?
	It has friendly value semantics, so that it can be passed to or returned from functions by value. Its interface makes it more convenient to find the size, and use with STL-style iterator-based algorithms.

	Is it more performant ?
	It should be exactly the same. By definition, it's a simple aggregate containing an array as its only member.

	Just easier to handle for copy/access ?
	Yes.
* 