#include "../toy2d/render_processor.hpp"
#include "../toy2d/shader.hpp"
#include "../toy2d/context.hpp"
namespace toy2d
{
	void RenderProcess::InitPipeline(int width, int height)
	{
		vk::GraphicsPipelineCreateInfo createInfo;
		
		//vk::ComputePipelineCreateInfo	
		//1.Vertex Input
		vk::PipelineVertexInputStateCreateInfo inputState;
		createInfo.setPVertexInputState(&inputState);
		
		//2. Vertex Assembly
		vk::PipelineInputAssemblyStateCreateInfo inputAsm;
		inputAsm.setPrimitiveRestartEnable(false)
			.setTopology(vk::PrimitiveTopology::eTriangleList);

		createInfo.setPInputAssemblyState(&inputAsm);

		//3. Shader
		auto stages = Shader::GetInstance().GetStages();
		createInfo.setStages(stages);
		
		//4.viewport
		vk::PipelineViewportStateCreateInfo viewportState;
		vk::Viewport viewport(0, 0, width, height, 0, 1);
		viewportState.setViewports(viewport);
		vk::Rect2D rect({ 0, 0 }, { static_cast<uint32_t>(width), static_cast<uint32_t>(height) });
		viewportState.setScissors(rect);
		createInfo.setPViewportState(&viewportState);
		//5.Resterization
		vk::PipelineRasterizationStateCreateInfo rastInfo;
		rastInfo.setRasterizerDiscardEnable(false)
			.setCullMode(vk::CullModeFlagBits::eBack)
			.setFrontFace(vk::FrontFace::eCounterClockwise)
			.setPolygonMode(vk::PolygonMode::eFill)
			.setLineWidth(1);
		createInfo.setPRasterizationState(&rastInfo);

		//6.multiSample
		vk::PipelineMultisampleStateCreateInfo multiSample;
		multiSample.setSampleShadingEnable(false)
			.setRasterizationSamples(vk::SampleCountFlagBits::e1);
		createInfo.setPMultisampleState(&multiSample);

		//7.test -stencil test, depth test

		//8.color blending
		vk::PipelineColorBlendStateCreateInfo blend;
		vk::PipelineColorBlendAttachmentState attachs;
		attachs.setBlendEnable(false)
			.setColorWriteMask(vk::ColorComponentFlagBits::eA |
				vk::ColorComponentFlagBits::eB |
				vk::ColorComponentFlagBits::eG |
				vk::ColorComponentFlagBits::eR);
		blend.setLogicOpEnable(false)
			.setAttachments(attachs);
		createInfo.setPColorBlendState(&blend);

		//9.renderPass and layout
		createInfo.setRenderPass(m_renderPass)
			.setLayout(m_layout);
		//
		auto result = Context::GetInstance()._device.createGraphicsPipeline(nullptr, createInfo);
		if (result.result != vk::Result::eSuccess)
		{
			throw std::runtime_error("create graphics pipeline failed.");
		}
		this->m_pipeline = result.value;
	}
	void RenderProcess::InitRenderPass()
	{
		vk::RenderPassCreateInfo createInfo;
		vk::AttachmentDescription attachDesc;
		attachDesc.setFormat(Context::GetInstance().m_swapChain->m_info.format.format)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setSamples(vk::SampleCountFlagBits::e1);

		createInfo.setAttachments(attachDesc);
			
		vk::AttachmentReference reference;
		reference.setLayout(vk::ImageLayout::eColorAttachmentOptimal)
			.setAttachment(0);
		vk::SubpassDescription subpass;
		subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
			.setColorAttachments(reference);
		createInfo.setSubpasses(subpass);

		vk::SubpassDependency dependency;
		dependency.setSrcSubpass(VK_SUBPASS_EXTERNAL)
			.setDstSubpass(0)
			.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
			.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
			.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
		createInfo.setDependencies(dependency);

		m_renderPass = Context::GetInstance()._device.createRenderPass(createInfo);
	}
	void RenderProcess::InitLayout()
	{
		vk::PipelineLayoutCreateInfo createInfo;
		m_layout = Context::GetInstance()._device.createPipelineLayout(createInfo);
	}
	RenderProcess::~RenderProcess()
	{
		auto& device = Context::GetInstance()._device;
		device.destroyPipelineLayout(this->m_layout);
		device.destroyRenderPass(this->m_renderPass);
		device.destroyPipeline(this->m_pipeline);
		
	}
	
}