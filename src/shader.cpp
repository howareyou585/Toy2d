#include "../toy2d/context.hpp"
#include "../toy2d/shader.hpp"

using namespace std;
namespace toy2d
{
	std::unique_ptr<Shader> Shader::m_instance = nullptr;
    void Shader::Init(const string& vertexSource, const string& fragSource)
    {
        m_instance.reset(new Shader(vertexSource, fragSource));
    }
    void Shader::Quit()
    {
        m_instance.reset();
    }
    Shader& Shader::GetInstance()
    {
        assert(m_instance);
        return *m_instance;
        
    }

    Shader::Shader(const string& vertexSource, const string& fragSource)
    {
        vk::ShaderModuleCreateInfo createInfo;
        createInfo.codeSize = vertexSource.size();
        createInfo.pCode = (const uint32_t*)vertexSource.c_str();
        auto& device = Context::GetInstance()._device;
        m_vertexModule = device.createShaderModule(createInfo);
        createInfo.codeSize = fragSource.size();
        createInfo.pCode = (const uint32_t*)fragSource.c_str();
        m_fragmentModule = device.createShaderModule(createInfo);
    }
    Shader::~Shader()
    {
        auto& device = Context::GetInstance()._device;
        device.destroyShaderModule(m_vertexModule);
        device.destroyShaderModule(m_fragmentModule);
    }
}