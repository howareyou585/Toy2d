#include "../toy2d/context.hpp"
#include "../toy2d/shader.hpp"
#include "../toy2d/tool.hpp"
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
        vector<char> vecVertex = ReadWholeFile(vertexSource);
        createInfo.codeSize = vecVertex.size();
        createInfo.pCode = (const uint32_t*)(vecVertex.data());
        auto& device = Context::Instance().device;
        m_vertexModule = device.createShaderModule(createInfo);

        vector<char> vecFrag = ReadWholeFile(fragSource);
        createInfo.codeSize = vecFrag.size();
        createInfo.pCode = (const uint32_t*)(vecFrag.data());
        m_fragmentModule = device.createShaderModule(createInfo);
        InitStage();
    }
    Shader::~Shader()
    {
        auto& device = Context::Instance().device;
        device.destroyShaderModule(m_vertexModule);
        device.destroyShaderModule(m_fragmentModule);
    }

    std::vector<vk::PipelineShaderStageCreateInfo> Shader::GetStages()
    {
        return m_stages;
    }

    void Shader::InitStage()
    {
        m_stages.resize(2);
        m_stages[0].setStage(vk::ShaderStageFlagBits::eVertex)
            .setModule(m_vertexModule)
            .setPName("main");
        m_stages[1].setStage(vk::ShaderStageFlagBits::eFragment)
            .setModule(m_fragmentModule)
            .setPName("main");
    }
}