#include "../toy2d/renderer.hpp"
#include "../toy2d/context.hpp"
#include "../toy2d/render_processor.hpp"
namespace toy2d
{
	const int Renderer::MAX_FRAMES_IN_FLIGHT = 2;
	Renderer::Renderer():m_currentFrame(0)
	{
		this->InitCmdPool();
		this->AllocCmdBuffer();
		CreateSemaphores();
		CreateFences();
	}
	Renderer::~Renderer()
	{
		auto& device = Context::GetInstance()._device;
		device.waitIdle();
		device.freeCommandBuffers(m_commandPool, m_commandBuffer);
		
		device.destroyCommandPool(m_commandPool);
		/*for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			device.destroySemaphore(m_imageAvaliables[i]);
			device.destroySemaphore(m_imageDrawFinishs[i]);
			device.destroyFence(m_cmdAvaliableFences[i]);
		}*/
		device.destroySemaphore(m_imageAvaliable);
		device.destroySemaphore(m_imageDrawFinish);
		device.destroyFence(m_cmdAvaliableFence);
	}
	void Renderer::Render()
	{
		auto& device = Context::GetInstance()._device;
		auto& swapChain = Context::GetInstance().m_swapChain;
		auto& renderProcessor = Context::GetInstance().m_renderProcessor;
		////在每一帧开始处理
		//if (device.waitForFences(m_cmdAvaliableFences[m_currentFrame], true, std::numeric_limits<uint64_t>().max()) !=
		//	vk::Result::eSuccess)
		//{
		//	std::cout << "wait for fence failed." << std::endl;
		//}
		//// 重置围栏为 UNSIGNALED 状态
		//device.resetFences(m_cmdAvaliableFences[m_currentFrame]);
		//CPU在此阻塞....
		auto res = device.acquireNextImageKHR(swapChain->m_swapChain, 
			std::numeric_limits<uint64_t>().max(),
			this->m_imageAvaliable);
		if (res.result != vk::Result::eSuccess)
		{
			std::cout << "acquire next image failed!" << std::endl;
		}
		auto imageIndex = res.value;
		m_commandBuffer.reset();

		vk::CommandBufferBeginInfo beginInfo;
		beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

		m_commandBuffer.begin(beginInfo);
		{
			m_commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, 
				renderProcessor->m_pipeline);
			vk::RenderPassBeginInfo renderPassBeginInfo;
			vk::Rect2D area;
			
			area.setOffset({ 0, 0 });
			area.setExtent(swapChain->m_info.imageExtent);

			vk::ClearValue clearValue;
			clearValue.color = vk::ClearColorValue(std::array<float, 4>{0.1, 0.1, 0.1, 1});
			renderPassBeginInfo.setRenderPass(renderProcessor->m_renderPass)
				.setRenderArea(area)
				.setFramebuffer(swapChain->m_framebuffers[imageIndex])
				.setClearValues(clearValue);
			m_commandBuffer.beginRenderPass(renderPassBeginInfo,{});
			{
				m_commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, renderProcessor->m_pipeline);
				m_commandBuffer.draw(3, 1, 0, 0);
			}
			m_commandBuffer.endRenderPass();
		}
		m_commandBuffer.end();

		vk::SubmitInfo submitInfo;
		std::array<vk::PipelineStageFlags, 1> waitStages ={vk::PipelineStageFlagBits::eColorAttachmentOutput };
		submitInfo.setCommandBuffers(m_commandBuffer)
			.setWaitSemaphores(m_imageAvaliable)
			//.setWaitDstStageMask(waitStages)//setWaitDstStageMasks
			.setSignalSemaphores(m_imageDrawFinish);
		//submit时CPU要等待GPU，否则如果CPU执行快，会再次执行m_commandBuffer.reset() 导致错误
		Context::GetInstance()._graphicsQueue.submit(submitInfo,this->m_cmdAvaliableFence);

		
		//呈现图像
		vk::PresentInfoKHR present;
		present.setImageIndices(imageIndex)
			.setSwapchains(swapChain->m_swapChain)
			.setWaitSemaphores(m_imageDrawFinish);

		if (Context::GetInstance()._presentQueue.presentKHR(present) != vk::Result::eSuccess)
		{
			std::cout << "image present failed." << std::endl;
		}

		// 等待命令缓冲区执行完成
		if (device.waitForFences(this->m_cmdAvaliableFence, true,
			std::numeric_limits<uint64_t>::max()) != vk::Result::eSuccess) //时间单位为纳秒
		{
			std::cout << "wait for fence failed" << std::endl;
		}
		//重置Fence
		device.resetFences(m_cmdAvaliableFence);
		//m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

	}

	void Renderer::InitCmdPool()
	{
		vk::CommandPoolCreateInfo info;
		info.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
		auto& device = Context::GetInstance()._device;
		this->m_commandPool = device.createCommandPool(info);
	}
	void Renderer::AllocCmdBuffer()
	{
		auto& device = Context::GetInstance()._device;
		vk::CommandBufferAllocateInfo info;
		info.setCommandPool(m_commandPool)
			.setCommandBufferCount(1)
			.setLevel(vk::CommandBufferLevel::ePrimary);
		m_commandBuffer = device.allocateCommandBuffers(info)[0];
	}
	void Renderer::CreateFences()
	{
		auto& device = Context::GetInstance()._device;
		
		/*this->m_cmdAvaliableFences.resize(MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			vk::FenceCreateInfo info;
			info.setFlags(vk::FenceCreateFlagBits::eSignaled);
			m_cmdAvaliableFences[i] = device.createFence(info);
		}*/
		vk::FenceCreateInfo info;
		//info.setFlags(vk::FenceCreateFlagBits::eSignaled);
		m_cmdAvaliableFence = device.createFence(info);
		
	}
	void Renderer::CreateSemaphores()
	{
		auto& device = Context::GetInstance()._device;
		vk::SemaphoreCreateInfo info;
		/*this->m_imageAvaliables.resize(MAX_FRAMES_IN_FLIGHT);
		this->m_imageDrawFinishs.resize(MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			this->m_imageAvaliables[i] = device.createSemaphore(info);
			this->m_imageDrawFinishs[i] = device.createSemaphore(info);
		}*/
		this->m_imageAvaliable= device.createSemaphore(info);
		this->m_imageDrawFinish = device.createSemaphore(info);
	}
}