#include "toy2d/toy2d.hpp"
#include "toy2d/context.hpp"
#include "toy2d/shader.hpp"
#include "toy2d/render_processor.hpp"
namespace toy2d
{
    void Init(const std::vector<const char*>& extensions, Context::GetSurfaceCallBack cb,int w,int h)
    {
        Context::Init(extensions, cb);
        //����swapchain
        Context::GetInstance().InitSwapChain(w, h);

        //��ʼ��shader

        //Shader::Init("../../../vert.spv", "../../../frag.spv");
        Shader::Init("E:\\github\\Toy2d\\vert.spv", "E:\\github\\Toy2d\\frag.spv");
        //��ʼ��layout����pipeline֮ǰ
        Context::GetInstance().m_renderProcessor->InitLayout();
        //��ʼ��RenderPass����pipeline֮ǰ
        Context::GetInstance().m_renderProcessor->InitRenderPass();
        //�ڴ���framebufferǰ��Ҫ�ȴ���RenderPass
        Context::GetInstance().m_swapChain->CreateFrameBuffers(w, h);
        //��ʼ����Ⱦ����
        Context::GetInstance().m_renderProcessor->InitPipeline(w, h);
        //��ʼ��Renderer
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