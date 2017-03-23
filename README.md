# SenVulkan
Begin with Vulkan 1.042, step by step

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
*

## Tips
### Vulkan
* 1. Extensions are to support vulkan applications, and layers are for runtime debugging that may even cover the extensions debugging at runtime;
in other words, each of the layers means to support some extensions, which means the supportable extentions could be found enumerated based on the layerName.
* 2. Physical device will be implicitly destroyed when the VkInstance is destroyed, so we don't need to, and there is no command to destroy a phisicalDevice.
* 3. Queues are automatically created when a logical device object is created; 
* 4. Vulkan lets you VkDeviceQueueCreateInfo.pQueuePriorities to queues to influence the scheduling of command buffer execution using floating point numbers between 0.0 and 1.0. This is required even if there is only a single queue.

### V++ / Debug
* 0. nullptr is special NULL in C++ for solving Overriding problem <br>
* 1. OutputDebugString <br>
#include <string>
std::string strExtension = std::to_string(i) + ". " + std::string(glfwInstanceExtensions[i]) + "\n";
OutputDebugString(strExtension.c_str());

* 2. Use reinterpret_cast to do cast between two unrelated types
* 3. Even though nullptr can handle the overloaded function problem of NULL, it cannot handle the 64bit to 32bit Vulkan handle transfer problem.
VK_NULL_HANDLE should be used to initial a Vulkan object handle instead of nullptr.
* 4. 