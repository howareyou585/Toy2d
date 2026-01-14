#include "../toy2d/buffer.hpp"
#include "../toy2d/context.hpp"
namespace toy2d
{
	Buffer::Buffer(size_t size, vk::BufferUsageFlagBits usage,vk::MemoryPropertyFlags property)
		:size(size)
	{
		createBuffer(size, usage);
		auto info = queryMemoryInfo(property);
		allocateMemory(info);
		bindingMem2Buf();
	}
	Buffer::~Buffer()
	{
		//先释放内存再销毁buffer
		Context::Instance().device.freeMemory(this->memory);
		Context::Instance().device.destroyBuffer(buffer);

	}
	void Buffer::createBuffer(size_t size, vk::BufferUsageFlagBits usage)
	{
		vk::BufferCreateInfo createInfo;
		createInfo.setSize(size)
			.setUsage(usage)
			.setSharingMode(vk::SharingMode::eExclusive);
		buffer = Context::Instance().device.createBuffer(createInfo);
	}
	//分配内存
	void Buffer::allocateMemory(MemoryInfo info)
	{
		vk::MemoryAllocateInfo allocateInfo;
		allocateInfo.setMemoryTypeIndex(info.index)
			.setAllocationSize(info.size);
		memory = Context::Instance().device.allocateMemory(allocateInfo);
	}
	//绑定buffer与分配的内存
	void Buffer::bindingMem2Buf()
	{
		Context::Instance().device.bindBufferMemory(buffer, memory, 0);
	}

	Buffer::MemoryInfo Buffer::queryMemoryInfo(vk::MemoryPropertyFlags property)
	{
		MemoryInfo info;
		auto requirments = Context::Instance().device.getBufferMemoryRequirements(buffer);
		info.size = requirments.size;
		auto properties = Context::Instance().phyDevice.getMemoryProperties();
		for (int i = 0; i < properties.memoryTypeCount; i++)
		{
			if(((1 << i) & requirments.memoryTypeBits) && (properties.memoryTypes[i].propertyFlags & property))
			{
				info.index = i;
				break;
			}
		}
		return info;

	}
}