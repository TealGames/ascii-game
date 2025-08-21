#include "Platform/OpenGl/OpenGlShader.hpp"

#ifdef OPENGL
#include "Core/Analyzation/Debug.hpp"
#include <format>
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
			//TODO: consider making multiple sources with one with ifdef statements to support one shader creating multiple others based on some
			//compile time flags, especially if there is a lot of repetitive stuff in multiple shader
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

		static RenderObjectId CreateShaderProgram(const char* vertexShader, const char* fragmentShader, 
			std::unordered_map<std::string, UniformBlockData>& blockData)
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

			GLint blockCount = 0;
			GL_CALL(glGetProgramiv(programId, GL_ACTIVE_UNIFORM_BLOCKS, &blockCount));

			if (blockCount > 0)
			{
				blockData.reserve(blockCount);

				constexpr size_t MAX_NAME_CHAR_COUNT = 30;
				char nameBuffer[MAX_NAME_CHAR_COUNT];
				GLsizei nameLength = 0;

				for (GLint i = 0; i < blockCount; i++) 
				{
					GL_CALL(glGetActiveUniformBlockName(programId, i, sizeof(nameBuffer), &nameLength, nameBuffer));
					blockData.emplace(std::string(nameBuffer, nameLength), UniformBlockData{});
				}
			}

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

		static RenderObjectId GetActiveShaderProgramId()
		{
			GLint currentProgramId = INVALID_OBJ_ID;
			glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgramId);
			return currentProgramId;
		}

		static bool TrySetShaderUniform(const Shader& shader, const UniformType uniform, const char* uniformName, const void* valuePtr)
		{
			const RenderObjectId programId = shader.GetId();
			if (glIsProgram(programId) == GL_FALSE)
			{
				LogError(std::format("OpenGL: Attempted to set shader:{} uniform:{} but shader program with that id does not exist",
					shader.ToString(), uniformName));
				return false;
			}

			int location = -1;
			GL_CALL(location = glGetUniformLocation(programId, uniformName));
			if (location == -1)
			{
				LogError(std::format("OpenGL: Invalid uniform location when setting shader:{} uniform:{}. "
					"Possibly undefined uniform name or wrong spelling", shader.ToString(), uniformName));
				return false;
			}

			if (GetActiveShaderProgramId() != programId)
			{
				LogError(std::format("OpenGL: Attempted to set shader:{} uniform:'{}' but that shader program is not currently bound."
					"OpenGL requires uniform setting to be done on the active shader", shader.ToString(), uniformName));
				return false;
			}

			if (uniform == UniformType::Float)
			{
				GL_CALL(glUniform1f(location, *static_cast<const float*>(valuePtr)));
			}
			else if (uniform == UniformType::Int || uniform == UniformType::Sampler2D)
			{
				GL_CALL(glUniform1i(location, *static_cast<const int*>(valuePtr)));
			}
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
		static bool TryGetShaderUniform(const Shader& shader, const UniformType uniform, const char* uniformName, void* outputPtr)
		{
			const RenderObjectId programId = shader.GetId();
			if (glIsProgram(programId) == GL_FALSE)
			{
				LogError(std::format("OpenGL: Attempted to get shader:{} uniform:{} but shader program with that id does not exist",
					shader.ToString(), uniformName));
				return false;
			}

			int location = -1;
			GL_CALL(location= glGetUniformLocation(programId, uniformName));
			if (location == -1)
			{
				LogError(std::format("OpenGL: Invalid uniform location when getting shader:{} uniform:{}. "
					"Possibly undefined uniform name or wrong spelling", shader.ToString(), uniformName));
				return false;
			}

			if (uniform == UniformType::Float || uniform == UniformType::Vector2 || uniform == UniformType::Vector3
				|| uniform == UniformType::Vector4 || uniform == UniformType::Matrix4x4)
			{
				GL_CALL(glGetUniformfv(programId, location, static_cast<float*>(outputPtr)));
			}
			else if (uniform == UniformType::Int || uniform == UniformType::Sampler2D)
			{
				GL_CALL(glGetUniformiv(programId, location, static_cast<int*>(outputPtr)));
			}
			else
			{
				LogError("OpenGL: Uniform type has no corresponding actions");
				return false;
			}

			return true;
		}

		static bool TryBindShaderUniformBlock(const Shader& shader, const char* uniformName, const UniformBufferBindIndex bindIndex)
		{
			const RenderObjectId programId = shader.GetId();
			if (glIsProgram(programId) == GL_FALSE)
			{
				LogError(std::format("OpenGL: Attempted to set shader:{} uniform block:{} but shader program with that id does not exist",
					shader.ToString(), uniformName));
				return false;
			}

			GLuint blockIndex = GL_INVALID_INDEX;
			GL_CALL(blockIndex = glGetUniformBlockIndex(shader.GetId(), uniformName));
			if (blockIndex == GL_INVALID_INDEX)
			{
				LogError(std::format("OpenGL: Invalid uniform block location when setting shader:{} uniform block:{}. "
					"Possibly undefined uniform name or wrong spelling", shader.ToString(), uniformName));
				return false;
			}
			GL_CALL(glUniformBlockBinding(shader.GetId(), blockIndex, bindIndex));
			return true;
		}
		static bool TryGetUniformBlockMembers(const Shader& shader, const char* uniformName, std::vector<UniformBlockMember>& members, size_t* fullSize)
		{
			const RenderObjectId programId = shader.GetId();
			if (glIsProgram(programId) == GL_FALSE)
			{
				LogError(std::format("OpenGL: Attempted to get shader:{} uniform block members for:{} but shader program with that id does not exist",
					shader.ToString(), uniformName));
				return false;
			}

			GLuint blockIndex = GL_INVALID_INDEX;
			GL_CALL(blockIndex = glGetUniformBlockIndex(shader.GetId(), uniformName));
			if (blockIndex == GL_INVALID_INDEX)
			{
				LogError(std::format("OpenGL: Invalid uniform block location when getting uniform block members of shader:{} uniform block:{}. "
					"Possibly undefined uniform name or wrong spelling. NOTE: USE THE UNIFORM BLOCK TYPE NOT THE INSTANCE NAME", shader.ToString(), uniformName));
				return false;
			}

			GLint uniformCount = 0;
			GL_CALL(glGetActiveUniformBlockiv(shader.GetId(), blockIndex, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &uniformCount));

			GLuint* uniformIndices = (GLuint*)alloca(sizeof(GLuint) * uniformCount);
			//Note: for some reason opengl gives indices as ints, but we need to use them as uints -> 
			// shouldn't be a problem just reinterpreting (assuming the index < INT_MAX)
			GL_CALL(glGetActiveUniformBlockiv(shader.GetId(), blockIndex, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, reinterpret_cast<GLint*>(uniformIndices)));

			GLint* offsets= (GLint*)alloca(sizeof(GLint) * uniformCount);
			GLint* sizes= (GLint*)alloca(sizeof(GLint) * uniformCount);
			GLint* arrayStrides = (GLint*)alloca(sizeof(GLint) * uniformCount);
			GLint* matrixStrides= (GLint*)alloca(sizeof(GLint) * uniformCount);

			GL_CALL(glGetActiveUniformsiv(shader.GetId(), uniformCount, uniformIndices, GL_UNIFORM_OFFSET, offsets));
			GL_CALL(glGetActiveUniformsiv(shader.GetId(), uniformCount, uniformIndices, GL_UNIFORM_SIZE, sizes));
			GL_CALL(glGetActiveUniformsiv(shader.GetId(), uniformCount, uniformIndices, GL_UNIFORM_ARRAY_STRIDE, arrayStrides));
			GL_CALL(glGetActiveUniformsiv(shader.GetId(), uniformCount, uniformIndices, GL_UNIFORM_MATRIX_STRIDE, matrixStrides));

			members.reserve(uniformCount);

			constexpr size_t MAX_NAME_LENGTH = 50;
			char nameBuffer[MAX_NAME_LENGTH] = {};
			GLsizei nameLength = 0;
			std::string nameStr = "";
			for (size_t i = 0; i < uniformCount; i++)
			{
				GL_CALL(glGetActiveUniformName(shader.GetId(), uniformIndices[i], sizeof(nameBuffer), &nameLength, nameBuffer));
				if (nameLength > MAX_NAME_LENGTH)
				{
					LogError(std::format("OpenGL: Attempted to read uniform buffer block:{} of shader:{} "
						"but found uniform member at index:{} with name size:{} greater than max size:{}", 
						uniformName, shader.ToString(), i, nameLength, MAX_NAME_LENGTH));
					return false;
				}
				//Note: we cutoff the first part to dot since that is the block type or instance name which we dont need
				nameStr = std::string(nameBuffer, nameLength);
				nameStr= nameStr.substr(nameStr.find('.') + 1);
				//LogWarning(std::format("Creating name str:{} len:{} og:{}", nameStr, nameLength, std::string(nameBuffer, nameLength)));

				if (i < members.size())
					members[i] = UniformBlockMember(nameStr, offsets[i], sizes[i], arrayStrides[i], matrixStrides[i]);
				else members.emplace_back(nameStr, offsets[i], sizes[i], arrayStrides[i]);
			}
			if (fullSize != nullptr)
			{
				GLint blockSize = 0;
				GL_CALL(glGetActiveUniformBlockiv(shader.GetId(), blockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize));
				*fullSize = blockSize;
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
					TrySetShaderUniform,
					TryGetShaderUniform,
					TryBindShaderUniformBlock,
					TryGetUniformBlockMembers
				});
		}
	}
}
#endif