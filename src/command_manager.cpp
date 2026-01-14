#include "toy2d/command_manager.hpp"
#include "toy2d/context.hpp"

namespace toy2d {

    CommandManager::CommandManager() {
        pool_ = createCommandPool();
    }

    CommandManager::~CommandManager() {
        auto& ctx = Context::Instance();
        ctx.device.destroyCommandPool(pool_);
    }

    void CommandManager::ResetCmds() {
        Context::Instance().device.resetCommandPool(pool_);
    }

    vk::CommandPool CommandManager::createCommandPool() {
        /*auto& ctx = Context::Instance();

        vk::CommandPoolCreateInfo createInfo;

        createInfo.setQueueFamilyIndex(ctx.queueInfo.graphicsIndex.value())
            .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

        return ctx.device.createCommandPool(createInfo);*/
        auto& ctx = Context::Instance();
        auto& device = ctx.device;
        vk::CommandPoolCreateInfo cmdPoolCreateInfo;
        cmdPoolCreateInfo.setQueueFamilyIndex(ctx.queueInfo.graphicsIndex.value())
            .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
        
        
        return device.createCommandPool(cmdPoolCreateInfo);
    }

    std::vector<vk::CommandBuffer> CommandManager::CreateCommandBuffers(std::uint32_t count) {
        /*auto& ctx = Context::Instance();

        vk::CommandBufferAllocateInfo allocInfo;
        allocInfo.setCommandPool(pool_)
            .setCommandBufferCount(count)
            .setLevel(vk::CommandBufferLevel::ePrimary);

        return ctx.device.allocateCommandBuffers(allocInfo);*/

        auto& ctx = Context::Instance();
        auto& device = ctx.device;
        vk::CommandBufferAllocateInfo cmdbufAllocateInfo;
        cmdbufAllocateInfo.setCommandBufferCount(1)
            .setCommandPool(this->pool_)
            .setLevel(vk::CommandBufferLevel::ePrimary);
        return device.allocateCommandBuffers(cmdbufAllocateInfo);
    }

    vk::CommandBuffer CommandManager::CreateOneCommandBuffer() {
        return CreateCommandBuffers(1)[0];
    }

}