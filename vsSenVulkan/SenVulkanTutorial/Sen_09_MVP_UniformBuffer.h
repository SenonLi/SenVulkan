#pragma once

#ifndef __Sen_09_MVP_UniformBuffer__
#define __Sen_09_MVP_UniformBuffer__

#include "../Support/SLVK_AbstractGLFW.h"

class Sen_09_MVP_UniformBuffer : public SLVK_AbstractGLFW
{
public:
	Sen_09_MVP_UniformBuffer();
	virtual ~Sen_09_MVP_UniformBuffer();

protected:
	void initVulkanApplication() { ; }
	void reCreateRenderTarget() { ; } // for resize window
	void finalizeWidget() { ; }
	void updateUniformBuffer() { ; }
	void cleanUpDepthStencil() { ; }

};

#endif // !__Sen_09_MVP_UniformBuffer__
