#include "../toy2d/texture.hpp"
#include "../toy2d/context.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "../toy2d/stb_image.h"
namespace toy2d {
	Texture::~Texture()
	{
		
	}
	Texture::Texture(std::string_view filename)
	{
		int width = 0; 
		int height = 0; 
		int channel = 0;
		stbi_uc* pixels = stbi_load(filename.data(), &width, &height, &channel, STBI_rgb_alpha);
		init(pixels, width, height);
		if(!pixels)
		{
			throw std::runtime_error("failed to load texture image!");
		}
		stbi_image_free(pixels);
	}

	Texture::Texture(void* data, uint32_t w, uint32_t h)
	{
		init(data, w, h);
	}
	void Texture::createImage(uint32_t w, uint32_t h)
	{
		auto& device = Context::Instance().device;
		vk::ImageCreateInfo imageCreateInfo{};
		imageCreateInfo.setImageType(vk::ImageType::e2D)
			.setArrayLayers(1)
			.setMipLevels(1)
			.setExtent({ w, h, 1 })
			.setFormat(vk::Format::eR8G8B8A8Srgb)
			.setTiling(vk::ImageTiling::eOptimal)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setUsage(vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled);
			
		m_image = device.createImage(imageCreateInfo);
	}
	void Texture::createImageView()
	{
		auto& device = Context::Instance().device;
		vk::ImageSubresourceRange subresourceRange{};
		vk::ComponentMapping componentMapping{};
		subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eColor)
			.setBaseMipLevel(0)
			.setLevelCount(1)
			.setBaseArrayLayer(0)
			.setLayerCount(1);
		
