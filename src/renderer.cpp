#include "../toy2d/renderer.hpp"
#include "../toy2d/context.hpp"
//#include "../toy2d/vertex.hpp"
#include "../toy2d/math.hpp"
#include "../toy2d/buffer.hpp"
#include "../toy2d/render_processor.hpp"
#include "../toy2d/descriptor_manager.hpp"
#include "../toy2d/math.hpp"

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
        createUniformBuffers(maxFlightCount);
        this->m_vecDescriptorSets = DescriptorSetManager::Instance().AllocateBufferSets(maxFlightCount);
        //初始化相机投影矩阵和视图矩阵
        this->intiMats();
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

    void Renderer::setProjection(int right, int left, int bottom, int top, int near, int far)
    {
        this->m_projectMat = Mat4::CreateOrtho(left, right, top, bottom, near, far);
        this->bufferMVPData();

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
        //当pipeline 指定了动态设置viewport 和 scissor，需要在此处理动态视口和裁剪设置。
        //设置视口和裁剪：
        //vk::Viewport viewport(0.f, 0.f,
        //    static_cast<float>(swapchain->GetExtent().width), static_cast<float>(swapchain->GetExtent().height),
        //    0.f, 1.0f);
        ////vk::Rect2D scissor{ {0, 0}, swapchainExtent };
        //
        //
        //cmdBufs_[curFrame_].setViewport(0, viewport);
        //cmdBufs_[curFrame_].setScissor(0, renderArea);
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

    void Renderer::createUniformBuffers(int flightcount)
    {
        this->m_vecUniformBuffer.resize(flightcount);
        size_t size = sizeof(Mat4) * 2;
        for (auto& buffer : this->m_vecUniformBuffer)
        {
			
            buffer.reset(new Buffer(size, 
                vk::BufferUsageFlagBits::eTransferSrc,
                vk::MemoryPropertyFlagBits::eHostVisible|vk::MemoryPropertyFlagBits::eHostCoherent));
                
        }
		this->m_vecDeviceUniformBuffer.resize(flightcount);
        for(auto& buffer : this->m_vecDeviceUniformBuffer)
        {
            buffer.reset(new Buffer(
                size,
                vk::BufferUsageFlagBits::eTransferDst|vk::BufferUsageFlagBits::eUniformBuffer,
                vk::MemoryPropertyFlagBits::eDeviceLocal));
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

    void Renderer::intiMats()
    {
        this->m_projectMat = Mat4::CreateIdentity();
        this->m_viewMat= Mat4::CreateIdentity();
    }

    void Renderer::updateDescriptorSet() 
    {
        auto& device = Context::Instance().device;
        for (int i = 0; i < this->m_vecDescriptorSets.size(); i++)
        {
            vk::DescriptorBufferInfo descBufferInfo;
            descBufferInfo.setBuffer(this->m_vecUniformBuffer[i]->buffer)
                .setOffset(0)
                .setRange(sizeof(Mat4) * 2);
                
                vk::WriteDescriptorSet writeDescSet;

            writeDescSet.setBufferInfo(descBufferInfo)
                .setDstSet(m_vecDescriptorSets[i].set)
                .setDstBinding(0)
                .setDstArrayElement(0)
                .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                .setDescriptorCount(1);
            
            device.updateDescriptorSets(
                writeDescSet,
                {}
            );
        }
		
    }

    void Renderer::bufferMVPData()
    {
       /* struct Matrices
        {
            Mat4 projection;
            Mat4 view;
        } matrices;*/
        auto& device = Context::Instance().device;
        for (int i = 0; i < m_vecUniformBuffer.size(); i++)
        {
            auto& uniformBuffer = m_vecUniformBuffer[i];
            memcpy(uniformBuffer->m_data, (void*)(&m_projectMat), sizeof(Mat4));
            memcpy(((char*)uniformBuffer->m_data) + sizeof(Mat4), (void*)(&m_viewMat), sizeof(Mat4));
            transformBuffer2Device(*uniformBuffer, *m_vecDeviceUniformBuffer[i], 0, 0, uniformBuffer->size);
        }
    }

    void Renderer::transformBuffer2Device(Buffer& src, Buffer& dst, size_t srcOffset, size_t destOffset, size_t size)
    {
        auto& commandManager = Context::Instance().commandManager;
        auto& graphicsQueue = Context::Instance().graphicsQueue;
        commandManager->ExecuteCmd(graphicsQueue, [&](vk::CommandBuffer& cmdbuffer) {
            vk::BufferCopy region;
            region.setSrcOffset(srcOffset)
                .setDstOffset(destOffset)
                .setSize(size);
            cmdbuffer.copyBuffer(src.buffer, dst.buffer, region);
            }
        );

    }
}