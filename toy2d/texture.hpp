#pragma once
#include <vulkan/vulkan.hpp>
#include <string>
#include <string_view>
#include "toy2d/buffer.hpp"
#include "descriptor_manager.hpp"
namespace toy2d
{
	class TextureManager;

	class Texture final
	{
	public:
		vk::Image m_image;
		vk::ImageView m_imageView;
		vk::DeviceMemory m_memory;
		DescriptorSetManager::SetInfo m_setInfo;

	public:
		friend class TextureManager;
		~Texture();
	private:
		Texture(std::string_view filename);
		Texture(void* data, uint32_t w, uint32_t h);
		
		void createImage(uint32_t w, uint32_t h);
		void createImageView();
		void allocMemory();
		uint32_t queryImageMemoryIndex(uint32_t filterType, vk::MemoryPropertyFlagBits requireProperty);
		void transitionImageLayoutFromUndefined2Dst();
		void transitionImageLayoutFromDst2Optimal();
		void transformData2Image(Buffer&, uint32_t w, uint32_t h);

		void updateDescriptorSet();

		void init(void* data, uint32_t w, uint32_t h);

	};
	class TextureManager final
	{
	public:
		Texture* load(std::string_view filename);
		Texture* create(void* data, uint32_t w, uint32_t h);
		void destroy(Texture* texture);
		void clear();
	private:
		static std::unique_ptr<TextureManager> m_instance;
		std::vector<std::unique_ptr<Texture>> m_textures;
		std::vector<std::unique_ptr<Texture>> m_avalibleTextures;
	};
}