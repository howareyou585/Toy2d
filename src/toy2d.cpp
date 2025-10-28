#include "toy2d/toy2d.hpp"
#include "toy2d/context.hpp"
#include "toy2d/shader.hpp"
#include "toy2d/render_processor.hpp"
namespace toy2d
{
    std::unique_ptr<Renderer> renderer_;

    void Init(std::vector<const char*>& extensions, Context::GetSurfaceCallback cb, int windowWidth, int windowHeight) {
        Context::Init(extensions, cb);
        auto& ctx = Context::Instance();
        ctx.initSwapchain(windowWidth, windowHeight);
        //³õÊ¼»¯shader

        //Shader::Init("../../../vert.spv", "../../../frag.spv");
        //Shader::Init("E:\\github\\Toy2d\\vert.spv", "E:\\github\\Toy2d\\frag.spv");
        ctx.initRenderProcess();
        ctx.initGraphicsPipeline();
        ctx.swapchain->InitFramebuffers();
        ctx.initCommandPool();

        renderer_ = std::make_unique<Renderer>();
    }

    void Quit() {
        renderer_.reset();
        Context::Quit();
    }

    Renderer* GetRenderer() {
        return renderer_.get();
    }
}