#pragma once
#include "vulkan/vulkan.hpp"
namespace toy2d
{
	class CommandManager final
	{
	public:
		CommandManager();
		~CommandManager();

		vk::CommandBuffer CreateOneCommandBuffer();
		std::vector<vk::CommandBuffer> CreateCommandBuffers(std::uint32_t count);
		void ResetCmds();
	private:
		//vk::CommandPool pool_;
		//定义CommandPool 对象
		vk::CommandPool pool_;
		//
		vk::CommandPool createCommandPool();
	};
}