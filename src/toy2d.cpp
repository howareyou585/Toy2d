#include "toy2d/toy2d.hpp"
#include "toy2d/context.hpp"
#include "toy2d/shader.hpp"
#include "toy2d/render_processor.hpp"
#include "toy2d/descriptor_manager.hpp"
namespace toy2d
{
    std::unique_ptr<Renderer> renderer_;

    void Init(std::vector<const char*>& extensions, Context::GetSurfaceCallback cb, int windowWidth, int windowHeight) {
        Context::Init(extensions, cb);
        auto& ctx = Context::Instance();
        ctx.initSwapchain(windowWidth, windowHeight);
        //��ʼ��shader

        //Shader::Init("../../../vert.spv", "../../../frag.spv");
        //Shader::Init("E:\\github\\Toy2d\\vert.spv", "E:\\github\\Toy2d\\frag.spv");
        Shader::Init("E:\\github\\Toy2d_self\\build\\sandbox\\Debug\\vert.spv", "E:\\github\\Toy2d_self\\build\\sandbox\\Debug\\frag.spv");
        ctx.initRenderProcess();
        ctx.initGraphicsPipeline();
        ctx.swapchain->InitFramebuffers();
        ctx.initCommandPool();
		ctx.initSampler();// 初始化纹理采样器
        int maxFlightCount = 2;
        DescriptorSetManager::Init(maxFlightCount);
        renderer_ = std::make_unique<Renderer>();
        renderer_->setProjection(windowWidth, 0, 0, windowHeight, -1, 1);
    }

    void Quit() {
        renderer_.reset();
        DescriptorSetManager::Quit();
        Context::Quit();
    }

    Renderer* GetRenderer() {
        return renderer_.get();
    }
}