#include "Platform/OpenGl/OpenGlShader.hpp"
#include "Core/Analyzation/Debug.hpp"
#include "Utils/OpenGlUtils.hpp"

namespace Rendering
{
	namespace OpenGl
	{
		//std::uint32_t OpenGlShader::ConvertShaderTypeToGlType(const ShaderType type) const
		//{
		//	if (type == ShaderType::Fragment) return GL_FRAGMENT_SHADER;
		//	else if (type == ShaderType::Vertex) return GL_VERTEX_SHADER;

		//	LogError("Attempted to convert shader type to OpenGl shader type in OpenGlShaderProgram but type has no actions");
		//	return -1;
		//}

		static std::uint32_t CompileShader(const std::uint32_t shaderType, const char* shaderSource)
		{
			GL_CALL(const std::uint32_t shaderId = glCreateShader(shaderType));
			GL_CALL(glShaderSource(shaderId, 1, &shaderSource, nullptr));
			GL_CALL(glCompileShader(shaderId));

			int result = 0;
			GL_CALL(glGetShaderiv(shaderId, GL_COMPILE_STATUS, &result));
			if (result == GL_FALSE)
			{
				int length = 0;
				GL_CALL(glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &length));
				char* message = (char*)alloca(length * sizeof(char));
				GL_CALL(glGetShaderInfoLog(shaderId, length, &length, message));

				LogError(std::format("Failed to compile OpenGlShader shader:{} Message:{}", shaderSource, message));
				GL_CALL(glDeleteShader(shaderId));
				return 0;
			}
			return shaderId;
		}

		static RenderObjectId CreateShaderProgram(const char* vertexShader, const char* fragmentShader)
		{
			GL_CALL(std::uint32_t programId = glCreateProgram());
			GL_CALL(std::uint32_t vsId = CompileShader(GL_VERTEX_SHADER, vertexShader));
			GL_CALL(std::uint32_t fsId = CompileShader(GL_FRAGMENT_SHADER, fragmentShader));

			GL_CALL(glAttachShader(programId, vsId));
			GL_CALL(glAttachShader(programId, fsId));
			GL_CALL(glLinkProgram(programId));
			GL_CALL(glValidateProgram(programId));

			//We only need the shaders to create the single shader program, then they can be deleted
			GL_CALL(glDeleteShader(vsId));
			GL_CALL(glDeleteShader(fsId));

			return programId;
		}

		static void BindActive(const Shader& shader)
		{
			glUseProgram(shader.GetId());
		}
		static void UnbindActive(const Shader& shader)
		{
			glUseProgram(0);
		}

		Shader CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
		{
			return Shader(vertexShader, fragmentShader, ShaderPlatformCallbacks
				{
					CreateShaderProgram,
					BindActive,
					UnbindActive
				});
		}
	}
}