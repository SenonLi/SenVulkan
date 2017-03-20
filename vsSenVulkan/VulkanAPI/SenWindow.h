#pragma once

#include "Shared.h"

#include <string>

class SenWindow
{
public:
	SenWindow( uint32_t size_x, uint32_t size_y, std::string name );
	~SenWindow();

	void closeSenWindow();
	bool updateSenWindow();

private:
	void								_InitOSWindow();
	void								_DeInitOSWindow();
	void								_UpdateOSWindow();
	//void								_InitOSSurface();

	uint32_t							_surfaceSize_X					= 512;
	uint32_t							_surfaceSize_Y					= 512;
	std::string							_window_name;

	bool								_windowShouldRun				= true;

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
