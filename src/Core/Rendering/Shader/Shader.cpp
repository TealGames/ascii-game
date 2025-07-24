#include "Core/Rendering/Shader/Shader.hpp"
#include "Core/Analyzation/Debug.hpp"

namespace Rendering
{
	std::string ToString(const ShaderType type)
	{
		if (type == ShaderType::Fragment) return "Fragment";
		else if (type == ShaderType::Vertex) return "Vertex";

		LogError("Failed to convert shader type with no defined actions to string");
		return "";
	}

	Shader::Shader(const ShaderType type, const std::string& sourceCode) 
		: m_type(type), m_sourceCode(sourceCode) {}

	const std::string& Shader::GetSource() const
	{
		return m_sourceCode;
	}
	const char* Shader::GetSourceCStyle() const
	{
		return m_sourceCode.c_str();
	}
	ShaderType Shader::GetType() const
	{
		return m_type;
	}
}