#pragma once
#include "vulkan/vulkan.hpp"
namespace toy2d
{
    class Buffer;
    class Renderer {
    public:
        Renderer(int maxFlightCount = 2);
        ~Renderer();

        void DrawTriangle();

    private:
        int maxFlightCount_;
        int curFrame_;
        std::vector<vk::Fence> fences_;
        std::vector<vk::Semaphore> imageAvaliableSems_;
        std::vector<vk::Semaphore> renderFinishSems_;
        std::vector<vk::CommandBuffer> cmdBufs_;

		std::unique_ptr<Buffer> vertexBuffer_;
        void createFences();
        void createSemaphores();
        void createCmdBuffers();
        void createVertextBuffer();
        void bufferVertexData();
    };

}