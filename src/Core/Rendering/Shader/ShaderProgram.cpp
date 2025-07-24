#include "Core/Rendering/Shader/ShaderProgram.hpp"
#include "Core/Analyzation/Debug.hpp"
#include "StaticGlobals.hpp"

namespace Rendering
{
	ShaderProgram::ShaderProgram(Shader& vertexShader, Shader& fragmentShader)
		: m_data{nullptr, nullptr}, m_impl()
	{
		if (vertexShader.GetType() != ShaderType::Vertex)
		{
			LogError(std::format("Attempted to create shader program with a VERTEX shader that does not have VERTEX type"));
			return;
		}
		if (fragmentShader.GetType() != ShaderType::Fragment)
		{
			LogError(std::format("Attempted to create shader program with a FRAGMENT shader that does not have FRAGMENT type"));
			return;
		}

		m_data.m_VertexShader = &vertexShader;
		m_data.m_FragmentShader = &fragmentShader;

#if defined(OPENGL)
		m_impl = OpenGlShaderProgram();
#else
		LogError("Tried to create shader program but current framework has no shader program implementation");
#endif
	}

	void ShaderProgram::CreateProgram()
	{
		std::visit([this](auto& programType)-> void
			{
				programType.CreateShaderProgram(*m_data.m_VertexShader, *m_data.m_FragmentShader);
			}, m_impl);
	}
}