#include "../toy2d/renderer.hpp"
#include "../toy2d/context.hpp"
#include "../toy2d/vertex.hpp"
#include "../toy2d/buffer.hpp"
#include "../toy2d/render_processor.hpp"


namespace toy2d {

    const std::array<Vertex, 3> vertices = {
        Vertex{0.0f, -0.5f},
        Vertex{0.5f, 0.5f}, 
        Vertex{-0.5f, 0.5f}
	};

    Renderer::Renderer(int maxFlightCount):maxFlightCount_(maxFlightCount), curFrame_(0)
    {
        createFences();
        createSemaphores();
        createCmdBuffers();
        createVertextBuffer();
        bufferVertexData();
    }

    Renderer::~Renderer() {
        auto& device = Context::Instance().device;
        //这里是是否要调用下waitIdle，防止出现GPU还在使用，
        device.waitIdle();
        for (auto& sem : imageAvaliableSems_)
        {
            device.destroySemaphore(sem);
        }
        for (auto& sem : renderFinishSems_)
        {
            device.destroySemaphore(sem);
        }
        for (auto& fence : fences_)
        {
            device.destroyFence(fence);
        }
    }

    void Renderer::DrawTriangle() {
        auto& ctx = Context::Instance();
        auto& device = ctx.device;
        //1.cpu 等待GPU 完成之前提交的帧ID为curFrame_的任务完成，当前将要执行的帧ID也是curFrame_
        if (vk::Result::eSuccess != device.waitForFences(fences_[curFrame_], true, std::numeric_limits<uint64_t>::max()))
        {
            throw std::runtime_error("wait for fence failed.");
        }
        //2.重置fence的状态为unsignaled
        device.resetFences(fences_[curFrame_]);//可以将fence 从signaled => unsignaled
        /*if (device.waitForFences(fences_[curFrame_], true, std::numeric_limits<std::uint64_t>::max()) != vk::Result::eSuccess) {
            throw std::runtime_error("wait for fence failed");
        }
        device.resetFences(fences_[curFrame_]);*/

        auto& swapchain = ctx.swapchain;
        auto resultValue = device.acquireNextImageKHR(swapchain->swapchain, std::numeric_limits<std::uint64_t>::max(), imageAvaliableSems_[curFrame_], nullptr);
        if (resultValue.result != vk::Result::eSuccess) {
            throw std::runtime_error("wait for image in swapchain failed");
        }
        auto imageIndex = resultValue.value;

        auto& cmdMgr = ctx.commandManager;
        cmdBufs_[curFrame_].reset();

        //1.begin info
        vk::CommandBufferBeginInfo cmdBufferBeginInfo;
		cmdBufferBeginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
		cmdBufs_[curFrame_].begin(cmdBufferBeginInfo);

		//2.render pass begin info
		vk::RenderPassBeginInfo renderPassBeginInfo;
        
		vk::Rect2D renderArea;
		renderArea.setOffset({ 0, 0 })
            .setExtent(swapchain->GetExtent());
		vk::ClearValue clearValue;
        clearValue.setColor(vk::ClearColorValue(std::array<float, 4>{0.1f, 0.1f, 0.1f, 1.f}))
			/*.setDepthStencil(vk::ClearDepthStencilValue(1.0f, 0))*/; // ？？？放开注释后，背景为红色
        renderPassBeginInfo.setClearValues(clearValue)
            .setRenderPass(ctx.renderProcess->renderPass)
            .setFramebuffer(swapchain->framebuffers[imageIndex])
            .setRenderArea(renderArea); // renderArea必续要设置

		cmdBufs_[curFrame_].beginRenderPass(&renderPassBeginInfo, vk::SubpassContents::eInline);
		cmdBufs_[curFrame_].bindPipeline(vk::PipelineBindPoint::eGraphics, ctx.renderProcess->graphicsPipeline);
        vk::DeviceSize offset = 0;
        cmdBufs_[curFrame_].bindVertexBuffers(0, vertexBuffer_->buffer, offset);
        cmdBufs_[curFrame_].draw(3, 1, 0, 0);
        
        cmdBufs_[curFrame_].endRenderPass();

        //可以有多个beginRenderPass,如下：
        /*vk::RenderPassBeginInfo renderpassBeginInfo2;
        cmdBufs_[curFrame_].beginRenderPass(&renderpassBeginInfo2, vk::SubpassContents::eInline);

        cmdBufs_[curFrame_].endRenderPass();*/
        //3
        cmdBufs_[curFrame_].end();



        /*vk::CommandBufferBeginInfo beginInfo;
        beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
        cmdBufs_[curFrame_].begin(beginInfo);
        vk::ClearValue clearValue;
        clearValue.setColor(vk::ClearColorValue(std::array<float, 4>{0.1, 0.1, 0.1, 1}));
        vk::RenderPassBeginInfo renderPassBegin;
        renderPassBegin.setRenderPass(ctx.renderProcess->renderPass)
            .setFramebuffer(swapchain->framebuffers[imageIndex])
            .setClearValues(clearValue)
            .setRenderArea(vk::Rect2D({}, swapchain->GetExtent()));
        cmdBufs_[curFrame_].beginRenderPass(&renderPassBegin, vk::SubpassContents::eInline);
        cmdBufs_[curFrame_].bindPipeline(vk::PipelineBindPoint::eGraphics, ctx.renderProcess->graphicsPipeline);
        vk::DeviceSize offset = 0;
        cmdBufs_[curFrame_].bindVertexBuffers(0, vertexBuffer_->buffer, offset );
        cmdBufs_[curFrame_].draw(3, 1, 0, 0);
        cmdBufs_[curFrame_].endRenderPass();
        cmdBufs_[curFrame_].end();*/


        //提交
        vk::PipelineStageFlags stageFlg = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        vk::SubmitInfo submitInfo;
        submitInfo.setCommandBuffers(cmdBufs_[curFrame_])
            .setWaitSemaphores(imageAvaliableSems_[curFrame_])
            .setWaitDstStageMask(stageFlg)
            .setSignalSemaphores(renderFinishSems_[curFrame_]);
        //当GPU完成与Fence关联的批次命令的执行时，Fence会从unsignaled => signaled
        ctx.graphicsQueue.submit(submitInfo, fences_[curFrame_]); // 告诉vulkan, 当这次提交的所有命令都执行完毕，
                                                                  // 请将这个特定的Fence设置为signaled。

        vk::PresentInfoKHR presentInfoKHR;
        presentInfoKHR.setSwapchains(swapchain->swapchain)
            .setImageIndices(imageIndex)
            .setWaitSemaphores(renderFinishSems_[curFrame_]);

        if (vk::Result::eSuccess != ctx.presentQueue.presentKHR(presentInfoKHR))
        {
            throw std::runtime_error("present queque excute failed");
        }

        /*vk::SubmitInfo submit;
        vk::PipelineStageFlags flags = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        submit.setCommandBuffers(cmdBufs_[curFrame_])
            .setWaitSemaphores(imageAvaliableSems_[curFrame_])
            .setWaitDstStageMask(flags)
            .setSignalSemaphores(renderFinishSems_[curFrame_]);
        ctx.graphicsQueue.submit(submit, fences_[curFrame_]);

        vk::PresentInfoKHR presentInfo;
        presentInfo.setWaitSemaphores(renderFinishSems_[curFrame_])
            .setSwapchains(swapchain->swapchain)
            .setImageIndices(imageIndex);
        if (ctx.presentQueue.presentKHR(presentInfo) != vk::Result::eSuccess) {
            throw std::runtime_error("present queue execute failed");
        }*/
        
        curFrame_ = (curFrame_ + 1) % maxFlightCount_;
    }

