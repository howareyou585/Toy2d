#include "../toy2d/swapchain.hpp"
#include "../toy2d/context.hpp"
#include "../toy2d/render_processor.hpp"
namespace toy2d
{
    Swapchain::Swapchain(vk::SurfaceKHR surface, int windowWidth, int windowHeight) 
        : surface(surface)
		
    {
        querySurfaceInfo(windowWidth, windowHeight);
		//查询交换链的共享信息
        querySharingInfo();
		//创建交换链
        swapchain = createSwapchain();
		//创建图像和图像视图
        createImageAndViews();
    }

    Swapchain::~Swapchain() {
        auto& ctx = Context::Instance();
        for (auto& img : images) {
            ctx.device.destroyImageView(img.view);
        }
        for (auto& framebuffer : framebuffers) {
            Context::Instance().device.destroyFramebuffer(framebuffer);
        }
        //ctx.device.destroySwapchainKHR(swapchain);
        ctx.device.destroySwapchainKHR(swapchain);
        ctx.instance.destroySurfaceKHR(surface);
    }

    void Swapchain::InitFramebuffers() {
        createFramebuffers();
    }

    void Swapchain::querySurfaceInfo(int windowWidth, int windowHeight) {
        surfaceInfo_.format = querySurfaceFormat();
		vk::PhysicalDevice& phyDevice = Context::Instance().phyDevice;
        vk::SurfaceCapabilitiesKHR& capability = phyDevice.getSurfaceCapabilitiesKHR(this->surface);
        surfaceInfo_.count = std::clamp(capability.minImageCount + 1, capability.minImageCount, capability.maxImageCount);
        surfaceInfo_.transform = capability.currentTransform;
		surfaceInfo_.extent = querySurfaceExtent(capability, windowWidth, windowHeight);
        
        surfaceInfo_.presentMode = queryPresentMode();
        
        /*surfaceInfo_.format = querySurfaceFormat();

        auto capability = Context::Instance().phyDevice.getSurfaceCapabilitiesKHR(surface);
        surfaceInfo_.count = std::clamp(capability.minImageCount + 1,
            capability.minImageCount, capability.maxImageCount);
        surfaceInfo_.transform = capability.currentTransform;
        surfaceInfo_.extent = querySurfaceExtent(capability, windowWidth, windowHeight);*/
        
    }

