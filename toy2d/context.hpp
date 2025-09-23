#pragma once
#include <memory>
#include <cassert>
#include <optional>
#include <iostream>
#include "vulkan/vulkan.hpp"
namespace toy2d
{
    class Context final
    {
    public:
        static void Init();
        static void Quit();
        static Context &GetInstance();
        Context(const Context&) = delete;
        Context &operator=(const Context &) = delete;
        ~Context();
    public:
        struct QueueFamliyIndices final
        {
            std::optional<uint32_t> graphicsQueue;
        };

        QueueFamliyIndices _queueFamilyIndices;
    private:
        Context();
        void CreateInstance();
        void PickupPhyiscalDevice();
        void CreateDevice();
        void QueryQueueFamilyIndices();
        void GetQueues();
    private:
        static std::unique_ptr<Context> m_instance;
        vk::Instance _instance;
        vk::PhysicalDevice _physicalDevice;
        vk::Device _device;
        vk::Queue _graphicsQueue;
    };
}