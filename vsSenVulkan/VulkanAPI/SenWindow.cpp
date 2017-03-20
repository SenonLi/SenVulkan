#include "SenWindow.h"


SenWindow::SenWindow( uint32_t size_x, uint32_t size_y, std::string name )
{
	_surfaceSize_X		= size_x;
	_surfaceSize_Y		= size_y;
	_window_name		= name;

	_InitOSWindow();
}

SenWindow::~SenWindow()
{
	_DeInitOSWindow();
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
