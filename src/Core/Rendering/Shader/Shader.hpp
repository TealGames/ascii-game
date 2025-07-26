#pragma once
#include <string>
#include <cstdint>
#include "Core/Rendering/RenderObjectId.hpp"

namespace Rendering
{
	using ShaderTypeIntegralType = std::uint8_t;
	enum class ShaderType : ShaderTypeIntegralType
	{
		Vertex =	0,
		Fragment =	1,
	};
	std::string ToString(const ShaderType type);

	class Shader;
	struct ShaderPlatformCallbacks
	{
		RenderObjectId(*m_InitFunc) (const char*, const char*);
		void(*m_BindActiveFunc) (const Shader&);
		void(*m_UnbindActiveFunc) (const Shader&);
	};

	class Shader
	{
	private:
		ShaderPlatformCallbacks m_platformCallbacks;

		RenderObjectId m_id;
		std::string m_vertexSourceCode;
		std::string m_fragmentSourceCode;
	public:

	private:
		void Init();
	public:
		Shader(const std::string& vertexSource, const std::string& fragmentSource, const ShaderPlatformCallbacks& callbacks);

		void SetSources(const std::string& vertexSource, const std::string& fragmentSource);

		RenderObjectId GetId() const;
		bool IsValid() const;

		const std::string& GetVertexSource() const;
		const char* GetVertexSourceCStyle() const;

		const std::string& GetFragmnetSource() const;
		const char* GetFragmentSourceCStyle() const;

		void BindActive() const;
		void UnbindActive() const;

		//ShaderType GetType() const;
	};

	Shader CreateShader(const std::string& vertexSource, const std::string& fragmentSource);
}