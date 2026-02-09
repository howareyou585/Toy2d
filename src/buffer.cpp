#include "../toy2d/buffer.hpp"
#include "../toy2d/context.hpp"
namespace toy2d
{
	Buffer::Buffer(size_t size, vk::BufferUsageFlags usage,vk::MemoryPropertyFlags property)
		:size(size)
	{
		createBuffer(size, usage);
		auto info = queryMemoryInfo(property);
		allocateMemory(info);
		bindingMem2Buf();
		if (property & vk::MemoryPropertyFlagBits::eHostVisible)
		{
			auto& device = Context::Instance().device;
			m_data = device.mapMemory(this->memory, 0, vk::WholeSize);
		}
		else
		{
			m_data = nullptr;
		}
	}
	Buffer::~Buffer()
	{
		auto& device = Context::Instance().device;
		if (m_data)
		{
			//unmap
			device.unmapMemory(this->memory);
		}
		//先释放内存再销毁buffer
		device.freeMemory(this->memory);
		device.destroyBuffer(buffer);

	}
	void Buffer::createBuffer(size_t size, vk::BufferUsageFlags usage)
	{
		/*vk::BufferCreateInfo createInfo;
		createInfo.setSize(size)
			.setUsage(usage)
			.setSharingMode(vk::SharingMode::eExclusive);
		buffer = Context::Instance().device.createBuffer(createInfo);*/

		auto& device = Context::Instance().device;
		vk::BufferCreateInfo bufferCreateInfo;
		bufferCreateInfo.setSize(size)
			.setUsage(/*vk::BufferUsageFlagBits::eVertexBuffer*/ usage)
			.setSharingMode(vk::SharingMode::eExclusive);
		buffer = device.createBuffer(bufferCreateInfo);

	}
	//分配内存
	void Buffer::allocateMemory(MemoryInfo info)
	{
		/*vk::MemoryAllocateInfo allocateInfo;
		allocateInfo.setMemoryTypeIndex(info.index)
			.setAllocationSize(info.size);
		memory = Context::Instance().device.allocateMemory(allocateInfo);*/



		auto& device = Context::Instance().device;
		vk::MemoryAllocateInfo memAllocInfo;
		memAllocInfo.setMemoryTypeIndex(info.index)
			.setAllocationSize(info.size);

		this->memory = device.allocateMemory(memAllocInfo);
	}
	//绑定buffer与分配的内存
	void Buffer::bindingMem2Buf()
	{
		Context::Instance().device.bindBufferMemory(buffer, memory, 0);
	}

	Buffer::MemoryInfo Buffer::queryMemoryInfo(vk::MemoryPropertyFlags property)
	{
		/*MemoryInfo info;
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
		return info;*/

		auto& phyDevice = Context::Instance().phyDevice;
		auto& device = Context::Instance().device;
		auto requirments = device.getBufferMemoryRequirements(this->buffer);
		MemoryInfo memInfo;
		memInfo.size = requirments.size;
		//requirments.memoryTypeBits
	
		vk::PhysicalDeviceMemoryProperties properites = phyDevice.getMemoryProperties();
		for (int i =0; i<properites.memoryTypeCount; i++)
		{
			const auto &type = properites.memoryTypes[i];
			if(((1 << i) & requirments.memoryTypeBits) &&(type.propertyFlags&property))
			{
				memInfo.index = i;
				break;
			}
		}
		return memInfo;
	}
}