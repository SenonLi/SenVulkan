#include "SenWindow.h"
#include "SenRenderer.h"

SenWindow::SenWindow(SenRenderer * renderer, uint32_t size_x, uint32_t size_y, std::string name )
{
	_renderer = renderer;
	_surfaceSize_X = size_x;
	_surfaceSize_Y = size_y;
	_window_name = name;

	_InitOSWindow();
	_InitSurface();

	_InitSwapchain();
	_InitSwapchainImages();
	_InitDepthStencilImage();
}

SenWindow::~SenWindow()
{
	_DeInitDepthStencilImage();
	_DeInitSwapchainImages();
	_DeInitSwapchain();
	_DeInitSurface();
	_DeInitOSWindow();
}

void SenWindow::_InitSwapchainImages()
{
	_swapChainImagesVector.resize(_swapchainImagesCount);
	_swapChainImageViewsVector.resize(_swapchainImagesCount);
	// Get swapChainImages this time, instead of asking for real count.
	ErrorCheck(vkGetSwapchainImagesKHR(_renderer->getDevice(), _swapchain, &_swapchainImagesCount, _swapChainImagesVector.data()));

	for (uint32_t i = 0; i < _swapchainImagesCount; ++i) {
		VkImageViewCreateInfo swapchainImageViewCreateInfo{};
		swapchainImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		swapchainImageViewCreateInfo.image = _swapChainImagesVector[i];
		swapchainImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; // handling 2D image
		swapchainImageViewCreateInfo.format = _surface_format.format;
		swapchainImageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		swapchainImageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		swapchainImageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		swapchainImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		swapchainImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // color/depth/stencil/metadata
		swapchainImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		swapchainImageViewCreateInfo.subresourceRange.levelCount = 1; // amount of mipmaps
		swapchainImageViewCreateInfo.subresourceRange.baseArrayLayer = 0; // ?
		swapchainImageViewCreateInfo.subresourceRange.layerCount = 1; // if larger than 1, .viewType needs to be array

		ErrorCheck(vkCreateImageView(_renderer->getDevice(), &swapchainImageViewCreateInfo, nullptr, &_swapChainImageViewsVector[i]));
	}
}

void SenWindow::_DeInitSwapchainImages()
{
	// swapChainImages are handled by the destroy of swapchain
	// but imageViews need to be dstroyed.
	for (auto view : _swapChainImageViewsVector) {
		vkDestroyImageView(_renderer->getDevice(), view, nullptr);
	}
}

