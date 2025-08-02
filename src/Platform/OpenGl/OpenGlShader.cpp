#include "Platform/OpenGl/OpenGlShader.hpp"

#ifdef OPENGL
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

		static std::uint32_t CompileShader(const RenderObjectId shaderType, const char* shaderSource)
		{
			RenderObjectId shaderId= INVALID_OBJ_ID;
			GL_CALL(shaderId = glCreateShader(shaderType));
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
			RenderObjectId programId = INVALID_OBJ_ID;
			GL_CALL(programId = glCreateProgram());

			RenderObjectId vsId = INVALID_OBJ_ID;
			RenderObjectId fsId = INVALID_OBJ_ID;
			GL_CALL(vsId = CompileShader(GL_VERTEX_SHADER, vertexShader));
			GL_CALL(fsId = CompileShader(GL_FRAGMENT_SHADER, fragmentShader));

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

		static bool TrySetShaderUniform(const Shader& shader, const UniformType uniform, const char* uniformName, const void* valuePtr)
		{
			const RenderObjectId programId = shader.GetId();
			const int location = glGetUniformLocation(programId, uniformName);
			if (location != -1)
			{
				LogError("OpenGL: Invalid uniform location. Possibly undefined uniform name or wrong spelling");
				return false;
			}
			
			if (uniform == UniformType::Float)
				GL_CALL(glUniform1f(location, *static_cast<const float*>(valuePtr)));
			else if (uniform == UniformType::Int)
				GL_CALL(glUniform1i(location, *static_cast<const int*>(valuePtr)));
			else if (uniform == UniformType::Vector2)
			{
				const float* floatArr = static_cast<const float*>(valuePtr);
				GL_CALL(glUniform2f(location, floatArr[0], floatArr[1]));
			}
			else if (uniform == UniformType::Vector3)
			{
				const float* floatArr = static_cast<const float*>(valuePtr);
				GL_CALL(glUniform3f(location, floatArr[0], floatArr[1], floatArr[2]));
			}
			else if (uniform == UniformType::Vector4)
			{
				const float* floatArr = static_cast<const float*>(valuePtr);
				GL_CALL(glUniform4f(location, floatArr[0], floatArr[1], floatArr[2], floatArr[3]));
			}
			else if (uniform == UniformType::Matrix4x4)
			{
				const float* floatMat = static_cast<const float*>(valuePtr);
				GL_CALL(glUniformMatrix4fv(location, 1, GL_FALSE, floatMat));
			}
			else
			{
				LogError("OpenGL: Uniform type has no corresponding actions");
				return false;
			}

			return true;
		}

		Shader CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
		{
			return Shader(vertexShader, fragmentShader, ShaderPlatformCallbacks
				{
					CreateShaderProgram,
					BindActive,
					UnbindActive,
					TrySetShaderUniform
				});
		}
	}
}
#endif