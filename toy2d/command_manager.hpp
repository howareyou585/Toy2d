#pragma once
#include "vulkan/vulkan.hpp"
#include <functional>
namespace toy2d
{
	class CommandManager final
	{
	public:
		using RecordCmdFunc = std::function<void(vk::CommandBuffer&)>;
	public:
		CommandManager();
		~CommandManager();

		vk::CommandBuffer CreateOneCommandBuffer();
		std::vector<vk::CommandBuffer> CreateCommandBuffers(std::uint32_t count);
		void ResetCmds();
		
		void ExecuteCmd(const vk::Queue& queue,RecordCmdFunc func);
	private:
		//vk::CommandPool pool_;
		//定义CommandPool 对象
		vk::CommandPool pool_;
		//
		vk::CommandPool createCommandPool();
	};
}