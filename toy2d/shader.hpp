#pragma once
#include "vulkan/vulkan.hpp"
#include <string>
#include <memory>
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
		vk::ShaderModule m_vertexModule;
		vk::ShaderModule m_fragmentModule;
	private:
		Shader(const std::string& vertexSource, const std::string& fragSource);
	private:
		static std::unique_ptr<Shader> m_instance;
	};
}