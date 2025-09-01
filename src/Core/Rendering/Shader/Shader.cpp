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
		: m_platformCallbacks(callbacks), m_vertexSourceCode(vertexSource), 
		m_fragmentSourceCode(fragmentSource), m_id(INVALID_OBJ_ID), m_uniformData(), m_unboundUniformBuffers(0)
	{
		//CreateProgram();
	}
	Shader::~Shader()
	{
		DeleteProgram(true);
	}
	Shader::Shader(Shader&& other) noexcept
		: m_platformCallbacks(std::exchange(other.m_platformCallbacks, {})), m_vertexSourceCode(std::exchange(other.m_vertexSourceCode, "")),
		m_fragmentSourceCode(std::exchange(other.m_fragmentSourceCode, "")), m_id(std::exchange(other.m_id, INVALID_OBJ_ID)), 
		m_uniformData(std::exchange(other.m_uniformData, {})), m_unboundUniformBuffers(std::exchange(other.m_unboundUniformBuffers, 0))
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
				m_vertexSourceCode = "";
				m_fragmentSourceCode = "";
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

	const std::string& Shader::GetVertexSource() const
	{
		return m_vertexSourceCode;
	}
	const char* Shader::GetVertexSourceCStyle() const
	{
		return m_vertexSourceCode.c_str();
	}

	const std::string& Shader::GetFragmentSource() const
	{
		return m_fragmentSourceCode;
	}
	const char* Shader::GetFragmentSourceCStyle() const
	{
		return m_fragmentSourceCode.c_str();
	}
	void Shader::SetSources(const std::string& vertexSource, const std::string& fragmentSource)
	{
		m_vertexSourceCode = vertexSource;
		m_fragmentSourceCode = fragmentSource;
	}
	void Shader::CreateProgram(const ShaderSourceDefines& vertexDefines, const ShaderSourceDefines& fragmentDefines)
	{
		if (m_vertexSourceCode.empty() || m_fragmentSourceCode.empty())
		{
			LogWarning(std::format("Exiting from shader initialization due to empty vertex "
				"and/or fragment source code. Vertex:{} \nFragment:{}", m_vertexSourceCode, m_fragmentSourceCode));
			return;
		}

		//NOTE: if we already have exisiting data, we delete old data
		//EXCEPT the uniform data since the buffer binds should NOT change
		//only the compiled program
		const bool hasExistingData = IsValid();
		if (hasExistingData) DeleteProgram(false);

		//NOTE: once again, if existing data, is it not cleared thus we provide not data reference
		m_id= m_platformCallbacks.m_CreateProgramFunc(ShaderSource(vertexDefines, m_vertexSourceCode), 
			ShaderSource(fragmentDefines, m_fragmentSourceCode), hasExistingData? nullptr : &m_uniformData);
		
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
		return std::format("[Shader Id:{} Vertex:\n{}\nFragment:{}\nUnboundUniforms:{}\nUniformData:{}]", 
			m_id, m_vertexSourceCode, m_fragmentSourceCode, m_unboundUniformBuffers,
			Utils::ToStringIterable<String16, UniformReflectionInfo>(m_uniformData));
	}

	Shader& Shader::operator=(Shader&& other) noexcept
	{
		m_platformCallbacks = std::exchange(other.m_platformCallbacks, {});
		m_vertexSourceCode = std::exchange(other.m_vertexSourceCode, "");
		m_fragmentSourceCode = std::exchange(other.m_fragmentSourceCode, "");
		m_id = std::exchange(other.m_id, INVALID_OBJ_ID);
		m_uniformData = std::exchange(other.m_uniformData, {}); 
		m_unboundUniformBuffers = std::exchange(other.m_unboundUniformBuffers, 0);
		return *this;
	}

	Shader CreateShader(const std::string& vertexSource, const std::string& fragmentSource)
	{
#if defined(OPENGL)
		return OpenGl::CreateShader(vertexSource, fragmentSource);
#endif
	}
}