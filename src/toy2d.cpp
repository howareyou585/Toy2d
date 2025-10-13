#include "toy2d/toy2d.hpp"
#include "toy2d/context.hpp"
#include "toy2d/shader.hpp"
#include "toy2d/render_processor.hpp"
namespace toy2d
{
    void Init(const std::vector<const char*>& extensions, Context::GetSurfaceCallBack cb,int w,int h)
    {
        Context::Init(extensions, cb);
        //创建swapchain
        Context::GetInstance().InitSwapChain(w, h);

        //初始化shader

        //Shader::Init("../../../vert.spv", "../../../frag.spv");
        Shader::Init("E:\\github\\Toy2d\\vert.spv", "E:\\github\\Toy2d\\frag.spv");
        //初始化layout：在pipeline之前
        Context::GetInstance().m_renderProcessor->InitLayout();
        //初始化RenderPass：在pipeline之前
        Context::GetInstance().m_renderProcessor->InitRenderPass();
        //在创建framebuffer前，要先创建RenderPass
        Context::GetInstance().m_swapChain->CreateFrameBuffers(w, h);
        //初始化渲染管线
        Context::GetInstance().m_renderProcessor->InitPipeline(w, h);
        //初始化Renderer
        Context::GetInstance().InitRenderer();
    }
    void Quit()
    {
        Context::GetInstance().m_renderer.reset();
        Context::GetInstance().m_renderProcessor.reset();
        Context::GetInstance().DestorySwapChain();
        Shader::Quit();
        Context::Quit();
    }
}