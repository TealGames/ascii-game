#pragma once
#include "Core/Rendering/Shader/Shader.hpp"
#include "variant"
#include "OpenGlShaderProgram.hpp"

namespace Rendering
{
	struct ShaderProgramData
	{
		Shader* m_VertexShader;
		Shader* m_FragmentShader;
	};

	class ShaderProgram
	{
	private:
		ShaderProgramData m_data;
		std::variant<OpenGlShaderProgram> m_impl;
	public:

	private:

		void CreateProgram();
	public:
		ShaderProgram(Shader& vertexShader, Shader& fragmentShader);
	};
}