void SenWindow::_InitDepthStencilImage()
{
	/******************************************************************************************************************************************************/
	/******************************  Create depthStencil Image ********************************************************************************************/
	{ // Check Image Format
		std::vector<VkFormat> tryFormatsVector{
			VK_FORMAT_D32_SFLOAT_S8_UINT,
			VK_FORMAT_D24_UNORM_S8_UINT,
			VK_FORMAT_D16_UNORM_S8_UINT,
			VK_FORMAT_D32_SFLOAT,
			VK_FORMAT_D16_UNORM
		};
		for (auto f : tryFormatsVector) {
			VkFormatProperties formatProperties{};
			vkGetPhysicalDeviceFormatProperties(_renderer->getPhysicalDevice(), f, &formatProperties);
			if (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
				_depthStencilFormat = f;
				break;
			}
		}
		if (_depthStencilFormat == VK_FORMAT_UNDEFINED) {
			assert(0 && "Depth stencil format not selected.");
			std::exit(-1);
		}
		if ((_depthStencilFormat == VK_FORMAT_D32_SFLOAT_S8_UINT) ||
			(_depthStencilFormat == VK_FORMAT_D24_UNORM_S8_UINT) ||
			(_depthStencilFormat == VK_FORMAT_D16_UNORM_S8_UINT) ||
			(_depthStencilFormat == VK_FORMAT_S8_UINT)) {
			_stencilAvailable = true;
		}
	}

	VkImageCreateInfo depthStencilImageCreateInfo{};
	depthStencilImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	depthStencilImageCreateInfo.flags = 0;
	depthStencilImageCreateInfo.imageType = VK_IMAGE_TYPE_2D; // 2D image
	depthStencilImageCreateInfo.format = _depthStencilFormat;
	depthStencilImageCreateInfo.extent.width = _surfaceSize_X;
	depthStencilImageCreateInfo.extent.height = _surfaceSize_Y;
	depthStencilImageCreateInfo.extent.depth = 1; // 2D image, has to be 1
	depthStencilImageCreateInfo.mipLevels = 1;
	depthStencilImageCreateInfo.arrayLayers = 1;
	depthStencilImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT; // multi-sampling
	depthStencilImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	depthStencilImageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;// for depth stencil image
	depthStencilImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // no share with anther QueueFamily
	depthStencilImageCreateInfo.queueFamilyIndexCount = VK_QUEUE_FAMILY_IGNORED;
	depthStencilImageCreateInfo.pQueueFamilyIndices = nullptr;
	depthStencilImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // will overwrite this with a command later

	vkCreateImage(_renderer->getDevice(), &depthStencilImageCreateInfo, nullptr, &_depthStencilImage);

	/******************************************************************************************************************************************************/
	/***************************  Allocate & Bind memory for depthStencil Image using the created handle *********************************************************/
	VkMemoryRequirements imageMemoryRequirements{};
	vkGetImageMemoryRequirements(_renderer->getDevice(), _depthStencilImage, &imageMemoryRequirements);

	uint32_t memoryTypeIndex = FindMemoryTypeIndex(
		&_renderer->getPhysicalDeviceMemoryProperties(),
		&imageMemoryRequirements,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT  // Set the resource to reside on GPU itself
	);

	VkMemoryAllocateInfo memoryAllocateInfo{};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = imageMemoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;

	vkAllocateMemory(_renderer->getDevice(), &memoryAllocateInfo, nullptr, &_depthStencilImageMemory);
	vkBindImageMemory(_renderer->getDevice(), _depthStencilImage, _depthStencilImageMemory, 0);

	/******************************************************************************************************************************************************/
	/******************************  Create depthStencil Image View ***************************************************************************************/
	VkImageViewCreateInfo depthStencilImageViewCreateInfo{};
	depthStencilImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	depthStencilImageViewCreateInfo.image = _depthStencilImage;
	depthStencilImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	depthStencilImageViewCreateInfo.format = _depthStencilFormat;
	depthStencilImageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	depthStencilImageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	depthStencilImageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	depthStencilImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	depthStencilImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | (_stencilAvailable ? VK_IMAGE_ASPECT_STENCIL_BIT : 0);
	depthStencilImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	depthStencilImageViewCreateInfo.subresourceRange.levelCount = 1;
	depthStencilImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	depthStencilImageViewCreateInfo.subresourceRange.layerCount = 1;

	vkCreateImageView(_renderer->getDevice(), &depthStencilImageViewCreateInfo, nullptr, &_depthStencilImageView);
}

void SenWindow::_DeInitDepthStencilImage()
{
	vkFreeMemory(_renderer->getDevice(), _depthStencilImageMemory, nullptr);
	vkDestroyImageView(_renderer->getDevice(), _depthStencilImageView, nullptr);
	vkDestroyImage(_renderer->getDevice(), _depthStencilImage, nullptr);
}

void SenWindow::closeSenWindow()
{
	_windowShouldRun		= false;
}

bool SenWindow::updateSenWindow()
{
	_UpdateOSWindow();
	return _windowShouldRun;
}

void SenWindow::_InitSurface()
{
	_InitOSSurface();

	auto gpu = _renderer->getPhysicalDevice();

	VkBool32 WSI_supported = false;
	vkGetPhysicalDeviceSurfaceSupportKHR(gpu, _renderer->getGraphicsQueueFamilyIndex(), _surface, &WSI_supported);
	if (!WSI_supported) {
		assert(0 && "WSI not supported");
		std::exit(-1);
	}

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, _surface, &_surface_capabilities);
	// Do the check & assignment below because what we surface size we got may not equal to what we set
	// Make sure the size of swapchain match the size of surface
	if (_surface_capabilities.currentExtent.width < UINT32_MAX) {
		_surfaceSize_X = _surface_capabilities.currentExtent.width;
		_surfaceSize_Y = _surface_capabilities.currentExtent.height;
	} 
	{
		uint32_t format_count = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, _surface, &format_count, nullptr);
		if (format_count == 0) {
			assert(0 && "Surface formats missing.");
			std::exit(-1);
		}
		std::vector<VkSurfaceFormatKHR> formats(format_count);
		vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, _surface, &format_count, formats.data());
		if (formats[0].format == VK_FORMAT_UNDEFINED) { // the surface layer doesnot care about the format
			_surface_format.format = VK_FORMAT_B8G8R8A8_UNORM;
			_surface_format.colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
		}
		else {
			_surface_format = formats[0];
		}
	}
}

