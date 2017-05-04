#include "Sen_07_Texture.h"

Sen_07_Texture::Sen_07_Texture()
{
}


Sen_07_Texture::~Sen_07_Texture()
{
	finalize();
}

void Sen_07_Texture::finalize()
{	
	SenAbstractGLFW::finalize();

	/************************************************************************************************************/
	/******************     Destroy background Memory, ImageView, Image     ***********************************/
	/************************************************************************************************************/
	if (VK_NULL_HANDLE != backgroundTextureImage) {
		vkDestroyImage(device, backgroundTextureImage, nullptr);
		vkDestroyImageView(device, backgroundTextureImageView, nullptr);
		vkDestroySampler(device, backgroundTextureSampler, nullptr);
		vkFreeMemory(device, backgroundTextureImageDeviceMemory, nullptr); 	// always try to destroy before free

		backgroundTextureImage				= VK_NULL_HANDLE;
		backgroundTextureImageDeviceMemory	= VK_NULL_HANDLE;
		backgroundTextureImageView			= VK_NULL_HANDLE;
		backgroundTextureSampler			= VK_NULL_HANDLE;
	}
}
