#include "../toy2d/context.hpp"
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
        return *m_instance;
    }
    Context::~Context()
    {
        this->_instance.destroy();
    }
    Context::Context()
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
}