void SenWindow::_DeInitSurface()
{
	vkDestroySurfaceKHR(_renderer->getInstance(), _surface, nullptr);
}

void SenWindow::_InitSwapchain()
{
	/***************************************************************************************/
	/********** Vulkan Image Source will be used to implement Vulkan SwapChain *************/
	/***************************************************************************************/

	// This code is old and the fixed one is below
	// if( _swapchainImagesCount > _surface_capabilities.maxImageCount ) _swapchainImagesCount = _surface_capabilities.maxImageCount;
	// if( _swapchainImagesCount < _surface_capabilities.minImageCount + 1 ) _swapchainImagesCount = _surface_capabilities.minImageCount + 1;

	// The code above will work just fine in our tutorials and likely on every possible implementation of vulkan as well
	// so this change isn't that important. Just to be absolutely sure we don't go over or below the given limits we should check this a
	// little bit different though. maxImageCount can actually be zero in which case the amount of swapchain images do not have an
	// upper limit other than available memory. It's also possible that the swapchain image amount is locked to a certain
	// value on certain systems. The code below takes into consideration both of these possibilities.
	if (_swapchainImagesCount < _surface_capabilities.minImageCount + 1) _swapchainImagesCount = _surface_capabilities.minImageCount + 1;
	if (_surface_capabilities.maxImageCount > 0) {
		if (_swapchainImagesCount > _surface_capabilities.maxImageCount) _swapchainImagesCount = _surface_capabilities.maxImageCount;
	}

	/***************************************************************************************/
	/*********************** Getting surface presentation modes ****************************/
	VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR; // VK_PRESENT_MODE_FIFO_KHR is always available.
	{
		uint32_t present_mode_count = 0;
		ErrorCheck(vkGetPhysicalDeviceSurfacePresentModesKHR(_renderer->getPhysicalDevice(), _surface, &present_mode_count, nullptr));
		std::vector<VkPresentModeKHR> present_mode_list(present_mode_count);
		ErrorCheck(vkGetPhysicalDeviceSurfacePresentModesKHR(_renderer->getPhysicalDevice(), _surface, &present_mode_count, present_mode_list.data()));
		for (auto m : present_mode_list) {
			// VK_PRESENT_MODE_MAILBOX_KHR is good for gaming
			if (m == VK_PRESENT_MODE_MAILBOX_KHR) present_mode = m;
		}
	}

	VkSwapchainCreateInfoKHR swapchain_create_info{};
	swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchain_create_info.surface = _surface;
	swapchain_create_info.minImageCount = _swapchainImagesCount;
	swapchain_create_info.imageFormat = _surface_format.format;
	swapchain_create_info.imageColorSpace = _surface_format.colorSpace;
	swapchain_create_info.imageExtent.width = _surfaceSize_X;
	swapchain_create_info.imageExtent.height = _surfaceSize_Y;
	swapchain_create_info.imageArrayLayers = 1; // interesting parameter
	swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;// share between QueueFamilies or not
	swapchain_create_info.queueFamilyIndexCount = 0;// no QueueFamily share
	swapchain_create_info.pQueueFamilyIndices = nullptr;
	swapchain_create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR; // ??
	swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // ??
	swapchain_create_info.presentMode = present_mode;
	swapchain_create_info.clipped = VK_TRUE; // ??
	swapchain_create_info.oldSwapchain = VK_NULL_HANDLE; // resize window

	ErrorCheck(vkCreateSwapchainKHR(_renderer->getDevice(), &swapchain_create_info, nullptr, &_swapchain));
	// Get actual amount/count of swapchain images
	ErrorCheck(vkGetSwapchainImagesKHR(_renderer->getDevice(), _swapchain, &_swapchainImagesCount, nullptr));
}

void SenWindow::_DeInitSwapchain()
{
	vkDestroySwapchainKHR(_renderer->getDevice(), _swapchain, nullptr);
}
