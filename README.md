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
* 26. All of the operations in drawFrame are asynchronous, which means that when we exit the loop in mainLoop, drawing and presentation operations may still be going on,
		and cleaning up resources while that is happening is a bad idea;
* 27. The stagingBuffer copy command requires a queue family that supports transfer operations, which is indicated using VK_QUEUE_TRANSFER_BIT;
		The good news is that any queue family with VK_QUEUE_GRAPHICS_BIT or VK_QUEUE_COMPUTE_BIT capabilities already implicitly support VK_QUEUE_TRANSFER_BIT operations;
* 28.	A pipeline layout allow a pipeline to access the descriptor sets;
		A descriptor set connects a given resource to the shader;
		A descriptor set layout is a collection of zero or more descriptor bindings;
		A descriptor consists of descriptor set objects, and it helps connect the resources with the shaders;
	Sum: A pipeline layout can contain zero or more descriptor sets in sequence, with each set up through a specific descriptor layout;
		this descriptor layout defines the interfaces between shader stages and shader resources, with each resource bound through descriptorLayoutBinding.
* 29. When a DescriptorPool is destroyed, all descriptor sets allocated from the pool are implicitly freed and become invalid;
		Descriptor sets allocated from a given pool do not need to be freed before destroying that descriptor pool.
* 30. Spec Image Layout Transition:
		Transitions can happen with 
						a. an image memory barrier, included as part of a vkCmdPipelineBarrier;
						b. or a vkCmdWaitEvents command buffer command;
						c. or as part of a subpass dependency within a render pass (VkSubpassDependency).
		When performing a layout transition on the image subresource,
			The old layout value must either equal the current layout of the image subresource (at the time the transition executes);
				or else be VK_IMAGE_LAYOUT_UNDEFINED (implying that the contents of the image subresource need not be preserved);
			The new layout used in a transition must not be VK_IMAGE_LAYOUT_UNDEFINED or VK_IMAGE_LAYOUT_PREINITIALIZED.
		When a layout transition is specified in a memory dependency,
			it happens-after the availability operations in the memory dependency, and happens-before the visibility operations;
			Image layout transitions may perform read and write accesses on all memory bound to the image subresource range,
				so applications must ensure that all memory writes have been made available before a layout transition is executed.
		During a render pass instance, an attachment can use a different layout in each subpass, if desired.

		a. Image memory barriers can be used to define image layout transitions or a queue family ownership transfer for the specified image subresource range;
		c. VkSubpassDependency: an application provides the layout that each attachment must be in at the start and end of a renderpass,
						and the layout it must be in during each subpass it is used in.
				Automatic layout transitions from the layout used in a subpass happen
											-after the availability operations for all dependencies with that subpass as the srcSubpass.
				Automatic layout transitions into the layout used in a subpass happen
											-before the visibility operations for all dependencies with that subpass as the dstSubpass.
* 31. Spec Copy/Transfer commands:
			Copy commands must be recorded outside of a render pass instance; 
			Copy regions must be non-empty;
			Source image subresources must be in either the VK_IMAGE_LAYOUT_GENERAL or VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL layout;
			Destination image subresources must be in the VK_IMAGE_LAYOUT_GENERAL or VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL layout;
			Source images must have been created with the VK_IMAGE_USAGE_TRANSFER_SRC_BIT usage bit enabled 
					and destination images must have been created with the VK_IMAGE_USAGE_TRANSFER_DST_BIT usage bit enabled;
			Source buffers must have been created with the VK_BUFFER_USAGE_TRANSFER_SRC_BIT usage bit enabled
					and destination buffers must have been created with the VK_BUFFER_USAGE_TRANSFER_DST_BIT usage bit enabled;
			All copy commands are treated as transfer operations for the purposes of synchronization barriers;
			The formats of srcImage and dstImage must be compatible. Formats are considered compatible if their element size is the same between both formats;
					For example, VK_FORMAT_R8G8B8A8_UNORM is compatible with VK_FORMAT_R32_UINT because both texels are 4 bytes in size;
					Depth/stencil formats must match exactly.
* 32. nVidia multiple threads presentation, Queues:
	• Queue “families” can accept different types of work, e.g,
			One form of work in a queue (e.g. DMA/memory transfer-only queue)
* 33. LearnVulkan OnlineTutorial --	VK_IMAGE_LAYOUT_GENERAL doesn't necessarily offer the best performance for any operation:
		VK_IMAGE_LAYOUT_GENERAL is required for some special cases,
				like using an image as both input and output;
				or for reading an image after it has left the preinitialized layout.
* 34. NVIDIA Blog: PushConstants can be practical for very a small amount of information passed to drawcalls.
		Too much information would slow down CPU side due to additional allocations for the values being alone.

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
* 5. Vulakn system coordinate is right handed, instead of (OpenGL) left handed, due to the opposite Y-axis direction;
	if a triangle with correct coordinates in OpenGL disappeared running in Vulkan, it might be caused by face culling.
* 6. Check the type of indicesArray if you cannot say what you draw, make sure uint16_t (instead of float) if you vkCmdBindIndexBuffer with VK_INDEX_TYPE_UINT16;
* 

### Problems to solve
00. Why call the image and sampler combined? 
	Why using combined?
0. Change transitionResourceImageLayout(...) and copyImage(...) to use only one singleTimeCommandBuffer;
		It is recommended to combine these operations in a single command buffer and execute them asynchronously for higher throughput,
			especially the transitions and copy in the createTextureImage function.
		Try to experiment with this by creating a setupCommandBuffer that the helper functions record commands into,
			and add a flushSetupCommands to execute the commands that have been recorded so far.
		It's best to do this after the texture mapping works to check if the texture resources are still set up correctly.
0.5 add srcImageLayout dstImageLayout as arguments for function transferResourceImage(...) with default input;
0.6 Figure out / Change imageMemoryBarrier.srcAccessMask / .dstAccessMask (VK_ACCESS_HOST_WRITE_BIT), to include as VK_IMAGE_LAYOUT_GENERAL oldImageLayout;
0.7 Add mipMap Level handler for resourceImage Creation;
0.8 Figure out an efficient descriptorPool, descriptorSetLayout, descriptorSets handler 
		to handle different bindings for different application with same mvpUniformBufferObject;
1. Dynamic pipeline reuse for resizing;
2. Memory allocator to solve the maximum count to allocate deviceMemory;
3. put model view projection into different descriptorLayoutBinding;
4. Write online_compileShader class;
5. Driver developers recommend that you also store multiple buffers, like the vertex and index buffer,
		into a single VkBuffer and use offsets in commands like vkCmdBindVertexBuffers.
	The advantage is that your data is more cache friendly in that case, because it's closer together.
	It is even possible to reuse the same chunk of memory for multiple resources if they are not used during the same render operations,
		provided that their data is refreshed, of course.
6. You may wish to create a separate command pool for these kinds of short - lived buffers, 
		because the implementation may be able to apply memory allocation optimizations.
	You should use the VK_COMMAND_POOL_CREATE_TRANSIENT_BIT flag during command pool generation in that case.
7. Use imageArray to draw SenCube.
8. 