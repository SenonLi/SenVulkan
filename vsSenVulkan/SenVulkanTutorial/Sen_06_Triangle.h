#pragma once

#ifndef __Sen_06_Triangle__
#define __Sen_06_Triangle__

#include "../Support/SenAbstractGLFW.h"

class Sen_06_Triangle : public SenAbstractGLFW
{
public:
	Sen_06_Triangle();
	virtual ~Sen_06_Triangle();
	
protected:
	void initVulkanApplication();
	void reCreateRenderTarget();// for resize window
	void paintVulkan();
	void finalizeWidget() { ; }	// no particular object is used in this triangle widget


private:

};




#endif // !__Sen_06_Triangle__

