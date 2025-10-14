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
		
		device.freeCommandBuffers(m_commandPool, m_commandBuffers);
		
		device.destroyCommandPool(m_commandPool);
		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			device.destroySemaphore(m_imageAvaliables[i]);
			device.destroySemaphore(m_imageDrawFinishs[i]);
			device.destroyFence(m_cmdAvaliableFences[i]);
		}
		/*device.destroySemaphore(m_imageAvaliable);
		device.destroySemaphore(m_imageDrawFinish);
		device.destroyFence(m_cmdAvaliableFence);*/
	}
	void Renderer::Render()
	{
		auto& device = Context::GetInstance()._device;
		auto& swapChain = Context::GetInstance().m_swapChain;
		auto& renderProcessor = Context::GetInstance().m_renderProcessor;
		////��ÿһ֡��ʼ����:�ȴ���ǰ֡��դ����ȷ�������������ʹ��
		if (device.waitForFences(m_cmdAvaliableFences[m_currentFrame], 
			true, 
			std::numeric_limits<uint64_t>().max()) != vk::Result::eSuccess)
		{
			std::cout << "wait for fence failed." << std::endl;
		}
		// ����Χ��Ϊ UNSIGNALED ״̬
		device.resetFences(m_cmdAvaliableFences[m_currentFrame]);
		//��ȡ��һ��ͼ�� ��CPU�ڴ�����....ֱ���õ�ͼ��
		auto res = device.acquireNextImageKHR(swapChain->m_swapChain, 
			std::numeric_limits<uint64_t>().max(),
			this->m_imageAvaliables[m_currentFrame]);
		if (res.result != vk::Result::eSuccess)
		{
			std::cout << "acquire next image failed!" << std::endl;
		}
		auto imageIndex = res.value;
		//���õ�ǰ֡���������
		m_commandBuffers[m_currentFrame].reset();

		//��ʼ��¼�������
		vk::CommandBufferBeginInfo beginInfo;
		beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

		m_commandBuffers[m_currentFrame].begin(beginInfo);
		{
			m_commandBuffers[m_currentFrame].bindPipeline(vk::PipelineBindPoint::eGraphics,
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
			m_commandBuffers[m_currentFrame].beginRenderPass(renderPassBeginInfo, 
				vk::SubpassContents::eInline);
			{
				//m_commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, renderProcessor->m_pipeline);
				m_commandBuffers[m_currentFrame].draw(3, 1, 0, 0);
			}
			m_commandBuffers[m_currentFrame].endRenderPass();
		}
		m_commandBuffers[m_currentFrame].end();

		//�ύ�������
		vk::SubmitInfo submitInfo;
		std::vector<vk::Semaphore> waitSemaphores = { m_imageAvaliables[m_currentFrame] };
		std::vector<vk::PipelineStageFlags> waitStages ={vk::PipelineStageFlagBits::eColorAttachmentOutput };
		submitInfo.setCommandBuffers(m_commandBuffers[m_currentFrame])
			.setWaitSemaphores(waitSemaphores)
			.setWaitDstStageMask(waitStages)//setWaitDstStageMasks
			.setSignalSemaphores(m_imageDrawFinishs[m_currentFrame]);
		//submitʱCPUҪ�ȴ�GPU���������CPUִ�п죬���ٴ�ִ��m_commandBuffer.reset() ���´���
		Context::GetInstance()._graphicsQueue.submit(submitInfo,this->m_cmdAvaliableFences[m_currentFrame]);

		
		//����ͼ��
		vk::PresentInfoKHR present;
		present.setImageIndices(imageIndex)
			.setSwapchains(swapChain->m_swapChain)
			.setWaitSemaphores(m_imageDrawFinishs[m_currentFrame]);

		if (Context::GetInstance()._presentQueue.presentKHR(present) != vk::Result::eSuccess)
		{
			std::cout << "image present failed." << std::endl;
		}

		//����֡����
		m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

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
			.setCommandBufferCount(MAX_FRAMES_IN_FLIGHT) //�������������
			.setLevel(vk::CommandBufferLevel::ePrimary);
		m_commandBuffers = device.allocateCommandBuffers(info);
	}
	void Renderer::CreateFences()
	{
		auto& device = Context::GetInstance()._device;
		
		this->m_cmdAvaliableFences.resize(MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			vk::FenceCreateInfo info;
			info.setFlags(vk::FenceCreateFlagBits::eSignaled);//��ʼ״̬Ϊsignaled
			m_cmdAvaliableFences[i] = device.createFence(info);
		}
		/*vk::FenceCreateInfo info;
		info.setFlags(vk::FenceCreateFlagBits::eSignaled);
		m_cmdAvaliableFence = device.createFence(info);*/
		
	}
	void Renderer::CreateSemaphores()
	{
		auto& device = Context::GetInstance()._device;
		
		this->m_imageAvaliables.resize(MAX_FRAMES_IN_FLIGHT);
		this->m_imageDrawFinishs.resize(MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			vk::SemaphoreCreateInfo info;
			this->m_imageAvaliables[i] = device.createSemaphore(info);
			this->m_imageDrawFinishs[i] = device.createSemaphore(info);
		}
		/*vk::SemaphoreCreateInfo info;
		this->m_imageAvaliable= device.createSemaphore(info);
		this->m_imageDrawFinish = device.createSemaphore(info);*/
	}
}