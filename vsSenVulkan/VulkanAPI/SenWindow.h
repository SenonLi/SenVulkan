#pragma once

#include "Shared.h"

#include <string>
#include <vector>

class SenRenderer;

class SenWindow
{
public:
	SenWindow( SenRenderer * renderer, uint32_t size_x, uint32_t size_y, std::string name );
	~SenWindow();

	//void finalize();
	void closeSenWindow();
	bool updateSenWindow();

private:
	void								_InitOSWindow();
	void								_DeInitOSWindow();
	void								_UpdateOSWindow();
	void								_InitOSSurface();

	void								_InitSurface();
	void								_DeInitSurface();

	void								_InitSwapchain();
	void								_DeInitSwapchain();
	void								_InitSwapchainImages();
	void								_DeInitSwapchainImages();

	SenRenderer							*_renderer = nullptr;

	std::string							_window_name;

	VkSurfaceKHR						_surface				= VK_NULL_HANDLE;
	VkSurfaceFormatKHR					_surface_format			= {};
	VkSurfaceCapabilitiesKHR			_surface_capabilities	= {};
	uint32_t							_surfaceSize_X			= 512;
	uint32_t							_surfaceSize_Y			= 512;
	bool								_windowShouldRun		= true;

	VkSwapchainKHR						_swapchain = VK_NULL_HANDLE;
	uint32_t							_swapchainImagesCount = 2;
	std::vector<VkImage>				_swapChainImagesVector;
	std::vector<VkImageView>			_swapChainImageViewsVector;


#if defined( _WIN32 )  // on Windows OS
	HINSTANCE							_win32_instance					= NULL;
	HWND								_win32_window					= NULL;
	std::string							_win32_class_name;
	static uint64_t						_win32_class_id_counter;

#elif defined( __linux ) // on Linux ( Via XCB library )

	xcb_connection_t				*	_xcb_connection					= nullptr;
	xcb_screen_t					*	_xcb_screen						= nullptr;
	xcb_window_t						_xcb_window						= 0;
	xcb_intern_atom_reply_t			*	_xcb_atom_window_reply			= nullptr;
#endif
};
