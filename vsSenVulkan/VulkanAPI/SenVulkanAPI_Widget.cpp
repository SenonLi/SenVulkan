#include "SenVulkanAPI_Widget.h"



SenVulkanAPI_Widget::SenVulkanAPI_Widget()
{
	strWindowName = "Sen VulkanAPI Widget";
}


SenVulkanAPI_Widget::~SenVulkanAPI_Widget()
{
	finalize();
	OutputDebugString("\n ~SenVulkanAPI_Widget()\n");
}

void SenVulkanAPI_Widget::initGlfwVulkan()
{

	initCommandBuffers();
}

void SenVulkanAPI_Widget::initCommandBuffers()
{
	device = renderer._device;
	queue = renderer._queue;

	VkFenceCreateInfo fenceCreateInfo{};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	vkCreateFence(device, &fenceCreateInfo, nullptr, &fence);

	VkSemaphoreCreateInfo semaphoreCreateInfo{};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphore);

	VkCommandPoolCreateInfo commandPoolCreateInfo{};
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.queueFamilyIndex = renderer._graphicsFamilyIndex;
	commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	vkCreateCommandPool(device, &commandPoolCreateInfo, nullptr, &commandPool);

	VkCommandBuffer commandBuffer[2];
	VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandPool = commandPool;
	commandBufferAllocateInfo.commandBufferCount = 2;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, commandBuffer);

	{
		VkCommandBufferBeginInfo commandBufferBeginInfo{};
		commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		vkBeginCommandBuffer(commandBuffer[0], &commandBufferBeginInfo);

		vkCmdPipelineBarrier(commandBuffer[0],
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			0,
			0, nullptr,
			0, nullptr,
			0, nullptr);

		VkViewport viewport{};
		viewport.maxDepth = 1.0f;
		viewport.minDepth = 0.0f;
		viewport.width = 512;
		viewport.height = 512;
		viewport.x = 0;
		viewport.y = 0;
		vkCmdSetViewport(commandBuffer[0], 0, 1, &viewport);

		vkEndCommandBuffer(commandBuffer[0]);
	}
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		vkBeginCommandBuffer(commandBuffer[1], &beginInfo);

		VkViewport viewport{};
		viewport.maxDepth = 1.0f;
		viewport.minDepth = 0.0f;
		viewport.width = 512;
		viewport.height = 512;
		viewport.x = 0;
		viewport.y = 0;
		vkCmdSetViewport(commandBuffer[1], 0, 1, &viewport);

		vkEndCommandBuffer(commandBuffer[1]);
	}
	/***********************************************************************/
	{
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer[0];
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &semaphore;
		vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
	}
	{
		VkPipelineStageFlags flags[]{ VK_PIPELINE_STAGE_ALL_COMMANDS_BIT };
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer[1];
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &semaphore;
		submitInfo.pWaitDstStageMask = flags;
		vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
	}
	//auto ret = vkWaitForFences( device, 1, &fence, VK_TRUE, UINT64_MAX );
	vkQueueWaitIdle(queue);// For Synchronization 
}


void SenVulkanAPI_Widget::showWidget()
{
	initGlfwVulkan();

	renderer.openSenWindow(800, 600, strWindowName);

	while (renderer.run()) {

	}

	//finalize();// all the clean up works
}


void SenVulkanAPI_Widget::finalize()
{
	renderer.closeSenWindow();

	if (VK_NULL_HANDLE != commandPool) {
		vkDestroyCommandPool(device, commandPool, nullptr);
		commandPool = VK_NULL_HANDLE;
	}
	if (VK_NULL_HANDLE != fence) {
		vkDestroyFence(device, fence, nullptr);
		fence = VK_NULL_HANDLE;
	}
	if (VK_NULL_HANDLE != semaphore) {
		vkDestroySemaphore(device, semaphore, nullptr);
		semaphore = VK_NULL_HANDLE;
	}

	renderer.finalize();
	// Destroy logical device
	if (VK_NULL_HANDLE != device) {
		// device command should be destroyed by Render

		//vkDestroyDevice(device, VK_NULL_HANDLE); 

		//// Device queues are implicitly cleaned up when the device is destroyed
		//if (VK_NULL_HANDLE != graphicsQueue) { graphicsQueue = VK_NULL_HANDLE; }
		//if (VK_NULL_HANDLE != presentQueue) { presentQueue = VK_NULL_HANDLE; }

		device = VK_NULL_HANDLE;
	}


}
