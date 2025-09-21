#pragma once
#include <memory>
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
    private:
        Context();

    private:
        static std::unique_ptr<Context> m_instance;
        vk::Instance _instance;
    };
}