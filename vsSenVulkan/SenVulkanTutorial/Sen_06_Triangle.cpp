#include "Sen_06_Triangle.h"

Sen_06_Triangle::Sen_06_Triangle()
{
	std::cout << "Constructor: Sen_06_Triangle()\n\n";
	
	strWindowName = "Sen Vulkan Triangle Tutorial";
}

Sen_06_Triangle::~Sen_06_Triangle()
{
	finalizeWidget();

	OutputDebugString("\n\t ~Sen_06_Triangle()\n");
}
/*********************************************************************************************************************/
/*********************************************************************************************************************/
void Sen_06_Triangle::initVulkanApplication()
{
	createColorAttachOnlyRenderPass();
	createDescriptorSetLayout();

	createTrianglePipeline();
	createColorAttachOnlySwapchainFramebuffers();
	createCommandPool();

	createTriangleVertexBuffer();
	createTriangleIndexBuffer();
	createUniformBuffers();
	createDescriptorPool();
	createDescriptorSet();
	createTriangleCommandBuffers();

	std::cout << "\n Finish  Sen_06_Triangle::initVulkanApplication()\n";
}

void Sen_06_Triangle::reCreateRenderTarget()
{
	createTrianglePipeline();
	createColorAttachOnlySwapchainFramebuffers();
	createTriangleCommandBuffers();
}

void Sen_06_Triangle::paintVulkan()
{
	/*******************************************************************************************************************************/
	/*********         Acquire an image from the swap chain                              *******************************************/
	/*******************************************************************************************************************************/
	// Use of a presentable image must occur only after the image is returned by vkAcquireNextImageKHR, and before it is presented by vkQueuePresentKHR.
	// This includes transitioning the image layout and rendering commands.
	uint32_t swapchainImageIndex;
	VkResult result = vkAcquireNextImageKHR(device, swapChain,
		UINT64_MAX,							// timeout for this Image Acquire command, i.e., (std::numeric_limits<uint64_t>::max)(),
		swapchainImageAcquiredSemaphore,	// semaphore to signal
		VK_NULL_HANDLE,						// fence to signal
		&swapchainImageIndex
	);
	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		reCreateRenderTarget();
		return;
	}else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("Failed to acquire swap chain image !!!!");
	}

	/*******************************************************************************************************************************/
	/*********       Execute the command buffer with that image as attachment in the framebuffer           *************************/
	/*******************************************************************************************************************************/
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	std::vector<VkSemaphore> submitInfoWaitSemaphoresVecotr;
	submitInfoWaitSemaphoresVecotr.push_back(swapchainImageAcquiredSemaphore);
	// Commands before this submitInfoWaitDstStageMaskArray stage could be executed before semaphore signaled
	VkPipelineStageFlags submitInfoWaitDstStageMaskArray[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = (uint32_t)submitInfoWaitSemaphoresVecotr.size();
	submitInfo.pWaitSemaphores = submitInfoWaitSemaphoresVecotr.data();
	submitInfo.pWaitDstStageMask = submitInfoWaitDstStageMaskArray;

	submitInfo.commandBufferCount = 1;	// wait for submitInfoCommandBuffersVecotr to be created
	submitInfo.pCommandBuffers = &swapchainCommandBufferVector[swapchainImageIndex];

	std::vector<VkSemaphore> submitInfoSignalSemaphoresVector;
	submitInfoSignalSemaphoresVector.push_back(paintReadyToPresentSemaphore);
	submitInfo.signalSemaphoreCount = (uint32_t)submitInfoSignalSemaphoresVector.size();
	submitInfo.pSignalSemaphores = submitInfoSignalSemaphoresVector.data();

	SenAbstractGLFW::errorCheck(
		vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE),
		std::string("Failed to submit draw command buffer !!!")
	);

	/*******************************************************************************************************************************/
	/*********             Return the image to the swap chain for presentation                **************************************/
	/*******************************************************************************************************************************/
	std::vector<VkSemaphore> presentInfoWaitSemaphoresVector;
	presentInfoWaitSemaphoresVector.push_back(paintReadyToPresentSemaphore);
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = (uint32_t)presentInfoWaitSemaphoresVector.size();
	presentInfo.pWaitSemaphores = presentInfoWaitSemaphoresVector.data();

	VkSwapchainKHR swapChainsArray[] = { swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChainsArray;
	presentInfo.pImageIndices = &swapchainImageIndex;

	result = vkQueuePresentKHR(presentQueue, &presentInfo);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
		reCreateRenderTarget();
	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to present swap chain image !!!");
	}
}
