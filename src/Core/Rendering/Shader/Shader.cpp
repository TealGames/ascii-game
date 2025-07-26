#include "Core/Rendering/Shader/Shader.hpp"
#include "Core/Analyzation/Debug.hpp"
#include "StaticGlobals.hpp"

#if defined(OPENGL)
#include "Platform/OpenGl/OpenGlShader.hpp"
#endif

namespace Rendering
{
	std::string ToString(const ShaderType type)
	{
		if (type == ShaderType::Fragment) return "Fragment";
		else if (type == ShaderType::Vertex) return "Vertex";

		LogError("Failed to convert shader type with no defined actions to string");
		return "";
	}

	Shader::Shader(const std::string& vertexSource, const std::string& fragmentSource, const ShaderPlatformCallbacks& callbacks)
		: m_platformCallbacks(callbacks), m_vertexSourceCode(vertexSource), m_fragmentSourceCode(fragmentSource), m_id(INVALID_OBJ_ID) 
	{
		Init();
	}

	void Shader::SetSources(const std::string& vertexSource, const std::string& fragmentSource)
	{
		m_vertexSourceCode = vertexSource;
		m_fragmentSourceCode = fragmentSource;
		Init();
	}

	RenderObjectId Shader::GetId() const
	{
		return m_id;
	}

	bool Shader::IsValid() const
	{
		return m_id != INVALID_OBJ_ID;
	}

	const std::string& Shader::GetVertexSource() const
	{
		return m_vertexSourceCode;
	}
	const char* Shader::GetVertexSourceCStyle() const
	{
		return m_vertexSourceCode.c_str();
	}

	const std::string& Shader::GetFragmnetSource() const
	{
		return m_fragmentSourceCode;
	}
	const char* Shader::GetFragmentSourceCStyle() const
	{
		return m_fragmentSourceCode.c_str();
	}

	void Shader::Init()
	{
		if (m_vertexSourceCode.empty() || m_fragmentSourceCode.empty())
		{
			LogWarning(std::format("Exiting from shader initialization due to empty vertex and/or fragment source code"));
			return;
		}

		m_id= m_platformCallbacks.m_InitFunc(GetVertexSourceCStyle(), GetFragmentSourceCStyle());
		if (m_id == INVALID_OBJ_ID)
		{
			LogError(std::format("Attempted to init shader with invalid render object id:{}", m_id));
		}
	}

	void Shader::BindActive() const
	{
		m_platformCallbacks.m_BindActiveFunc(*this);
	}
	void Shader::UnbindActive() const
	{
		m_platformCallbacks.m_UnbindActiveFunc(*this);
	}

	Shader CreateShader(const std::string& vertexSource, const std::string& fragmentSource)
	{
#if defined(OPENGL)
		return OpenGl::CreateShader(vertexSource, fragmentSource);
#endif
	}
}