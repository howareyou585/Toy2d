#include "../toy2d/context.hpp"
#include "../toy2d/shader.hpp"
#include "../toy2d/tool.hpp"
#include "../toy2d/render_processor.hpp"
#include "../toy2d/renderer.hpp"
//#include "../toy2d/tool.hpp"
#include <iostream>
#include <string>
namespace toy2d
{
    std::unique_ptr<Context> Context::m_instance = nullptr;
    void Context::Init(const std::vector<const char*>& extensions,
        GetSurfaceCallBack cb/*,int w, int h*/)
    {
        m_instance.reset(new Context(extensions,cb));
    }
    void Context::Quit()
    {
        GetInstance().m_renderProcessor.reset();
        GetInstance().DestorySwapChain();
        Shader::Quit();
        m_instance.reset();
    }
    void Context::DestorySwapChain()
    {
        this->m_swapChain.reset();
    }
    Context& Context::GetInstance()
    {
        assert(m_instance);
        return *m_instance;
    }
    Context::~Context()
    {
        //vkDestroySurfaceKHR(instance, surface, nullptr)
        //����������instanceǰ������surface
        this->_instance.destroySurfaceKHR(_surface);
        this->_device.destroy();
        this->_instance.destroy();
        
    }
    Context::Context(const std::vector<const char*>& extensions, GetSurfaceCallBack cb)
    {
        CreateInstance(extensions);
        PickupPhyiscalDevice();
        //QueryQueueFamilyIndices(); //ΪCreateDevice��׼��
        //����deviceǰ������surface
        this->_surface = cb(_instance);
        QueryQueueFamilyIndices(); //ΪCreateDevice��׼��
        CreateDevice();
        GetQueues();
        this->m_renderProcessor.reset(new RenderProcess());
    }
    void Context::InitSwapChain(int w, int h)
    {
        m_swapChain.reset(new SwapChain(w,h));
    }
    void Context::InitRenderer()
    {
        m_renderer.reset(new Renderer());
    }
    void Context::CreateInstance(const std::vector<const char*>& extensions)
    {
        vk::InstanceCreateInfo insCreateInfo;
        vk::ApplicationInfo appInfo;
        //appInfo.setAppVersion(VK_API_VERSION_1_3);
        appInfo.setApiVersion(VK_API_VERSION_1_3);
        insCreateInfo.setPApplicationInfo(&appInfo);
        std::vector<const char*> layers = { "VK_LAYER_KHRONOS_validation" };
        insCreateInfo.setPEnabledLayerNames(layers).setPEnabledExtensionNames(extensions);
        try
        {
            this->_instance = vk::createInstance(insCreateInfo);
        }
        catch (const vk::SystemError &err)
        {
            std::cerr << "Failed to create vulkan instance:" << err.what() << std::endl;
        }
    }

    void Context::PickupPhyiscalDevice()
    {
        auto devices = _instance.enumeratePhysicalDevices();
        this->_physicalDevice = devices[0];
        std::string strDeviceName = this->_physicalDevice.getProperties().deviceName;
        //std::cout << strDeviceName << std::endl;
    }
    void Context::CreateDevice()
    {
        vk::DeviceCreateInfo createInfo;
        //��Vulkan�У�������������ͨ����չ�ṩ�ģ�VK_KHR_swapchain������˱����ڴ����߼��豸ʱ��ʽ���ø���չ��
        //�����ڴ����߼��豸��vk::Device��ʱ���Ƿ�VK_KHR_swapchain��չ��ӵ������õ���չ�б��С�
        std::array extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
        createInfo.setPEnabledExtensionNames(extensions);

        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
        //vk::DeviceQueueCreateInfo queueCreateInfo;
        float priorities = 1.0;
        if (_queueFamilyIndices.graphicsQueue.value() ==
            _queueFamilyIndices.presentQueue.value())
        {
            vk::DeviceQueueCreateInfo queueCreateInfo;
            queueCreateInfo.setPQueuePriorities(&priorities)
                .setQueueCount(1)
                .setQueueFamilyIndex(_queueFamilyIndices.graphicsQueue.value());
            queueCreateInfos.push_back(std::move(queueCreateInfo));
                

        }
        else
        {
            vk::DeviceQueueCreateInfo queueCreateInfo;
            queueCreateInfo.setPQueuePriorities(&priorities)
                .setQueueCount(1)
                .setQueueFamilyIndex(_queueFamilyIndices.graphicsQueue.value());
            queueCreateInfos.push_back(queueCreateInfo);

            queueCreateInfo.setPQueuePriorities(&priorities)
                .setQueueCount(1)
                .setQueueFamilyIndex(_queueFamilyIndices.presentQueue.value());
            queueCreateInfos.push_back(queueCreateInfo);
        }
        
        createInfo.setQueueCreateInfos(queueCreateInfos);
        _device = _physicalDevice.createDevice(createInfo);
    }

    void Context::QueryQueueFamilyIndices()
    {
        auto properties = _physicalDevice.getQueueFamilyProperties();
        for (int i = 0; i < properties.size(); i++)
        {
            const auto& property = properties[i];
            if (property.queueFlags & vk::QueueFlagBits::eGraphics)
            {
                _queueFamilyIndices.graphicsQueue = i;
                //break;
            }
            //surfaceΪ��չ
            if (_physicalDevice.getSurfaceSupportKHR(i, _surface))
            {
                _queueFamilyIndices.presentQueue = i;
            }
            if (_queueFamilyIndices)
            {
                break;
            }
        }
    }

    void Context::GetQueues()
    {
        _graphicsQueue = _device.getQueue(_queueFamilyIndices.graphicsQueue.value(), 0);
        _presentQueue = _device.getQueue(_queueFamilyIndices.presentQueue.value(), 0);
    }
}