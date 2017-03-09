# SenVulkan
Begin with Vulkan 1.042, step by step

### Use VS2015 to directly push to github
## Compile three folders (cmake, vs2015) before using VulkanSDK:
* glslang
* spirv-tools
* Samples

## Questions
* the resource delete management VDeleter templete ? 

## Tips
* 0. nullptr is special NULL in C++ for solving Overriding problem <br>
* 1. OutputDebugString <br>
#include <string>
std::string strExtension = std::to_string(i) + ". " + std::string(glfwInstanceExtensions[i]) + "\n";
OutputDebugString(strExtension.c_str());

* 2. If you want to enable a device layer, that layer should also be enabled as an active instance layer (otherwise crash)		<br>