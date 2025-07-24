#pragma once
#include <string>
#include <cstdint>

namespace Rendering
{
	enum class ShaderType : std::uint8_t
	{
		Vertex =	0,
		Fragment =	1,
	};
	std::string ToString(const ShaderType type);

	class Shader
	{
	private:
		std::string m_sourceCode;
		ShaderType m_type;
	public:

	private:
	public:
		Shader(const ShaderType type, const std::string& sourceCode);

		const std::string& GetSource() const;
		const char* GetSourceCStyle() const;
		ShaderType GetType() const;
	};
}