		vk::ImageViewCreateInfo imageViewCreateInfo;
		imageViewCreateInfo.setImage(m_image)
			.setViewType(vk::ImageViewType::e2D)
			.setFormat(vk::Format::eR8G8B8A8Srgb)
			.setSubresourceRange(subresourceRange)
			.setComponents(componentMapping);
		device.createImageView(imageViewCreateInfo);
	}
	void Texture::allocMemory()
	{
		auto& device = Context::Instance().device;
		auto requirements = device.getImageMemoryRequirements(m_image);
		uint32_t memoryIndex = queryImageMemoryIndex(requirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);
		vk::MemoryAllocateInfo memAllocInfo;
		memAllocInfo.setAllocationSize(requirements.size)
			.setMemoryTypeIndex(memoryIndex);
		m_memory = device.allocateMemory(memAllocInfo);
		//device.bindImageMemory(m_image, m_memory, 0);
	}
	uint32_t Texture::queryImageMemoryIndex(uint32_t filterType, vk::MemoryPropertyFlagBits requireProperty)
	{
		auto& phyDevice = Context::Instance().phyDevice;

		vk::PhysicalDeviceMemoryProperties memProperties = phyDevice.getMemoryProperties();
		for (int i = 0; i < memProperties.memoryTypeCount; i++)
		{
			const auto& type = memProperties.memoryTypes[i];
			if (((1 << i) & filterType) && (type.propertyFlags & requireProperty))
			{
				return i;
			}
		}
		return std::numeric_limits<uint32_t>::max();
	}


	void Texture::transitionImageLayoutFromUndefined2Dst()
	{
		auto& commandManager = Context::Instance().commandManager;
		auto& graphicsQueue = Context::Instance().graphicsQueue;
		vk::ImageMemoryBarrier imgMemBarrier;
		vk::ImageSubresourceRange subresourceRange;
		subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eColor)
			.setBaseMipLevel(0)
			.setLevelCount(1)
			.setBaseArrayLayer(0)
			.setLayerCount(1);
		imgMemBarrier.setImage(m_image)
			.setOldLayout(vk::ImageLayout::eUndefined)
			.setNewLayout(vk::ImageLayout::eTransferDstOptimal)
			.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
			.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
			.setDstAccessMask(vk::AccessFlagBits::eTransferWrite)
			.setSubresourceRange(subresourceRange);
		commandManager->ExecuteCmd(graphicsQueue, [&](vk::CommandBuffer& cmdBuffer) {
			cmdBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe,
				vk::PipelineStageFlagBits::eTransfer, vk::DependencyFlags(),
				{},
				{},
				imgMemBarrier);
			});
	}
	void Texture::transitionImageLayoutFromDst2Optimal()
	{
		auto& cmdMgr = Context::Instance().commandManager;
		auto& graphicsQueue = Context::Instance().graphicsQueue;
		cmdMgr->ExecuteCmd(graphicsQueue, 
			[&](vk::CommandBuffer& cmdBuffer) {
			vk::ImageSubresourceRange subresourceRange;
			subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eColor)
				.setBaseMipLevel(0)
				.setLevelCount(1)
				.setBaseArrayLayer(0)
				.setLayerCount(1);
			vk::ImageMemoryBarrier imgMemBarrier;
			imgMemBarrier.setImage(m_image)
				.setOldLayout(vk::ImageLayout::eTransferDstOptimal)
				.setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
				.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
				.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
				.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
				.setDstAccessMask(vk::AccessFlagBits::eShaderRead)
				.setSubresourceRange(subresourceRange);

			cmdBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
				vk::PipelineStageFlagBits::eFragmentShader,
				vk::DependencyFlags(),
				{},
				{},
				imgMemBarrier);
			});
	}
	void Texture::transformData2Image(Buffer&srcBuffer, uint32_t w, uint32_t h)
	{
		auto& cmdMgr = Context::Instance().commandManager;
		auto& graphicsQueue = Context::Instance().graphicsQueue;
			
		cmdMgr->ExecuteCmd(graphicsQueue, [&](vk::CommandBuffer& cmdBuffer) {
			//cmdBuffer.copyBufferToImage();
			vk::ImageSubresourceLayers subresourceLayers;
			subresourceLayers.setAspectMask(vk::ImageAspectFlagBits::eColor)
				.setMipLevel(0)
				.setBaseArrayLayer(0)
				.setLayerCount(1);
			vk::BufferImageCopy region;

			region.setBufferImageHeight(0)
				.setBufferOffset(0)
				.setImageOffset({ 0,0,0 })
				.setImageExtent({ w, h,1 })
				.setBufferRowLength(0)
				.setImageSubresource(subresourceLayers);
			
			cmdBuffer.copyBufferToImage(srcBuffer.buffer,
				m_image,
				vk::ImageLayout::eTransferDstOptimal,
				region);
			});
	}

	void Texture::updateDescriptorSet()
	{
		vk::DescriptorImageInfo descriptorImgInfo;
		descriptorImgInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
			.setImageView(m_imageView)
			.setSampler(Context::Instance().m_sampler);
		auto& device = Context::Instance().device;

		vk::WriteDescriptorSet writeDescriptorSet;
		writeDescriptorSet.setDstSet(this->m_setInfo.set)
			.setDstBinding(0)
			.setDstArrayElement(0)
			.setDstSet(this->m_setInfo.set)
			.setDescriptorCount(1)
			.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
			.setImageInfo(descriptorImgInfo);

		device.updateDescriptorSets(writeDescriptorSet, {});
	}

	void Texture::init(void* data, uint32_t w, uint32_t h)
	{
		auto& device = Context::Instance().device;
		const uint32_t size = w * h * 4;
		Buffer stagingBuffer(size,vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
		memcpy(stagingBuffer.m_data, data, size);
		
		createImage(w, h);
		allocMemory();
		device.bindImageMemory(m_image, m_memory, 0);
		transitionImageLayoutFromUndefined2Dst();
		transformData2Image(stagingBuffer, w, h);
		transitionImageLayoutFromDst2Optimal();
		
		createImageView();
		this->m_setInfo = DescriptorSetManager::Instance().AllocateImageSet();
		updateDescriptorSet();
		
	}
	//------------------------------------------------------------------
	std::unique_ptr<TextureManager> TextureManager::m_instance = nullptr;

	Texture* TextureManager::load(std::string_view filename)
	{
		this->m_avalibleTextures.push_back(std::unique_ptr<Texture>(new Texture(filename)));
		return this->m_avalibleTextures.back().get();
	
	}
	Texture* TextureManager::create(void* data, uint32_t w, uint32_t h)
	{
		
		this->m_avalibleTextures.push_back(std::unique_ptr<Texture>(new Texture(data, w, h)));
		return this->m_avalibleTextures.back().get();
		//return nullptr;
	}

	void TextureManager::destroy(Texture* texture)
	{
		auto iter = std::find_if(m_textures.begin(), m_textures.end(), [&](const std::unique_ptr<Texture>& ptr) {
			return ptr.get() == texture;
			});
		if (iter != m_textures.end())
		{
			auto& device = Context::Instance().device;
			device.waitIdle();
			m_textures.erase(iter);
		}
		else
		{
			std::cerr << "try to destroy a texture that is not exist." << std::endl;
		}
	}

	void TextureManager::clear()
	{
		auto& device = Context::Instance().device;
		device.waitIdle();
		m_textures.clear();
	}
}