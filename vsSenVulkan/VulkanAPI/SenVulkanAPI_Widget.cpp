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
	ErrorCheck(vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, commandBuffer));

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
	//initGlfwVulkan();

	auto w = renderer.openSenWindow(800, 600, strWindowName);

	VkCommandPool command_pool = VK_NULL_HANDLE;
	VkCommandPoolCreateInfo pool_create_info{};
	pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_create_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	pool_create_info.queueFamilyIndex = renderer.getGraphicsQueueFamilyIndex();
	vkCreateCommandPool(renderer.getDevice(), &pool_create_info, nullptr, &command_pool);

	VkCommandBuffer command_buffer = VK_NULL_HANDLE;
	VkCommandBufferAllocateInfo	command_buffer_allocate_info{};
	command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	command_buffer_allocate_info.commandPool = command_pool;
	command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	command_buffer_allocate_info.commandBufferCount = 1;
	vkAllocateCommandBuffers(renderer.getDevice(), &command_buffer_allocate_info, &command_buffer);

	VkSemaphore render_complete_semaphore = VK_NULL_HANDLE;
	VkSemaphoreCreateInfo semaphore_create_info{};
	semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	vkCreateSemaphore(renderer.getDevice(), &semaphore_create_info, nullptr, &render_complete_semaphore);

	float color_rotator = 0.0f;
	auto timer = std::chrono::steady_clock();
	auto last_time = timer.now();
	uint64_t frame_counter = 0;
	uint64_t fps = 0;

	while (renderer.run()) {
		// CPU logic calculations

		++frame_counter;
		if (last_time + std::chrono::seconds(1) < timer.now()) {
			last_time = timer.now();
			fps = frame_counter;
			frame_counter = 0;
			std::cout << "FPS: " << fps << std::endl;
		}

		// Begin render
		w->BeginRender();
		// Record command buffer
		VkCommandBufferBeginInfo command_buffer_begin_info{};
		command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info);

		VkRect2D render_area{};
		render_area.offset.x = 0;
		render_area.offset.y = 0;
		render_area.extent = w->GetVulkanSurfaceSize();

		color_rotator += 0.001;

		std::array<VkClearValue, 2> clear_values{};
		clear_values[0].depthStencil.depth = 0.0f;
		clear_values[0].depthStencil.stencil = 0;
		clear_values[1].color.float32[0] = std::sin(color_rotator + CIRCLE_THIRD_1) * 0.5 + 0.5;
		clear_values[1].color.float32[1] = std::sin(color_rotator + CIRCLE_THIRD_2) * 0.5 + 0.5;
		clear_values[1].color.float32[2] = std::sin(color_rotator + CIRCLE_THIRD_3) * 0.5 + 0.5;
		clear_values[1].color.float32[3] = 1.0f;

		VkRenderPassBeginInfo render_pass_begin_info{};
		render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass_begin_info.renderPass = w->GetVulkanRenderPass();
		render_pass_begin_info.framebuffer = w->GetVulkanActiveFramebuffer();
		render_pass_begin_info.renderArea = render_area;
		render_pass_begin_info.clearValueCount = clear_values.size();
		render_pass_begin_info.pClearValues = clear_values.data();

		vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdEndRenderPass(command_buffer);

		vkEndCommandBuffer(command_buffer);

		// Submit command buffer
		VkSubmitInfo submit_info{};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.waitSemaphoreCount = 0;
		submit_info.pWaitSemaphores = nullptr;
		submit_info.pWaitDstStageMask = nullptr;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &command_buffer;
		submit_info.signalSemaphoreCount = 1;
		submit_info.pSignalSemaphores = &render_complete_semaphore;

		vkQueueSubmit(renderer.getQueue(), 1, &submit_info, VK_NULL_HANDLE);

		// End render
		w->EndRender({ render_complete_semaphore });
	}


	vkQueueWaitIdle(renderer.getQueue());

	vkDestroySemaphore(renderer.getDevice(), render_complete_semaphore, nullptr);
	vkDestroyCommandPool(renderer.getDevice(), command_pool, nullptr);
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
