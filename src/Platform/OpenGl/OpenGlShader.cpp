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
			LogWarning(std::format("Created shader:{}", shaderId));
			
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

			GLint result = GL_FALSE;
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

			LogWarning(std::format("Compiled source: {}", shaderSource.m_fullSource));
			return shaderId;
		}

		static RenderObjectId CreateShaderProgram(const FinalShaderInitData& firstInitData, const FinalShaderInitData* secondInitData,
			ShaderGlobalVarReflectionCollectionType* blockData)
		{
			RenderObjectId programId = INVALID_OBJ_ID;
			GL_CALL(programId = glCreateProgram());
			LogWarning(std::format("Created program: {}", programId));

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

			/*
			if (firstInitData.m_Type == ShaderType::Compute)
			{
				GLuint dummyTex;
				glGenTextures(1, &dummyTex);
				glBindTexture(GL_TEXTURE_2D, dummyTex);
				glBindImageTexture(0, dummyTex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
				GLint loc = glGetUniformLocation(programId, "uTextureOutput");
				glUniform1i(loc, 0);

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, dummyTex);
				loc = glGetUniformLocation(programId, "uTextureInput");
				glUniform1i(loc, 0);
				LogWarning("BOUND DUMMY");
			}
			*/
			
			//TODO: this right now causes problems for compute shader creation
			GL_CALL(glLinkProgram(programId));

			GLint linkStatus = GL_FALSE;
			GL_CALL(glGetProgramiv(programId, GL_LINK_STATUS, &linkStatus));
			if (linkStatus == GL_FALSE) 
			{
				char log[1024];
				glGetProgramInfoLog(programId, 1024, nullptr, log);
				LogError(std::format("[OPENGL]: Failed to link due to error: {}", std::string(log)));
				return INVALID_OBJ_ID;
			}

			//NOTE: on some drivers validate program on compute shaders may be buggy so we avoid it
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
			GLint storageBufferCount = 0;
			GL_CALL(glGetProgramiv(programId, GL_ACTIVE_UNIFORM_BLOCKS, &blockCount));
			GL_CALL(glGetProgramInterfaceiv(programId, GL_SHADER_STORAGE_BLOCK, GL_ACTIVE_RESOURCES, &storageBufferCount));
			GL_CALL(glGetProgramiv(programId, GL_ACTIVE_UNIFORMS, &uniformCount));

			blockData->reserve(blockCount + uniformCount + storageBufferCount);
			char nameBuffer[MAX_GLOBAL_VAR_NAME_SIZE];
			GLsizei nameLength = 0;

			const ShaderVarNameType** uniformBlockNames = nullptr;
			if (blockCount > 0)
			{
				uniformBlockNames= (const ShaderVarNameType**)alloca(sizeof(ShaderVarNameType*) * blockCount);
				for (GLint i = 0; i < blockCount; i++)
				{
					GL_CALL(glGetActiveUniformBlockName(programId, i, sizeof(nameBuffer), &nameLength, nameBuffer));
					uniformBlockNames[i] = &(blockData->emplace(FixedString<MAX_GLOBAL_VAR_NAME_SIZE>(nameBuffer, nameLength),
						ShaderGlobalVarReflectionInfo{ ShaderGlobalVarType::UniformBuffer }).first->first);
					//LogWarning(std::format("Created uniform buffer: {}", nameBuffer));
				}
			}
			if (uniformCount > 0)
			{
				GLint elementSize = 0;
				GLenum dataType = 0;
				for (GLint i = 0; i < uniformCount; i++)
				{
					GL_CALL(glGetActiveUniform(programId, i, sizeof(nameBuffer), &nameLength, &elementSize, &dataType, nameBuffer));
					//If a uniform contains the DOT it means it is EITHER a struct single uniform OR any uniform part of a uniform buffer
					//so we must separate and ensure that we are not including a buffer uniform in the var info
					size_t dotIndex = FindCharIndex(nameBuffer, nameLength, '.');
					if (dotIndex != std::string::npos && StringEqualsAny(std::string_view(nameBuffer, nameLength).substr(0, dotIndex), 
						uniformBlockNames, blockCount))
						continue;

					blockData->emplace(ShaderVarNameType(nameBuffer, nameLength),
						ShaderGlobalVarReflectionInfo{ elementSize > 1 ? ShaderGlobalVarType::UniformArray : ShaderGlobalVarType::UniformSingle });
					//LogWarning(std::format("Created uniform: {}", nameBuffer));
				}
			}
			if (storageBufferCount > 0)
			{
				for (GLint i = 0; i < storageBufferCount; i++)
				{
					GL_CALL(glGetProgramResourceName(programId, GL_SHADER_STORAGE_BLOCK, i, sizeof(nameBuffer), &nameLength, nameBuffer));
					blockData->emplace(ShaderVarNameType(nameBuffer, nameLength),
						ShaderGlobalVarReflectionInfo{ ShaderGlobalVarType::StorageBuffer });
					//LogWarning(std::format("Created storage buffer: {}", nameBuffer));
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
			else if (uniform == UniformDataType::Uint)
			{
				GL_CALL(glProgramUniform1ui(programId, location, *static_cast<const GLuint*>(valuePtr)));
			}
			else if (uniform == UniformDataType::IVector2)
			{
				const int* intArr = static_cast<const GLint*>(valuePtr);
				GL_CALL(glProgramUniform2i(programId, location, intArr[0], intArr[1]));
			}
			else if (uniform == UniformDataType::IVector3)
			{
				const int* intArr = static_cast<const GLint*>(valuePtr);
				GL_CALL(glProgramUniform3i(programId, location, intArr[0], intArr[1], intArr[2]));
			}
			else if (uniform == UniformDataType::IVector4)
			{
				const int* intArr = static_cast<const GLint*>(valuePtr);
				GL_CALL(glProgramUniform4i(programId, location, intArr[0], intArr[1], intArr[2], intArr[3]));
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
			else if (uniform == UniformDataType::Uint)
			{
				GL_CALL(glProgramUniform1uiv(programId, location, writeElementCount,
					static_cast<const GLuint*>(valuePtr)));
			}
			else if (uniform == UniformDataType::IVector2)
			{
				GL_CALL(glProgramUniform2iv(programId, location, writeElementCount,
					static_cast<const GLint*>(valuePtr)));
			}
			else if (uniform == UniformDataType::IVector3)
			{
				GL_CALL(glProgramUniform3iv(programId, location, writeElementCount,
					static_cast<const GLint*>(valuePtr)));
			}
			else if (uniform == UniformDataType::IVector4)
			{
				GL_CALL(glProgramUniform4iv(programId, location, writeElementCount,
					static_cast<const GLint*>(valuePtr)));
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
				|| uniform == UniformDataType::Image2D || uniform == UniformDataType::IVector2 || uniform == UniformDataType::IVector3
				|| uniform == UniformDataType::IVector4)
			{
				GL_CALL(glGetUniformiv(programId, location, static_cast<int*>(outputPtr)));
			}
			else if (uniform == UniformDataType::Uint)
			{
				GL_CALL(glGetUniformuiv(programId, location, static_cast<std::uint32_t*>(outputPtr)));
			}
			else
			{
				LogError("OpenGL: Uniform type has no corresponding actions");
				return false;
			}

			return true;
		}

		static bool TryBindShaderBufferBlock(const Shader& shader, const ShaderBufferType bufferType, 
			const char* blockName, const BufferBindIndex bindIndex)
		{
			const RenderObjectId programId = shader.GetId();
			if (glIsProgram(programId) == GL_FALSE)
			{
				LogError(std::format("OpenGL: Attempted to set shader:{} uniform block:{} but shader program with that id does not exist",
					shader.ToString(), blockName));
				return false;
			}

			GLuint blockIndex = GL_INVALID_INDEX;
			if (bufferType == ShaderBufferType::Uniform)
			{
				GL_CALL(blockIndex = glGetUniformBlockIndex(programId, blockName));
			}
			else if (bufferType == ShaderBufferType::Storage)
			{
				GL_CALL(blockIndex = glGetProgramResourceIndex(programId, GL_SHADER_STORAGE_BLOCK, blockName));
			}

			if (blockIndex == GL_INVALID_INDEX)
			{
				LogError(std::format("OpenGL: Invalid shader block location when setting shader:{} uniform block:{}. "
					"Possibly undefined uniform name or wrong spelling", shader.ToString(), blockName));
				return false;
			}

			if (bufferType == ShaderBufferType::Uniform)
			{
				GL_CALL(glUniformBlockBinding(programId, blockIndex, bindIndex));
			}
			else if (bufferType == ShaderBufferType::Storage)
			{
				GL_CALL(glShaderStorageBlockBinding(programId, blockIndex, bindIndex));
			}
			
			return true;
		}
		static bool TryGetUniformBlockMembers(const Shader& shader, const char* uniformName, 
			std::vector<ShaderBlockMemberMemoryInfo>& members, size_t* outFullByteSize)
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
					members[i] = ShaderBlockMemberMemoryInfo(nameStr, offsets[i], sizes[i], arrayStrides[i], matrixStrides[i]);
				else members.emplace_back(nameStr, offsets[i], sizes[i], arrayStrides[i]);
			}
			if (outFullByteSize != nullptr)
			{
				GLint blockSize = 0;
				GL_CALL(glGetActiveUniformBlockiv(shader.GetId(), blockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize));
				*outFullByteSize = blockSize;
			}
			return true;
		}

		static bool TryGetStorageBufferMembers(const Shader& shader, const char* bufferName, 
			std::vector<ShaderBlockMemberMemoryInfo>& members, size_t* outFullByteSize)
		{
			const RenderObjectId programId = shader.GetId();
			if (glIsProgram(programId) == GL_FALSE)
			{
				LogError(std::format("OpenGL: Attempted to get shader:{} storage buffer block members for:{} but shader program with that id does not exist",
					shader.ToString(), bufferName));
				return false;
			}

			GLuint blockIndex = GL_INVALID_INDEX;
			GL_CALL(blockIndex = glGetProgramResourceIndex(programId, GL_SHADER_STORAGE_BLOCK, bufferName));
			if (blockIndex == GL_INVALID_INDEX)
			{
				LogError(std::format("OpenGL: Invalid shader storage buffer block location when getting block members of shader:{} storage block:{}. "
					"Possibly undefined block name or wrong spelling. NOTE: USE THE BLOCK TYPE NOT THE INSTANCE NAME", shader.ToString(), bufferName));
				return false;
			}

			constexpr GLint BASIC_BUFFER_DATA_QUERY_COUNT = 2;
			GLenum resourceQuery[BASIC_BUFFER_DATA_QUERY_COUNT] = { GL_BUFFER_DATA_SIZE, GL_NUM_ACTIVE_VARIABLES };
			GLint* bufferInfoResult = (GLint*)alloca(sizeof(GLint) * BASIC_BUFFER_DATA_QUERY_COUNT);
			GL_CALL(glGetProgramResourceiv(programId, GL_SHADER_STORAGE_BLOCK, blockIndex, 
				BASIC_BUFFER_DATA_QUERY_COUNT, resourceQuery, BASIC_BUFFER_DATA_QUERY_COUNT, nullptr, bufferInfoResult));

			GLint variablesCount = bufferInfoResult[1];
			GLint* storageIndices = (GLint*)alloca(sizeof(GLint) * variablesCount);
			resourceQuery[0] = {GL_ACTIVE_VARIABLES};
			GL_CALL(glGetProgramResourceiv(programId, GL_SHADER_STORAGE_BLOCK, blockIndex, 1, resourceQuery, variablesCount,
				nullptr, storageIndices));

			for (int i = 0; i< variablesCount; i++)
			{
				GLuint variableIndex = storageIndices[i];

				char variableNameBuffer[128];
				GLsizei variableLength = 0;
				//LogWarning(std::format("Program id:{} buffer:{} var name:{} index:{}/varaible count:{}", programId, bufferName, variableNameBuffer, variableIndex, variablesCount));
				GL_CALL(glGetProgramResourceName(programId, GL_BUFFER_VARIABLE, variableIndex, 
					sizeof(variableNameBuffer), &variableLength, variableNameBuffer));

				GLenum propertyNames[] = { GL_BLOCK_INDEX, GL_TYPE, GL_OFFSET, GL_ARRAY_SIZE, GL_ARRAY_STRIDE, GL_MATRIX_STRIDE, GL_TOP_LEVEL_ARRAY_SIZE };
				GLint propertyValues[7];
				GL_CALL(glGetProgramResourceiv(programId, GL_BUFFER_VARIABLE, variableIndex, 7, propertyNames, 7, nullptr, propertyValues));

				//NOTE: unlike uniform buffers which begin with their block name, the storage buffer does not
				std::string variableName = std::string(variableNameBuffer, variableLength);
				//LogWarning(std::format("buffer:{} Variable: {} size:{}", bufferName, variableName, propertyValues[6]));

				//if (properties[3] == -1) LogError(std::format("Found dynamic array: {}", variableName));
				//Args are: name, offset, array size(1 if not array OR 0 and array with dynamic size), array stride(0 if not array), matrix stride
				if (i < members.size())
					members[i] = ShaderBlockMemberMemoryInfo(variableName, propertyValues[2], propertyValues[3], propertyValues[4], propertyValues[5]);
				else members.emplace_back(variableName, propertyValues[2], propertyValues[3], propertyValues[4], propertyValues[5]);
			}
			if (outFullByteSize != nullptr)
			{
				*outFullByteSize = bufferInfoResult[0];
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
					TryBindShaderBufferBlock,
					TryGetUniformBlockMembers,
					TryGetStorageBufferMembers,
					QueryProgram,
					DeleteProgram
				});
		}
	}
}
#endif