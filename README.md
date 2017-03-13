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

## Terminologies
Vulkan is a layered architecture, made up of (The Vulkan Application, The Vulkan Loader, Vulkan Layers and Installable Client Drivers)
* ICDs: Installable Client Drivers (talked in conjunction with Vulkan Loader and Vulkan Layers)

## Tips
* 0. nullptr is special NULL in C++ for solving Overriding problem <br>
* 1. OutputDebugString <br>
#include <string>
std::string strExtension = std::to_string(i) + ". " + std::string(glfwInstanceExtensions[i]) + "\n";
OutputDebugString(strExtension.c_str());

* 2. If you want to enable a device layer, that layer should also be enabled as an active instance layer (otherwise crash)		<br>
* 3. Use reinterpret_cast to do cast between two unrelated types
