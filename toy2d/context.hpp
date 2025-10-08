#pragma once
#include <memory>
#include <cassert>
#include <optional>
#include <iostream>
#include <vector>
#include <functional>
#include "vulkan/vulkan.hpp"
#include "swapchain.hpp"
namespace toy2d
{
    class RenderProcess;
    class Context final
    {
    public:
        using GetSurfaceCallBack = std::function<VkSurfaceKHR(VkInstance)>;
        static void Init(const std::vector<const char*>& extensions, GetSurfaceCallBack, int w, int h);
        static void Quit();
        static Context &GetInstance();
        Context(const Context&) = delete;
        Context &operator=(const Context &) = delete;
        ~Context();
        void InitSwapChain(int w, int h);
        void DestorySwapChain();
    public:
        struct QueueFamliyIndices final
        {
            std::optional<uint32_t> graphicsQueue;
            std::optional<uint32_t> presentQueue; //À©Õ¹
            operator bool() const
            {
                return graphicsQueue.has_value() &&
                    presentQueue.has_value();
            }
        };

       
    private:
        Context(const std::vector<const char*>& extensions,GetSurfaceCallBack cb);
        void CreateInstance(const std::vector<const char*>& extensions);
        void PickupPhyiscalDevice();
        
        void CreateDevice();
        void QueryQueueFamilyIndices();
        void GetQueues();
    public:
        vk::PhysicalDevice _physicalDevice;
        vk::Device _device;
        vk::SurfaceKHR _surface;
        QueueFamliyIndices _queueFamilyIndices;
        std::unique_ptr<SwapChain>m_swapChain;
        std::unique_ptr<RenderProcess>m_renderProcessor;
    private:
        static std::unique_ptr<Context> m_instance;
        vk::Instance _instance;
        vk::Queue _graphicsQueue;
        vk::Queue _presentQueue;
       
    };
}