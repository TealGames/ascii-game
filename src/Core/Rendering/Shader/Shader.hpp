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

	enum class UniformType : std::uint8_t
	{
		Float = 0,
		Int = 1,
		Vector2 = 2,
		Vector3 = 3,
		Vector4 = 4,
		Matrix4x4 = 5,
		/// <summary>
		/// Represents a special value that holds the 
		/// texture slot index that the sampler uses for the
		/// texture. NOTE: sampler uniform must be get/set with int*
		/// </summary>
		Sampler2D	  = 6,
	};

	class Shader;
	struct ShaderPlatformCallbacks
	{
		RenderObjectId(*m_InitFunc) (const char* vertexSource, const char* fragmentSource);
		void(*m_BindActiveFunc) (const Shader& shader);
		void(*m_UnbindActiveFunc) (const Shader& shader);
		bool(*m_TrySetUniformFunc) (const Shader& shader, const UniformType uniform, const char* uniformName, const void* valuePtr);
		bool(*m_TryGetUniformFunc) (const Shader& shader, const UniformType uniform, const char* uniformName, void* outputPtr);
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

		bool TrySetUniform(const UniformType type, const char* uniformName, const void* valuePtr) const;
		bool TryGetUniform(const UniformType type, const char* uniformName, void* outputValue) const;

		std::string ToString() const;
		//ShaderType GetType() const;
	};

	Shader CreateShader(const std::string& vertexSource, const std::string& fragmentSource);
}