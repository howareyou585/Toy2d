#pragma once
#include "vulkan/vulkan.hpp"
namespace toy2d
{
    class Swapchain final {
    public:
        struct ImageInfo {
            vk::Image image; 
            vk::ImageView view;
        };

        vk::SurfaceKHR surface = nullptr;
        vk::SwapchainKHR swapchain = nullptr;
        std::vector<ImageInfo> images;
        std::vector<vk::Framebuffer> framebuffers;

        const auto& GetExtent() const { return surfaceInfo_.extent; }
        const auto& GetFormat() const { return surfaceInfo_.format; }

        Swapchain(vk::SurfaceKHR, int windowWidth, int windowHeight);
        ~Swapchain();

        void InitFramebuffers();

    private:
        struct SurfaceInfo {
			vk::SurfaceFormatKHR format;// 包括format和colorSpace
            vk::Extent2D extent;
            std::uint32_t count;
            vk::SurfaceTransformFlagBitsKHR transform;
			vk::PresentModeKHR presentMode;//显示模式
            
        } surfaceInfo_;

        struct SharingInfo
        {
            vk::SharingMode mode;
			std::vector<uint32_t> familyIndices;
        } m_sharingInfo;

        vk::SwapchainKHR createSwapchain();

        void querySurfaceInfo(int windowWidth, int windowHeight);
        vk::SurfaceFormatKHR querySurfaceFormat();
        vk::Extent2D querySurfaceExtent(const vk::SurfaceCapabilitiesKHR& capability, int windowWidth, int windowHeight);
		//查询交换链的显示模式
		vk::PresentModeKHR queryPresentMode();
		//查询交换链的共享信息
		SharingInfo querySharingInfo();
        void createImageAndViews();
        void createFramebuffers();
    };
}