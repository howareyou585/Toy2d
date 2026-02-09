#pragma once
#include <vulkan/vulkan.hpp>
namespace toy2d
{
	class Buffer final
	{
	public:
		/*vk::Buffer buffer;
		vk::DeviceMemory memory;
		size_t size;*/
		vk::Buffer buffer; // GPU 上的一块缓存
		vk::DeviceMemory memory; //
		size_t size;//缓存的大小
		void* m_data; //数据区
	public:
		//
		Buffer(size_t size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags property);
		~Buffer();
	private:
		struct MemoryInfo final {
			size_t size;
			size_t index;
		};
		//创建buffer
		void createBuffer(size_t size, vk::BufferUsageFlags usage);
		//分配内存
		void allocateMemory(MemoryInfo info);
		//绑定buffer与分配的内存
		void bindingMem2Buf();
		//查询内存信息
		MemoryInfo queryMemoryInfo(vk::MemoryPropertyFlags property);
	};
}