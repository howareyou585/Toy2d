#pragma once
#include "vulkan/vulkan.hpp"
#include "context.hpp"
#include "renderer.hpp"
namespace toy2d{

    void Init(const std::vector<const char*>& extensions, Context::GetSurfaceCallBack, int w, int h);
    void Quit();
    inline Renderer* GetRenderer()
    {
        return (Context::GetInstance().m_renderer).get();
    }
}