    void Renderer::createFences() {
        /*this->fences_.resize(maxFlightCount_);
        for (auto& fence : fences_)
        {
            vk::FenceCreateInfo fenceCreateInfo;
            fenceCreateInfo.setFlags(vk::FenceCreateFlagBits::eSignaled);
            fence = Context::Instance().device.createFence(fenceCreateInfo);
        }*/
        //创建vk::Fence
        
        auto& ctx = Context::Instance();
        auto& device = ctx.device;
        this->fences_.resize(maxFlightCount_); // 设置Fence容器的大小
        for (auto& fence : this->fences_)
        {
            vk::FenceCreateInfo fenceCreateInfo;
            fenceCreateInfo.setFlags(vk::FenceCreateFlagBits::eSignaled); //初始状态设置为有信号
            fence = device.createFence(fenceCreateInfo);
        }
    }

    void Renderer::createSemaphores() {
        auto& device = Context::Instance().device;
        vk::SemaphoreCreateInfo info;
        imageAvaliableSems_.resize(maxFlightCount_);
        renderFinishSems_.resize(maxFlightCount_);
        for (auto& sem : imageAvaliableSems_)
        {
            sem = device.createSemaphore(info);
        }
        for (auto& sem : renderFinishSems_)
        {
            sem = device.createSemaphore(info);
        }
    }

    void Renderer::createCmdBuffers()
    {
        cmdBufs_.resize(maxFlightCount_);
        for (auto& cmd : cmdBufs_)
        {
            cmd = Context::Instance().commandManager->CreateOneCommandBuffer();
        }
    }

    void Renderer::createVertextBuffer()
    {
		vertexBuffer_.reset(new Buffer(sizeof(vertices),
            vk::BufferUsageFlagBits::eVertexBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible));
    }
    void Renderer::bufferVertexData()
    {
        void* ptr = Context::Instance().device.mapMemory(
            vertexBuffer_->memory, 0, sizeof(vertices)
        );
		memcpy(ptr, vertices.data(), sizeof(vertices));
		Context::Instance().device.unmapMemory(vertexBuffer_->memory);
    }
}