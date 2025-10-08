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
	private:
		
	};
}