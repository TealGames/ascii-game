#pragma once
#include "Core/Rendering/Shader/Shader.hpp"

namespace Rendering
{
	class OpenGlShaderProgram
	{
	private:
	public:

	private:
		std::uint32_t ConvertShaderTypeToGlType(const ShaderType type) const;
		std::uint32_t CompileShader(const Shader& shader) const;
		
	public:
		OpenGlShaderProgram();

		std::uint32_t CreateShaderProgram(const Shader& vertexShader, const Shader& fragmentShader) const;
	};
}