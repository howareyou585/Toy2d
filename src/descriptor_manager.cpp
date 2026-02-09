#include "../toy2d/descriptor_manager.hpp"
#include "../toy2d/context.hpp"
#include "../toy2d/shader.hpp"
namespace toy2d {
	std::unique_ptr<DescriptorSetManager> DescriptorSetManager::m_instance = nullptr;
	DescriptorSetManager::DescriptorSetManager(uint32_t maxFlight) : m_maxFlight(maxFlight)
	{
		auto& device = Context::Instance().device;
		vk::DescriptorPoolSize size;
		size.setType(vk::DescriptorType::eUniformBuffer)
			.setDescriptorCount(maxFlight);
		/*vk::DescriptorPoolSize size2;
		size2.setType(vk::DescriptorType::eCombinedImageSampler)
			.setDescriptorCount(maxFlight);*/
		//size.setType(vk::DescriptorType::eCombinedImageSampler)
		vk::DescriptorPoolCreateInfo createInfo;
		createInfo.setMaxSets(maxFlight)
			.setPoolSizes(size);
		
		auto pool = device.createDescriptorPool(createInfo);
		this->m_bufferSetPool.m_pool = pool;
		this->m_bufferSetPool.m_remainNum = maxFlight;
	}
	DescriptorSetManager::~DescriptorSetManager()
	{
		auto& device = Context::Instance().device;
		device.destroyDescriptorPool(this->m_bufferSetPool.m_pool);
	}

	std::vector<DescriptorSetManager::SetInfo> DescriptorSetManager::AllocateBufferSets(uint32_t num)
	{
		auto& device = Context::Instance().device;
		vk::DescriptorSetAllocateInfo allocInfo;
		
		std::vector<vk::DescriptorSetLayout> vecLayouts(static_cast<int>(num), Shader::GetInstance().GetDescriptSetLayouts()[0]);
		allocInfo.setDescriptorPool(this->m_bufferSetPool.m_pool)
			//.setDescriptorSetCount(num)
			.setSetLayouts(vecLayouts);
			

		auto sets = device.allocateDescriptorSets(allocInfo);
		std::vector<SetInfo> result(num);
		for (int i = 0; i < sets.size(); i++)
		{
			result[i].pool = this->m_bufferSetPool.m_pool;
			result[i].set = sets[i];
		}
		return result;
	}

	DescriptorSetManager::SetInfo DescriptorSetManager::AllocateImageSet()
	{
		auto& device = Context::Instance().device;
		vk::DescriptorSetAllocateInfo allocInfo;
		std::vector<vk::DescriptorSetLayout> vecLayouts;
		vecLayouts.emplace_back(Shader::GetInstance().GetDescriptSetLayouts()[1]);
		auto& poolInfo = this->getAvalibleImageSetPool();
		allocInfo.setDescriptorPool(poolInfo.m_pool)
			.setDescriptorSetCount(1)
			.setSetLayouts(vecLayouts);
		auto sets = device.allocateDescriptorSets(allocInfo);
		SetInfo result;
		result.pool = this->m_bufferSetPool.m_pool;
		result.set = sets[0];

		poolInfo.m_remainNum = std::max<uint32_t>(poolInfo.m_remainNum - sets.size(), 0u);
		if (poolInfo.m_remainNum == 0)
		{
			this->m_fulledImageSetPools.push_back(poolInfo);
			this->m_avalibleImageSetPools.pop_back();
		}

		return result;
	}
	void DescriptorSetManager::addImageSetPool()
	{
		auto& device = Context::Instance().device;
		constexpr uint32_t MaxSetNum = 10;
		vk::DescriptorPoolSize size;
		size.setType(vk::DescriptorType::eCombinedImageSampler)
			.setDescriptorCount(MaxSetNum);
		vk::DescriptorPoolCreateInfo createInfo;
		createInfo.setMaxSets(MaxSetNum)
			.setPoolSizes(size)
			.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);
		auto pool = device.createDescriptorPool(createInfo);
		this->m_avalibleImageSetPools.push_back({pool,MaxSetNum});
		
	}
	DescriptorSetManager::PoolInfo& DescriptorSetManager::getAvalibleImageSetPool()
	{
		if (this->m_avalibleImageSetPools.size() == 0)
		{
			this->addImageSetPool();
		}
		return this->m_avalibleImageSetPools.back();
	}
}