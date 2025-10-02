#pragma once
#include "vulkan/vulkan.hpp"
namespace toy2d
{
    class SwapChain final
    {
    public:
        struct  SwapChainInfo
        {
            vk::Extent2D imageExtent;
            uint32_t imageCount;
            vk::SurfaceFormatKHR format;
            vk::SurfaceTransformFlagsKHR transform;
            vk::PresentModeKHR present;

        };
    public:
        SwapChain(int w, int h);
        ~SwapChain();

        void QueryInfo(int w, int h);

        void GetImages();
        void CreateImageViews();
    public:
        vk::SwapchainKHR m_swapChain;
        SwapChainInfo m_info;
        std::vector<vk::Image> m_images;
        std::vector<vk::ImageView> m_imageViews;
    };
}