#include "Platform/OpenGl/OpenGlShader.hpp"
#include <iostream>

#ifdef OPENGL
#include "Utils/Debug.hpp"
#include <format>
#include "Utils/Platform/OpenGlUtils.hpp"

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

		static GLenum GetShaderType(const ShaderType type)
		{
			if (type == ShaderType::Vertex)
				return GL_VERTEX_SHADER;
			else if (type == ShaderType::Fragment)
				return GL_FRAGMENT_SHADER;
			else if (type == ShaderType::Compute)
				return GL_COMPUTE_SHADER;
			
			LogError(std::format("[OpenGL]: Attempted to get shader type to Opengl type"));
			return 0;
		}

		static RenderObjectId CompileShader(const FinalShaderInitData& shaderSource)
		{
			RenderObjectId shaderId= INVALID_OBJ_ID;
			GL_CALL(shaderId = glCreateShader(GetShaderType(shaderSource.m_Type)));
			
			//TODO: consider making multiple sources with one with ifdef statements to support one shader creating multiple others based on some
			//compile time flags, especially if there is a lot of repetitive stuff in multiple shader
			const char* shaderSources[] = 
			{
				//NOTE: yes we could check if source has header and then not add it 
				//but since header must come first ALWAYS it is easier to do it this way
				shaderSource.m_fullSource.data()
			};
			GL_CALL(glShaderSource(shaderId, 1, shaderSources, nullptr));
			GL_CALL(glCompileShader(shaderId));

			int result = 0;
			GL_CALL(glGetShaderiv(shaderId, GL_COMPILE_STATUS, &result));
			if (result == GL_FALSE)
			{
				int length = 0;
				GL_CALL(glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &length));
				char* message = (char*)alloca(length * sizeof(char));
				GL_CALL(glGetShaderInfoLog(shaderId, length, &length, message));

				LogError(std::format("[OpenGL]: Failed to compile shader:{} Message:{}", 
					shaderSource.m_fullSource, message));
				GL_CALL(glDeleteShader(shaderId));
				return INVALID_OBJ_ID;
			}

			//LogWarning(std::format("Compiled source: {}", shaderSource.m_fullSource));
			return shaderId;
		}

		static RenderObjectId CreateShaderProgram(const FinalShaderInitData& firstInitData, const FinalShaderInitData* secondInitData,
			UniformReflectionCollectionType* blockData)
		{
			RenderObjectId programId = INVALID_OBJ_ID;
			GL_CALL(programId = glCreateProgram());
			//LogWarning("Created program: {}"+programId);

			RenderObjectId idSource1 = INVALID_OBJ_ID;
			RenderObjectId idSource2 = INVALID_OBJ_ID;
			GL_CALL(idSource1 = CompileShader(firstInitData));
			GL_CALL(glAttachShader(programId, idSource1));

			if (secondInitData != nullptr)
			{
				GL_CALL(idSource2 = CompileShader(*secondInitData));
				GL_CALL(glAttachShader(programId, idSource2));
			}			
			
			//std::cout << "GL_VERSION: " << glGetString(GL_VERSION) << "\n";
			//std::cout << "GL_SHADING_LANGUAGE_VERSION: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n";

			/*
			GLint count = 0;
			glGetProgramiv(programId, GL_ATTACHED_SHADERS, &count);
			std::vector<GLuint> attached(count);
			glGetAttachedShaders(programId, count, nullptr, attached.data());
			for (auto id : attached) {
				GLint type;
				glGetShaderiv(id, GL_SHADER_TYPE, &type);
				std::cout << "Attached type: 0x" << std::hex << type << "\n";
			}
			*/
			//LogWarning("Is program: "+ glIsProgram(programId)!=GL_FALSE? "TRUE" : "FALSE");

			//TODO: this right now causes problems for compute shader creation
			GL_CALL(glLinkProgram(programId));
			GLint linkStatus = 0;
			GL_CALL(glGetProgramiv(programId, GL_LINK_STATUS, &linkStatus));
			if (!linkStatus) 
			{
				char log[1024];
				glGetProgramInfoLog(programId, 1024, nullptr, log);
				LogError(std::format("[OPENGL]: Failed to link due to error: {}", log));
				return INVALID_OBJ_ID;
			}

			//NOTE: on some drivers validate program on compute shaders may be bugger so we avoid it
			if (firstInitData.m_Type != ShaderType::Compute)
			{
				GL_CALL(glValidateProgram(programId));
			}

			//We only need the shaders to create the single shader program, then they can be deleted
			GL_CALL(glDeleteShader(idSource1));
			if (idSource2 != INVALID_OBJ_ID) GL_CALL(glDeleteShader(idSource2));

			if (blockData == nullptr)
				return programId;

			GLint blockCount = 0;
			GLint uniformCount = 0;
			GL_CALL(glGetProgramiv(programId, GL_ACTIVE_UNIFORM_BLOCKS, &blockCount));
			GL_CALL(glGetProgramiv(programId, GL_ACTIVE_UNIFORMS, &uniformCount));

			blockData->reserve(blockCount + uniformCount);
			constexpr size_t MAX_NAME_CHAR_COUNT = 16;
			char nameBuffer[MAX_NAME_CHAR_COUNT];
			GLsizei nameLength = 0;

			if (blockCount > 0)
			{
				for (GLint i = 0; i < blockCount; i++)
				{
					GL_CALL(glGetActiveUniformBlockName(programId, i, sizeof(nameBuffer), &nameLength, nameBuffer));
					blockData->emplace(FixedString<MAX_NAME_CHAR_COUNT>(nameBuffer, nameLength), UniformReflectionInfo{ UniformType::Buffer });
				}
			}
			if (uniformCount > 0)
			{
				GLint elementSize = 0;
				GLenum dataType = 0;
				for (GLint i = 0; i < uniformCount; i++)
				{
					GL_CALL(glGetActiveUniform(programId, i, sizeof(nameBuffer), &nameLength, &elementSize, &dataType, nameBuffer));
					blockData->emplace(FixedString<MAX_NAME_CHAR_COUNT>(nameBuffer, nameLength),
						UniformReflectionInfo{ elementSize > 1 ? UniformType::Array : UniformType::Single });
				}
			}

			return programId;
		}
		static void DeleteProgram(const Shader& shader)
		{
			GL_CALL(glDeleteProgram(shader.GetId()));
		}
		static void BindActive(const Shader& shader)
		{
			GL_CALL(glUseProgram(shader.GetId()));
		}
		static void UnbindActive(const Shader& shader)
		{
			GL_CALL(glUseProgram(0));
		}
		static void DispatchComputeShaderGroups(std::uint32_t x, std::uint32_t y, std::uint32_t z)
		{
			GLint activeProgram = INVALID_OBJ_ID;
			GL_CALL(glGetIntegerv(GL_CURRENT_PROGRAM, &activeProgram));
			if (activeProgram == INVALID_OBJ_ID)
			{
				LogError(std::format("Attempted to dispatch compute shader groups "
					"but no program is active right now"));
				return;
			}

			GL_CALL(glDispatchCompute(x, y, z));
		}

		static RenderObjectId GetActiveShaderProgramId()
		{
			GLint currentProgramId = INVALID_OBJ_ID;
			glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgramId);
			return currentProgramId;
		}

		static std::string TrySetShaderUniform(const Shader& shader, const UniformDataType uniform, 
			const char* uniformName, const void* valuePtr)
		{
			const RenderObjectId programId = shader.GetId();
			if (glIsProgram(programId) == GL_FALSE)
			{
				return std::format("OpenGL: Attempted to set shader:{} uniform:{} "
					"but shader program with that id does not exist", shader.ToString(), uniformName);
			}

			int location = -1;
			GL_CALL(location = glGetUniformLocation(programId, uniformName));
			if (location == -1)
			{
				return std::format("OpenGL: Invalid uniform location when setting shader:{} uniform:{}. "
					"Possibly undefined uniform name or wrong spelling", shader.ToString(), uniformName);
			}

			if (GetActiveShaderProgramId() != programId)
			{
				return std::format("OpenGL: Attempted to set shader:{} uniform:'{}' but that shader program is not currently bound."
					"OpenGL requires uniform setting to be done on the active shader", shader.ToString(), uniformName);
			}

			if (uniform == UniformDataType::Bool)
			{
				GL_CALL(glProgramUniform1f(programId, location, *static_cast<const GLboolean*>(valuePtr)));
			}
			else if (uniform == UniformDataType::Float)
			{
				GL_CALL(glProgramUniform1f(programId, location, *static_cast<const GLfloat*>(valuePtr)));
			}
			else if (uniform == UniformDataType::Int || uniform == UniformDataType::Sampler2D 
				|| uniform == UniformDataType::CubeSampler || uniform == UniformDataType::Image2D)
			{
				GL_CALL(glProgramUniform1i(programId, location, *static_cast<const GLint*>(valuePtr)));
			}
			else if (uniform == UniformDataType::Vector2)
			{
				const float* floatArr = static_cast<const GLfloat*>(valuePtr);
				GL_CALL(glProgramUniform2f(programId, location, floatArr[0], floatArr[1]));
			}
			else if (uniform == UniformDataType::Vector3)
			{
				const float* floatArr = static_cast<const GLfloat*>(valuePtr);
				GL_CALL(glProgramUniform3f(programId, location, floatArr[0], floatArr[1], floatArr[2]));
			}
			else if (uniform == UniformDataType::Vector4)
			{
				const float* floatArr = static_cast<const GLfloat*>(valuePtr);
				GL_CALL(glProgramUniform4f(programId, location, floatArr[0], floatArr[1], floatArr[2], floatArr[3]));
			}
			else if (uniform == UniformDataType::Matrix4x4)
			{
				const float* floatMat = static_cast<const GLfloat*>(valuePtr);
				GL_CALL(glProgramUniformMatrix4fv(programId, location, 1, GL_FALSE, floatMat));
			}
			else
			{
				return "OpenGL: Uniform type has no corresponding actions";
			}

			return "";
		}
		static std::string TrySetShaderUniformArray(const Shader& shader, const UniformDataType uniform, 
			const char* uniformName, const void* valuePtr, const size_t writeElementCount)
		{
			const RenderObjectId programId = shader.GetId();
			if (glIsProgram(programId) == GL_FALSE)
			{
				return std::format("OpenGL: Attempted to set shader:{} uniform:{} but shader program with that id does not exist",
					shader.ToString(), uniformName);
			}

			int location = -1;
			GL_CALL(location = glGetUniformLocation(programId, uniformName));
			if (location == -1)
			{
				return std::format("OpenGL: Invalid uniform location when setting shader:{} uniform:{}. "
					"Possibly undefined uniform name or wrong spelling", shader.ToString(), uniformName);
			}

			if (GetActiveShaderProgramId() != programId)
			{
				return std::format("OpenGL: Attempted to set shader:{} uniform:'{}' but that shader program is not currently bound."
					"OpenGL requires uniform setting to be done on the active shader", shader.ToString(), uniformName);
			}

			if (uniform == UniformDataType::Bool)
			{
				GLint* values = static_cast<GLint*>(alloca(sizeof(GLint) * writeElementCount));
				const bool* boolArr = static_cast<const bool*>(valuePtr);
				for (size_t i = 0; i < writeElementCount; i++)
					values[i] = boolArr[i] ? 1 : 0;

				GL_CALL(glProgramUniform1iv(programId, location, writeElementCount, values));
			}
			else if (uniform == UniformDataType::Float)
			{
				GL_CALL(glProgramUniform1fv(programId, location, writeElementCount,
					static_cast<const GLfloat*>(valuePtr)));
			}
			else if (uniform == UniformDataType::Int || uniform == UniformDataType::Sampler2D
				|| uniform == UniformDataType::CubeSampler || uniform == UniformDataType::Image2D)
			{
				GL_CALL(glProgramUniform1iv(programId, location, writeElementCount,
					static_cast<const GLint*>(valuePtr)));
			}
			else if (uniform == UniformDataType::Vector2)
			{
				GL_CALL(glProgramUniform2fv(programId, location, writeElementCount,
					static_cast<const GLfloat*>(valuePtr)));
			}
			else if (uniform == UniformDataType::Vector3)
			{
				GL_CALL(glProgramUniform3fv(programId, location, writeElementCount,
					static_cast<const GLfloat*>(valuePtr)));
			}
			else if (uniform == UniformDataType::Vector4)
			{
				GL_CALL(glProgramUniform4fv(programId, location, writeElementCount,
					static_cast<const GLfloat*>(valuePtr)));
			}
			else if (uniform == UniformDataType::Matrix4x4)
			{
				GL_CALL(glProgramUniformMatrix4fv(programId, location, writeElementCount,
					GL_FALSE, static_cast<const GLfloat*>(valuePtr)));
			}
			else
			{
				return "OpenGL: Array uniform type has no corresponding actions";
			}

			return "";
		}
		static bool TryGetShaderUniform(const Shader& shader, const UniformDataType uniform, const char* uniformName, void* outputPtr)
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

			if (uniform == UniformDataType::Bool)
			{
				GLint output = 0;
				GL_CALL(glGetUniformiv(programId, location, &output));
				*static_cast<bool*>(outputPtr) = (output > 0)? true : false;
			}
			else if (uniform == UniformDataType::Float || uniform == UniformDataType::Vector2 || uniform == UniformDataType::Vector3
				|| uniform == UniformDataType::Vector4 || uniform == UniformDataType::Matrix4x4)
			{
				GL_CALL(glGetUniformfv(programId, location, static_cast<float*>(outputPtr)));
			}
			else if (uniform == UniformDataType::Int || uniform == UniformDataType::Sampler2D || uniform == UniformDataType::CubeSampler
				|| uniform == UniformDataType::Image2D)
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
		static bool TryGetUniformBlockMembers(const Shader& shader, const char* uniformName, std::vector<UniformBlockMemberMemoryInfo>& members, size_t* fullSize)
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
					members[i] = UniformBlockMemberMemoryInfo(nameStr, offsets[i], sizes[i], arrayStrides[i], matrixStrides[i]);
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

		static void QueryProgram(const Shader& shader, ShaderProgramQuery query, int* queryResult)
		{
			GLenum openglQueryType = 0;
			if (query == ShaderProgramQuery::ComputeShaderWorkGroupSize)
			{
				openglQueryType = GL_COMPUTE_WORK_GROUP_SIZE;
			}
			glGetProgramiv(shader.GetId(), openglQueryType, queryResult);
		}

		Shader CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
		{
			return Shader(vertexShader, fragmentShader, ShaderPlatformCallbacks
				{
					CreateShaderProgram,
					BindActive,
					DispatchComputeShaderGroups,
					UnbindActive,
					TrySetShaderUniform,
					TrySetShaderUniformArray,
					TryGetShaderUniform,
					TryBindShaderUniformBlock,
					TryGetUniformBlockMembers,
					QueryProgram,
					DeleteProgram
				});
		}
	}
}
#endif