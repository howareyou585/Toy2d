#pragma once
#include "vulkan/vulkan.hpp"
#include <string>
#include <memory>
#include <vector>
namespace toy2d
{
	class Shader final
	{
	public:
		~Shader();
		static void Init(const std::string& vertexSource, const std::string& fragSource);
		static void Quit();
		static Shader& GetInstance();
	public:
		const std::vector<vk::PipelineShaderStageCreateInfo> & GetStages()const;
	public:
		vk::ShaderModule m_vertexModule;
		vk::ShaderModule m_fragmentModule;
	private:
		Shader(const std::string& vertexSource, const std::string& fragSource);
		void InitStage();
	private:
		static std::unique_ptr<Shader> m_instance;
		std::vector<vk::PipelineShaderStageCreateInfo> m_stages;
	};
}