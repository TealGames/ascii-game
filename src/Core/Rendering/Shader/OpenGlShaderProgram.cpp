#include "Core/Rendering/Shader/OpenGlShaderProgram.hpp"
#include "Core/Analyzation/Debug.hpp"
#include "Utils/OpenGlUtils.hpp"

namespace Rendering
{
	OpenGlShaderProgram::OpenGlShaderProgram() {}

	std::uint32_t OpenGlShaderProgram::ConvertShaderTypeToGlType(const ShaderType type) const
	{
		if (type == ShaderType::Fragment) return GL_FRAGMENT_SHADER;
		else if (type == ShaderType::Vertex) return GL_VERTEX_SHADER;

		LogError("Attempted to convert shader type to OpenGl shader type in OpenGlShaderProgram but type has no actions");
		return -1;
	}

	std::uint32_t OpenGlShaderProgram::CompileShader(const Shader& shader) const
	{
		const std::uint32_t shaderId = GL_CALL(glCreateShader(ConvertShaderTypeToGlType(shader.GetType())));
		GL_CALL(glShaderSource(shaderId, 1, &shader.GetSourceCStyle(), nullptr));
		GL_CALL(glCompileShader(shaderId));

		int result = 0;
		GL_CALL(glGetShaderiv(shaderId, GL_COMPILE_STATUS, &result));
		if (result == GL_FALSE)
		{
			int length = 0;
			GL_CALL(glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &length));
			char* message = (char*)alloca(length * sizeof(char));
			GL_CALL(glGetShaderInfoLog(id, length, &length, message));

			LogError(std::format("Failed to compile OpenGlShader program shader of type:{}. Message:{}", ToString(shader.GetType()), message));
			GL_CALL(glDeleteShader(shaderId));
			return 0;
		}
		return shaderId;
	}

	std::uint32_t OpenGlShaderProgram::CreateShaderProgram(const Shader& vertexShader, const Shader& fragmentShader) const
	{
		GL_CALL(std::uint32_t programId = glCreateProgram());
		GL_CALL(std::uint32_t vsId = CompileShader(vertexShader));
		GL_CALL(std::uint32_t fsId = CompileShader(fragmentShader));

		GL_CALL(glAttachShader(programId, vsId));
		GL_CALL(glAttachShader(programId, fsId));
		GL_CALL(glLinkProgram(programId));
		GL_CALL(glValidateProgram(programId));

		//We only need the shaders to create the single shader program, then they can be deleted
		GL_CALL(glDeleteShader(vsId));
		GL_CALL(glDeleteShader(fsId));

		return programId;
	}
}