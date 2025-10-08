#pragma once
#include "vulkan/vulkan.hpp"
#include <string>
#include <memory>
#include <vector>
namespace toy2d
{
	class RenderProcess final
	{
	public:
		void InitPipeline(int width, int height);
		void InitRenderPass();
		void InitLayout();
		//void DestroyPipeline();
		~RenderProcess();
	public:
		vk::Pipeline m_pipeline;
		vk::PipelineLayout m_layout;
		vk::RenderPass m_renderPass;
	};
}