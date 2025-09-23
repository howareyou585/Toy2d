#include "../toy2d/context.hpp"
#include <iostream>
namespace toy2d
{
    std::unique_ptr<Context> Context::m_instance = nullptr;
    void Context::Init()
    {
        m_instance.reset(new Context());
    }
    void Context::Quit()
    {
        m_instance.reset();
    }
    Context& Context::GetInstance()
    {
        assert(m_instance);
        return *m_instance;
    }
    Context::~Context()
    {
        //this->_device.destroy();
        this->_instance.destroy();
        
    }
    Context::Context()
    {
        CreateInstance();
        PickupPhyiscalDevice();
        QueryQueueFamilyIndices();
        CreateDevice();
        GetQueues();
    }
    
    void Context::CreateInstance()
    {
        vk::InstanceCreateInfo insCreateInfo;
        vk::ApplicationInfo appInfo;
        //appInfo.setAppVersion(VK_API_VERSION_1_3);
        appInfo.setApiVersion(VK_API_VERSION_1_3);
        insCreateInfo.setPApplicationInfo(&appInfo);
        std::vector<const char*> layers = { "VK_LAYER_KHRONOS_validation" };
        insCreateInfo.setPEnabledLayerNames(layers);
        this->_instance = vk::createInstance(insCreateInfo);
    }

    void Context::PickupPhyiscalDevice()
    {
        auto devices = _instance.enumeratePhysicalDevices();
        this->_physicalDevice = devices[0];
        std::cout << this->_physicalDevice.getProperties().deviceName << std::endl;
    }
    void Context::CreateDevice()
    {
        vk::DeviceCreateInfo createInfo;
        vk::DeviceQueueCreateInfo queueCreatInfo;
        float priorities = 1.0;
        queueCreatInfo.setPQueuePriorities(&priorities)
            .setQueueCount(1)
            .setQueueFamilyIndex(_queueFamilyIndices.graphicsQueue.value());
        createInfo.setQueueCreateInfos(queueCreatInfo);
        _device = _physicalDevice.createDevice(createInfo);
    }

    void Context::QueryQueueFamilyIndices()
    {
        auto properties = _physicalDevice.getQueueFamilyProperties();
        for (int i = 0; i < properties.size(); i++)
        {
            const auto& property = properties[i];
            if (property.queueFlags | vk::QueueFlagBits::eGraphics)
            {
                _queueFamilyIndices.graphicsQueue = i;
                break;
            }
        }
    }

    void Context::GetQueues()
    {
        _graphicsQueue = _device.getQueue(_queueFamilyIndices.graphicsQueue.value(), 0);
    }
}