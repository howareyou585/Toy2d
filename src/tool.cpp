#include"../toy2d/tool.hpp"
namespace toy2d
{
	std::vector<char> ReadWholeFile(const std::string& fileName)
	{
		std::ifstream file(fileName, std::ios::binary | std::ios::ate);
		if (!file.is_open())
		{
			std::cout << "read " << fileName << " failed" << std::endl;
			return std::vector<char>{ };
		}
		auto size = file.tellg();
		file.seekg(0);
		std::vector<char> content(size);
		file.read(content.data(), size);
		return content;
	}
}