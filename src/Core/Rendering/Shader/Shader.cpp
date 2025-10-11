#include "Core/Rendering/Shader/Shader.hpp"
#include "Utils/Debug.hpp"
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
		else if (type == ShaderType::Compute) return "Compute";

		LogError("Failed to convert shader type with no defined actions to string");
		return "";
	}
	std::string ToString(const UniformType type)
	{
		if (type == UniformType::Single) return "Single";
		else if (type == UniformType::Array) return "Array";
		else if (type == UniformType::Buffer) return "Buffer";

		LogError("Failed to convert uniform type with no defined actions to string");
		return "";
	}

	std::string UniformBlockMemberMemoryInfo::ToString() const
	{
		return std::format("[Name:{} Offset:{} ArrSize:{} ArrStride:{} MatStride:{}]", 
			m_Name, m_ByteOffset, m_ArraySize, m_ArrayByteStride, m_MatrixBytStride);
	}

	std::string UniformReflectionInfo::ToString() const
	{
		return std::format("[UniformInfo Type:{} BindIndex:{}]", 
			Rendering::ToString(m_Type), m_BufferBindIndex);
	}

	Shader::Shader(const std::string& vertexSource, const std::string& fragmentSource, const ShaderPlatformCallbacks& callbacks)
		: m_platformCallbacks(callbacks), m_sourceCode{ vertexSource, fragmentSource },
		m_id(INVALID_OBJ_ID), m_uniformData(), m_unboundUniformBuffers(0), m_maybeProgramType(std::nullopt)
	{
		//CreateProgram();
	}
	Shader::~Shader()
	{
		DeleteProgram(true);
	}
	Shader::Shader(Shader&& other) noexcept
		: m_platformCallbacks(std::exchange(other.m_platformCallbacks, {})), m_sourceCode{ std::exchange(other.m_sourceCode[0], ""),
		std::exchange(other.m_sourceCode[1], "") }, m_id(std::exchange(other.m_id, INVALID_OBJ_ID)),
		m_uniformData(std::exchange(other.m_uniformData, {})), m_unboundUniformBuffers(std::exchange(other.m_unboundUniformBuffers, 0)),
		m_maybeProgramType(std::exchange(other.m_maybeProgramType, std::nullopt))
	{

	}

	void Shader::DeleteProgram(const bool clearExistingData)
	{
		if (m_id != INVALID_OBJ_ID)
		{
			m_platformCallbacks.m_DeleteProgramFunc(*this);
			m_id = INVALID_OBJ_ID;

			if (clearExistingData)
			{
				m_unboundUniformBuffers = 0;
				m_sourceCode[0] = "";
				m_sourceCode[1] = "";
				m_maybeProgramType = std::nullopt;
				m_uniformData.clear();
			}
		}
	}

	RenderObjectId Shader::GetId() const
	{
		return m_id;
	}

	bool Shader::IsValid() const
	{
		return m_id != INVALID_OBJ_ID;
	}
	bool Shader::PassesValidCheck() const
	{
		if (m_id == INVALID_OBJ_ID)
		{
			LogError(std::format("Attempted to use shader:{}, "
				"but the shader has invalid id meaning it was not compiled yet", ToString()));
			return false;
		}
		return true;
	}
	/*bool Shader::HasUniformBlocks() const
	{
		return !m_uniformData.empty();
	}
	bool Shader::HasAllUniformBlocksBounds() const
	{
		return m_boundUniformBlocksCount >= m_uniformData.size();
	}
	bool Shader::NeedsUniformBlockBound(const std::string& name) const
	{
		if (HasAllUniformBlocksBounds())
			return false;

		auto it = m_uniformData.find(name);
		if (it == m_uniformData.end())
			return false;

		return it->second.m_BufferBindIndex == INVALID_BUFFER_BIND_INDEX;
	}*/

	const std::string& Shader::GetSource1() const { return m_sourceCode[0]; }
	const std::string& Shader::GetSource2() const { return m_sourceCode[1]; }
	std::optional<ShaderProgramType> Shader::GetProgramType() const { return m_maybeProgramType; }

	void Shader::SetSources(const std::optional<ShaderProgramType>& programType, 
		const std::string& source1, const std::string& source2)
	{
		m_sourceCode[0] = source1;
		m_sourceCode[1] = source2;
		m_maybeProgramType = programType;
	}
	void Shader::CreateProgram(const TypedShaderInitData& initData1, const TypedShaderInitData* initData2)
	{
		//NOTE: if we already have exisiting data, we delete old data
		//EXCEPT the uniform data since the buffer binds should NOT change
		//only the compiled program
		const bool hasExistingData = IsValid();
		if (hasExistingData) DeleteProgram(false);

		//NOTE: once again, if existing data, is it not cleared thus we provide not data reference
		m_id = m_platformCallbacks.m_CreateProgramFunc(initData1, initData2, hasExistingData ? nullptr : &m_uniformData);

		for (const auto& uniformData : m_uniformData)
		{
			if (uniformData.second.m_Type != UniformType::Buffer)
				continue;

			//NOTE: we make sure to increase unbound buffers if there is NONE existing data (since we have no previous bind indices so we need them)
			//AND if we do to ensure that if there were any previous unbound blocks in previous data, that remains (since by increasing one here
			//and then decreasing in bind we cancel out)
			m_unboundUniformBuffers++;

			//NOTE: if we had existing data -> buffers existed but shader was recompiled with new id
			//so we must rebind each uniform block with the newly compiled shader id (but uniform bind index stays the same
			//assumuing nothing changed with the buffers)
			if (hasExistingData)
			{
				if (!TryBindUniformBlock(uniformData.first.GetMemPointer(), uniformData.second.m_BufferBindIndex))
				{
					LogError(std::format("Attempted to create shader program:{} but failed to rebind uniform block:{}"
						"after creating new program using old program's bindind indices", ToString(), uniformData.second.m_BufferBindIndex));
					return;
				}
			}
		}

		if (m_id == INVALID_OBJ_ID)
		{
			LogError(std::format("Attempted to init shader with invalid render object id:{}", m_id));
		}
	}
	bool Shader::TryCreateProgram(const std::array<ShaderSourceDefines, SHADER_SOURCES>& defines)
	{
		if (m_maybeProgramType == std::nullopt)
			return false;

		if (m_maybeProgramType == ShaderProgramType::VertexFragment) 
			CreateVertexFragmentProgram(defines[0], defines[1]);
		else if (m_maybeProgramType == ShaderProgramType::Compute)
			CreateVertexFragmentProgram(defines[0]);

		return true;
	}

	void Shader::CreateVertexFragmentProgram(const ShaderSourceDefines& vertexDefines, const ShaderSourceDefines& fragmentDefines)
	{
		if (m_sourceCode[0].empty() || m_sourceCode[1].empty())
		{
			LogWarning(std::format("Exiting from shader initialization due to empty vertex "
				"and/or fragment source code. Vertex:{} \nFragment:{}", m_sourceCode[0], m_sourceCode[1]));
			return;
		}

		TypedShaderInitData fragmentInitData{ ShaderType::Fragment, ShaderInitData{fragmentDefines, m_sourceCode[1]} };
		CreateProgram(TypedShaderInitData{ ShaderType::Vertex, ShaderInitData{vertexDefines, m_sourceCode[0]}}, &fragmentInitData);
	}
	void Shader::CreateComputeProgram(const ShaderSourceDefines& computeDefines)
	{
		if (m_sourceCode[0].empty())
		{
			LogWarning(std::format("Exiting from shader initialization due to empty compute source code "
				"Compute Shader: ", m_sourceCode[0]));
			return;
		}
		CreateProgram(TypedShaderInitData{ ShaderType::Compute, ShaderInitData{computeDefines, m_sourceCode[0]} }, nullptr);
	}

	void Shader::BindActive()
	{
		if (!PassesValidCheck())
			return;

		if (m_unboundUniformBuffers != 0)
		{
			LogError(std::format("Attempted to bind shader:{} active, but that is not allowed "
				"until all uniform blocks have a buffer bound", ToString()));
			return;
		}
		m_platformCallbacks.m_BindActiveFunc(*this);
	}
	void Shader::UnbindActive()
	{
		m_platformCallbacks.m_UnbindActiveFunc(*this);
	}
	bool Shader::HasUniform(const std::string_view& view) const
	{
		return m_uniformData.find(String16(view)) != m_uniformData.end();
	}
	void Shader::SetUniform(const UniformDataType type, const char* uniformName, const void* valuePtr)
	{
		if (!PassesValidCheck())
			return;

		const std::string error= m_platformCallbacks.m_TrySetUniformFunc(*this, type, uniformName, valuePtr);
		if (!error.empty()) LogError(error);
	}
	bool Shader::TrySetUniform(const UniformDataType type, const char* uniformName, const void* valuePtr)
	{
		if (!PassesValidCheck())
			return false;

		return m_platformCallbacks.m_TrySetUniformFunc(*this, type, uniformName, valuePtr).empty();
	}
	void Shader::SetUniformArray(const UniformDataType arrayType, const char* uniformName, const void* arrPtr, const size_t elementCount)
	{
		if (!PassesValidCheck())
			return;

		const std::string error = m_platformCallbacks.m_TrySetArrayUniformFunc(*this, arrayType, uniformName, arrPtr, elementCount);
		if (!error.empty()) LogError(error);
	}
	bool Shader::TrySetUniformArray(const UniformDataType arrayType, const char* uniformName, const void* arrPtr, const size_t elementCount)
	{
		if (!PassesValidCheck())
			return false;

		return m_platformCallbacks.m_TrySetArrayUniformFunc(*this, arrayType, uniformName, arrPtr, elementCount).empty();
	}
	bool Shader::TryGetUniform(const UniformDataType type, const char* uniformName, void* outputValue) const
	{
		if (!PassesValidCheck())
			return false;

		if (!m_platformCallbacks.m_TryGetUniformFunc(*this, type, uniformName, outputValue))
		{
			LogError(std::format("Attempted to get uniform of name:{} in shader but resulted in error", uniformName));
			return false;
		}
		return true;
	}
	
	bool Shader::TryBindUniformBlock(const char* blockName, const UniformBufferBindIndex index)
	{
		if (!PassesValidCheck())
			return false;

		if (m_uniformData.empty())
		{
			LogError(std::format("Attempted to bind uniform block of named:{} but shader:{} has no uniform blocks", 
				blockName, ToString()));
			return false;
		}
		if (index == INVALID_BUFFER_BIND_INDEX)
		{
			LogError(std::format("Attempted to bind uniform block named:{} for shader:{} "
				"but bind index is invalid", blockName, ToString()));
			return false;
		}

		//Note: we do not check if valid name because that is essnetially done in bind callback
		if (!m_platformCallbacks.m_TryBindUniformBlockFunc(*this, blockName, index))
		{
			LogError(std::format("Attempted to bind uniform block of name:{} in shader but resulted in error", blockName));
			return false;
		}
		if (m_unboundUniformBuffers>0) m_unboundUniformBuffers--;
		m_uniformData[String16(blockName)].m_BufferBindIndex = index;
		////If the previous index was invalid it means this is a new block that is bound
		//if (blockData.m_BufferBindIndex == INVALID_BUFFER_BIND_INDEX)
		//	m_boundUniformBlocksCount++;
		//else blockData.m_BufferBindIndex = index;

		return true;
	}
	/*bool Shader::HasUniformBlock(const std::string& blockName) const
	{
		return m_uniformData.find(blockName) != m_uniformData.end();
	}
	bool Shader::BindUniformBlockIfNeeded(const std::string& name, const UniformBufferBindIndex index)
	{
		if (HasAllUniformBlocksBounds())
			return false;

		return TryBindUniformBlock(name.c_str(), index);
	}*/
	bool Shader::TryGetUniformBlockMembers(const char* blockName, std::vector<UniformBlockMemberMemoryInfo>& members, size_t* fullSize) const
	{
		if (!PassesValidCheck())
			return false;

		if (!m_platformCallbacks.TryGetUniformBlockMembers(*this, blockName, members, fullSize))
		{
			LogError(std::format("Attempted to get uniform block members of name:{} in shader but resulted in error", blockName));
			return false;
		}
		return true;
	}
	const UniformReflectionCollectionType& Shader::GetAllUniformInfo() const
	{
		return m_uniformData;
	}

	std::string Shader::ToString() const
	{
		return std::format("[Shader Id:{} Source1:\n{}\nSource2:{}\nUnboundUniforms:{}\nUniformData:{}]", 
			m_id, m_sourceCode[0], m_sourceCode[1], m_unboundUniformBuffers,
			Utils::ToStringIterable<String16, UniformReflectionInfo>(m_uniformData));
	}

	Shader& Shader::operator=(Shader&& other) noexcept
	{
		m_platformCallbacks = std::exchange(other.m_platformCallbacks, {});
		m_sourceCode = { std::exchange(other.m_sourceCode[0], ""), std::exchange(other.m_sourceCode[1], "")};
		m_id = std::exchange(other.m_id, INVALID_OBJ_ID);
		m_uniformData = std::exchange(other.m_uniformData, {}); 
		m_unboundUniformBuffers = std::exchange(other.m_unboundUniformBuffers, 0);
		m_maybeProgramType = std::exchange(other.m_maybeProgramType, std::nullopt);
		return *this;
	}

	Shader CreateShader(const std::string& vertexSource, const std::string& fragmentSource)
	{
#if defined(OPENGL)
		return OpenGl::CreateShader(vertexSource, fragmentSource);
#endif
	}
}