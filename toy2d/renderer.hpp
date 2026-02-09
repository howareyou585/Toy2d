#pragma once
#include "vulkan/vulkan.hpp"
#include "toy2d/math.hpp"
#include "descriptor_manager.hpp"
namespace toy2d
{
    class Buffer;
    class Renderer {
    public:
        Renderer(int maxFlightCount = 2);
        ~Renderer();

        void setProjection(int right, int left, int bottom, int top, int near, int far);
        //void DrawTexture(const Rect&,)

        void DrawTriangle();


        void StartRender();
        void EndRender();
    private:
        void createFences();
        void createSemaphores();
        void createCmdBuffers();
        void createVertextBuffer();
        void createUniformBuffers(int flightcount);
        void bufferVertexData();

        void intiMats();
        void updateDescriptorSet();
        void bufferMVPData();
        void transformBuffer2Device(Buffer& src, Buffer& dst, size_t srcOffset, size_t destOffset, size_t size);
    private:
        int maxFlightCount_;
        int curFrame_;
        std::vector<vk::Fence> fences_;
        std::vector<vk::Semaphore> imageAvaliableSems_;
        std::vector<vk::Semaphore> renderFinishSems_;
        std::vector<vk::CommandBuffer> cmdBufs_;
        
        std::vector<std::unique_ptr<Buffer>> m_vecBuffer;
		std::vector<std::unique_ptr<Buffer>> m_vecDeviceUniformBuffer;//每个flight一个device local uniform buffer
		std::vector<std::unique_ptr<Buffer>> m_vecUniformBuffer;//每个flight一个uniform buffer
        
		std::vector<DescriptorSetManager::SetInfo> m_vecDescriptorSets;

		std::unique_ptr<Buffer> vertexBuffer_;

        Mat4 m_projectMat;
        Mat4 m_viewMat;

        vk::Sampler m_sampler;

        
    };

}