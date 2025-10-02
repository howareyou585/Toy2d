#include "../toy2d/swapchain.hpp"
#include "../toy2d/context.hpp"
namespace toy2d
{
    SwapChain::SwapChain(int w, int h)
    {
        QueryInfo(w, h);
        vk::SwapchainCreateInfoKHR createInfo;
        createInfo.setClipped(true)
            .setImageArrayLayers(1)
            .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
            .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
            .setSurface(Context::GetInstance()._surface)
            .setImageColorSpace(m_info.format.colorSpace)
            .setImageFormat(m_info.format.format)
            .setImageExtent(m_info.imageExtent)
            .setMinImageCount(m_info.imageCount)
            .setPresentMode(m_info.present);

        auto& queueIndicecs = Context::GetInstance()._queueFamilyIndices;
        if (queueIndicecs.graphicsQueue.value() == queueIndicecs.presentQueue.value())
        {
            createInfo.setQueueFamilyIndices(queueIndicecs.graphicsQueue.value())
                .setImageSharingMode(vk::SharingMode::eExclusive);
        }
        else
        {
            std::array indices = { queueIndicecs.graphicsQueue.value(),
               queueIndicecs.presentQueue.value() };
            createInfo.setQueueFamilyIndices(indices)
                .setImageSharingMode(vk::SharingMode::eConcurrent);
        }
        this->m_swapChain = Context::GetInstance()._device.createSwapchainKHR(createInfo);
        GetImages();
        CreateImageViews();
    }
    SwapChain::~SwapChain()
    {
        for (auto i = 0; i < m_imageViews.size(); i++)
        {
            Context::GetInstance()._device.destroyImageView(m_imageViews[i]);
        }
        Context::GetInstance()._device.destroySwapchainKHR(m_swapChain);
    }

    void SwapChain::QueryInfo(int w, int h)
    {
        auto& phyDevice = Context::GetInstance()._physicalDevice;
        auto& surface = Context::GetInstance()._surface;
        auto formats = phyDevice.getSurfaceFormatsKHR(surface);
        m_info.format = formats[0];
        for (const auto& fmt : formats)
        {
            if (fmt.format == vk::Format::eR8G8B8A8Srgb &&
                fmt.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
            {
                m_info.format = fmt;
                break;
            }
        }
        auto capability = Context::GetInstance()._physicalDevice.getSurfaceCapabilitiesKHR(surface);
        m_info.imageCount = std::clamp((uint32_t)2, capability.minImageCount, capability.maxImageCount);
        m_info.transform = capability.currentTransform;
        m_info.imageExtent.width = std::clamp<uint32_t>(w, capability.minImageExtent.width, capability.maxImageExtent.width);
        m_info.imageExtent.height = std::clamp<uint32_t>(h, capability.minImageExtent.height, capability.maxImageExtent.height);
        //m_info.present
        auto presents = phyDevice.getSurfacePresentModesKHR(surface);
        m_info.present = vk::PresentModeKHR::eFifo;
        for (const auto& present : presents)
        {
            if (present == vk::PresentModeKHR::eMailbox)
            {
                m_info.present = present;
                break;
        
            }
        }
    }

    void SwapChain::GetImages()
    {
       m_images = Context::GetInstance()._device.getSwapchainImagesKHR(m_swapChain);
    }
    void SwapChain::CreateImageViews()
    {
        m_imageViews.reserve(m_images.size());
        //for (int i = 0; i < m_images.size(); i++)
        for(auto & image : m_images)
        {
            vk::ImageViewCreateInfo info;
            vk::ComponentMapping  mapping;
            vk::ImageSubresourceRange range;
            range.setBaseMipLevel(0)
                .setLevelCount(1)
                .setBaseArrayLayer(0)
                .setLayerCount(1)
                .setAspectMask(vk::ImageAspectFlagBits::eColor);
            info.setImage(image)
                .setViewType(vk::ImageViewType::e2D)
                .setComponents(mapping)
                .setFormat(m_info.format.format)// 格式必须设置，否则报错
                .setSubresourceRange(range);
            
            m_imageViews.push_back(Context::GetInstance()._device.createImageView(info));
           
        }
        
    }
}