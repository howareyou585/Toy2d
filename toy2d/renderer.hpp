#pragma once
#include "vulkan/vulkan.hpp"
namespace toy2d
{
	class Renderer final
	{
	public:
		Renderer();
		~Renderer();
		void Render();
	private:
		void InitCmdPool();
		void AllocCmdBuffer();
		void CreateFences();
		void CreateSemaphores();
	private:
		// Ë«»º³å¼Ü¹¹
		static const int MAX_FRAMES_IN_FLIGHT ;
		size_t m_currentFrame;
		vk::CommandPool m_commandPool;
		
		std::vector<vk::Semaphore> m_imageAvaliables;
		std::vector<vk::Semaphore> m_imageDrawFinishs;
		std::vector <vk::Fence> m_cmdAvaliableFences;
		std::vector<vk::CommandBuffer> m_commandBuffers;
		/*vk::Semaphore m_imageAvaliable;
		vk::Semaphore m_imageDrawFinish;
		vk::Fence m_cmdAvaliableFence;*/
	};
}