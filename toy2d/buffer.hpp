#pragma once
#include <vulkan/vulkan.hpp>
namespace toy2d
{
	class Buffer final
	{
	public:
		vk::Buffer buffer;
		vk::DeviceMemory memory;
		size_t size;
	public:
		Buffer(size_t size, vk::BufferUsageFlagBits usage, vk::MemoryPropertyFlags property);
		~Buffer();
	private:
		struct MemoryInfo final {
			size_t size;
			size_t index;
		};
		//创建buffer
		void createBuffer(size_t size, vk::BufferUsageFlagBits usage);
		//分配内存
		void allocateMemory(MemoryInfo info);
		//绑定buffer与分配的内存
		void bindingMem2Buf();
		//查询内存信息
		MemoryInfo queryMemoryInfo(vk::MemoryPropertyFlags property);
	};
}