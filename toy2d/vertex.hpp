#pragma once
#include <vulkan/vulkan.hpp>
namespace toy2d
{
	struct  Vertex final
	{
		float x; // x coordinate
		float y; // y coordinate
		/*static vk::VertexInputAttributeDescription getAttribute()
		{
			vk::VertexInputAttributeDescription attr;
			attr.setBinding(0)
				.setFormat(vk::Format::eR32G32Sfloat)
				.setLocation(0)
				.setOffset(0);
			return attr;
		}
		static vk::VertexInputBindingDescription getBinding()
		{
			vk::VertexInputBindingDescription binding;
			binding.setBinding(0)
				.setInputRate(vk::VertexInputRate::eVertex)
				.setStride(sizeof(Vertex));
			return binding;
		}*/
		static vk::VertexInputBindingDescription getBinding()
		{
			vk::VertexInputBindingDescription binding;
			binding.setBinding(0)
				.setStride(sizeof(Vertex))
				.setInputRate(vk::VertexInputRate::eVertex);
			return binding;
		}
		static vk::VertexInputAttributeDescription getAttribute()
		{
			vk::VertexInputAttributeDescription attr;
			attr.setBinding(0)
				.setFormat(vk::Format::eR32G32Sfloat)
				.setLocation(0)
				.setOffset(0);
			return attr;
		}
	};
}