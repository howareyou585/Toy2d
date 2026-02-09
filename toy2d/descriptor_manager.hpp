#pragma once
#include "vulkan/vulkan.hpp"
namespace toy2d
{
	class DescriptorSetManager final {
	public:
		struct SetInfo
		{
			vk::DescriptorSet set;
			vk::DescriptorPool pool;
		};
		DescriptorSetManager(uint32_t maxFlight);
		~DescriptorSetManager();

		std::vector<SetInfo> AllocateBufferSets(uint32_t num);

		SetInfo AllocateImageSet();
		

		static void Init(uint32_t maxFlight)
		{
			m_instance.reset(new DescriptorSetManager(maxFlight));
		}
		static void Quit()
		{
			m_instance.reset();
		}
		static DescriptorSetManager& Instance()
		{
			return *m_instance;
		}
	
	private:
		struct PoolInfo {
			vk::DescriptorPool m_pool;
			uint32_t m_remainNum;
		};
		PoolInfo m_bufferSetPool;
		std::vector<PoolInfo> m_fulledImageSetPools;
		std::vector<PoolInfo> m_avalibleImageSetPools;
		uint32_t m_maxFlight;
		static std::unique_ptr<DescriptorSetManager> m_instance;
	private:
		void addImageSetPool();
		PoolInfo& getAvalibleImageSetPool();
	};
}