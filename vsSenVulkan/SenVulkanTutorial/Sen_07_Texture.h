#pragma once

#ifndef __Sen_07_Texture__
#define __Sen_07_Texture__

#include "../Support/SenAbstractGLFW.h"

class Sen_07_Texture :	public SenAbstractGLFW
{
public:
	Sen_07_Texture();
	virtual ~Sen_07_Texture();

protected:
	//void initGlfwVulkan();
	//void paintVulkan();
	//void reCreateTriangleSwapchain(); // for resize window
	void finalize();

private:
	//void createTextureImageView();
	//void createTextureSampler();

	int backgroundTextureWidth, backgroundTextureHeight, backgroundTextureChannels;

	VkImage backgroundTextureImage;
	VkDeviceMemory backgroundTextureImageDeviceMemory;
	VkImageView backgroundTextureImageView;
	VkSampler backgroundTextureSampler;


};


#endif // !__Sen_07_Texture__