    vk::SurfaceFormatKHR Swapchain::querySurfaceFormat() {
		//通过查询可用的格式，优先选择 VK_FORMAT_R8G8B8A8_SRGB 和 VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
		vk::PhysicalDevice& phyDevice = Context::Instance().phyDevice;
        std::vector<vk::SurfaceFormatKHR> vecSurfaceFormat = phyDevice.getSurfaceFormatsKHR(this->surface);
        for (const auto& surfaceFormat : vecSurfaceFormat)
        {
            if (surfaceFormat.format == vk::Format::eR8G8B8A8Srgb && surfaceFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
            {
                return surfaceFormat;
            }
        }
		return vecSurfaceFormat[0];
       /* auto formats = Context::Instance().phyDevice.getSurfaceFormatsKHR(surface);
        for (auto& format : formats) {
            if (format.format == vk::Format::eR8G8B8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
                return format;
            }
        }
        return formats[0];*/
    }

    vk::Extent2D Swapchain::querySurfaceExtent(const vk::SurfaceCapabilitiesKHR& capability, int windowWidth, int windowHeight) {
        
        
        if (capability.currentExtent.width != std::numeric_limits<uint32_t>::max()) 
        {
            return capability.currentExtent;
        }
        else
        {
            vk::Extent2D extent={static_cast<uint32_t>(windowWidth),static_cast<uint32_t>(windowHeight)};
			extent.width = std::clamp(extent.width, capability.minImageExtent.width, capability.maxImageExtent.width);
			extent.height = std::clamp(extent.height, capability.minImageExtent.height, capability.maxImageExtent.height);
            return extent;
        }
        /*if (capability.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capability.currentExtent;
        }
        else {
            auto extent = vk::Extent2D{
                static_cast<uint32_t>(windowWidth),
                static_cast<uint32_t>(windowHeight)
            };

            extent.width = std::clamp(extent.width, capability.minImageExtent.width, capability.maxImageExtent.width);
            extent.height = std::clamp(extent.height, capability.minImageExtent.height, capability.maxImageExtent.height);
            return extent;
        }*/
        
    }

    vk::PresentModeKHR Swapchain::queryPresentMode()
    {
        vk::PhysicalDevice& phyDevice = Context::Instance().phyDevice;
        const auto& vecPresentMode = phyDevice.getSurfacePresentModesKHR(this->surface);
        for(const auto& presentMode : vecPresentMode)
        {
            if (presentMode == vk::PresentModeKHR::eMailbox)
            {
                return presentMode;
            }
		}
		return vk::PresentModeKHR::eFifo;
    }

    Swapchain::SharingInfo Swapchain::querySharingInfo()
    {
        const auto& queueInfo = Context::Instance().queueInfo;
        if (queueInfo.graphicsIndex.value() == queueInfo.presentIndex.value())
        {
            m_sharingInfo.mode = vk::SharingMode::eExclusive;
        }
        else
        {
            m_sharingInfo.mode = vk::SharingMode::eConcurrent;
			m_sharingInfo.familyIndices = { queueInfo.graphicsIndex.value(), queueInfo.presentIndex.value() };
        }
        
        return m_sharingInfo;
    }

    vk::SwapchainKHR Swapchain::createSwapchain() {
        vk::Device& device = Context::Instance().device;
        
        vk::SwapchainCreateInfoKHR swapchainCreateInfo{};
		swapchainCreateInfo.surface = surface;
        swapchainCreateInfo.setClipped(true)
            .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
            .setImageArrayLayers(1)
            .setImageColorSpace(surfaceInfo_.format.colorSpace)
            .setImageExtent(surfaceInfo_.extent)
            .setImageFormat(surfaceInfo_.format.format)
           
            .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
            .setMinImageCount(surfaceInfo_.count)
            
            .setPresentMode(surfaceInfo_.presentMode)
			
            .setSurface(surface)
			.setImageSharingMode(m_sharingInfo.mode);


        if (m_sharingInfo.mode == vk::SharingMode::eConcurrent)
        {
			swapchainCreateInfo.setQueueFamilyIndices(m_sharingInfo.familyIndices);
        }


        return device.createSwapchainKHR(swapchainCreateInfo);
        


       /* vk::SwapchainCreateInfoKHR createInfo;
        createInfo.setClipped(true)
            .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
            .setImageExtent(surfaceInfo_.extent)
            .setImageColorSpace(surfaceInfo_.format.colorSpace)
            .setImageFormat(surfaceInfo_.format.format)
            .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
            .setMinImageCount(surfaceInfo_.count)
            .setImageArrayLayers(1)
            .setPresentMode(vk::PresentModeKHR::eFifo)
            .setPreTransform(surfaceInfo_.transform)
            .setSurface(surface);

        auto& ctx = Context::Instance();
        if (ctx.queueInfo.graphicsIndex.value() == ctx.queueInfo.presentIndex.value()) {
            createInfo.setImageSharingMode(vk::SharingMode::eExclusive);
        }
        else {
            createInfo.setImageSharingMode(vk::SharingMode::eConcurrent);
            std::array queueIndices = { ctx.queueInfo.graphicsIndex.value(), ctx.queueInfo.presentIndex.value() };
            createInfo.setQueueFamilyIndices(queueIndices);
        }

        return ctx.device.createSwapchainKHR(createInfo);*/
    }

    void Swapchain::createImageAndViews() {
        auto& ctx = Context::Instance();
        auto& device = ctx.device;
        auto vecImage = device.getSwapchainImagesKHR(this->swapchain);
        for(const auto& image : vecImage)
        {
            ImageInfo imgInfo;
			vk::ImageViewCreateInfo viewCreateInfo{};
            vk::ImageSubresourceRange subresourceRange{};
            subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eColor)
                .setBaseArrayLayer(0)
                .setBaseMipLevel(0)
                .setLayerCount(1)
                .setLevelCount(1);
            viewCreateInfo.setImage(image)
                .setFormat(surfaceInfo_.format.format)
                .setViewType(vk::ImageViewType::e2D)
                .setSubresourceRange(subresourceRange)
				.setComponents(vk::ComponentMapping{});
            vk::ImageView imgView = device.createImageView(viewCreateInfo);

            imgInfo.image = image;
            imgInfo.view = imgView;
   			this->images.push_back(imgInfo);
		}
        /*auto& ctx = Context::Instance();
        auto images = ctx.device.getSwapchainImagesKHR(swapchain);
        for (auto& image : images) {
            ImageInfo img;
            img.image = image;
            vk::ImageViewCreateInfo viewCreateInfo;
            vk::ImageSubresourceRange range;
            range.setBaseArrayLayer(0)
                .setBaseMipLevel(0)
                .setLayerCount(1)
                .setLevelCount(1)
                .setAspectMask(vk::ImageAspectFlagBits::eColor);
            viewCreateInfo.setImage(image)
                .setFormat(surfaceInfo_.format.format)
                .setViewType(vk::ImageViewType::e2D)
                .setSubresourceRange(range)
                .setComponents(vk::ComponentMapping{});
            img.view = ctx.device.createImageView(viewCreateInfo);
            this->images.push_back(img);
        }*/
    }

    void Swapchain::createFramebuffers() {
        
       /* for (auto& img : images) {
            auto& view = img.view;

            vk::FramebufferCreateInfo createInfo;
            createInfo.setAttachments(view)
                .setLayers(1)
                .setHeight(GetExtent().height)
                .setWidth(GetExtent().width)
                .setRenderPass(Context::Instance().renderProcess->renderPass);

            framebuffers.push_back(Context::Instance().device.createFramebuffer(createInfo));
        }*/
        //创建FrameBuffer
        auto& ctx = Context::Instance();
        auto& device = ctx.device;
        for(auto & img : images)
        {
            auto& imgView = img.view;
            vk::FramebufferCreateInfo frameBufferCreateInfo;
            frameBufferCreateInfo.setAttachments(imgView)
                .setLayers(1)
                .setHeight(GetExtent().height)
                .setWidth(GetExtent().width)
                .setRenderPass(ctx.renderProcess->renderPass);// 这里还要设置RenderPass,与在创建Pipeline 设置RenderPass ;
                                                              // 在CommandBuffer 的BeginRenderPass 中设置RenderPsss有区别
            framebuffers.emplace_back(device.createFramebuffer(frameBufferCreateInfo));
        }
    }

}