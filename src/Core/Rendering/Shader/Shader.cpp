#include "Core/Rendering/Shader/Shader.hpp"
#include "Core/Analyzation/Debug.hpp"
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

	std::string UniformBlockMember::ToString() const
	{
		return std::format("[Name:{} Offset:{} ArrSize:{} ArrStride:{} MatStride:{}]", 
			m_Name, m_ByteOffset, m_ArraySize, m_ArrayByteStride, m_MatrixBytStride);
	}

	Shader::Shader(const std::string& vertexSource, const std::string& fragmentSource, const ShaderPlatformCallbacks& callbacks)
		: m_platformCallbacks(callbacks), m_vertexSourceCode(vertexSource), m_fragmentSourceCode(fragmentSource), m_id(INVALID_OBJ_ID),
		m_boundUniformBlocksCount(0)
	{
		Init();
	}

	void Shader::SetSources(const std::string& vertexSource, const std::string& fragmentSource)
	{
		m_vertexSourceCode = vertexSource;
		m_fragmentSourceCode = fragmentSource;
		Init();
	}

	RenderObjectId Shader::GetId() const
	{
		return m_id;
	}

	bool Shader::IsValid() const
	{
		return m_id != INVALID_OBJ_ID;
	}
	bool Shader::HasUniformBlocks() const
	{
		return !m_uniformBlockData.empty();
	}
	bool Shader::HasAllUniformBlocksBounds() const
	{
		return m_boundUniformBlocksCount >= m_uniformBlockData.size();
	}
	bool Shader::NeedsUniformBlockBound(const std::string& name) const
	{
		if (HasAllUniformBlocksBounds())
			return false;

		auto it = m_uniformBlockData.find(name);
		if (it == m_uniformBlockData.end())
			return false;

		return it->second.m_BufferBindIndex == INVALID_BUFFER_BIND_INDEX;
	}

	const std::string& Shader::GetVertexSource() const
	{
		return m_vertexSourceCode;
	}
	const char* Shader::GetVertexSourceCStyle() const
	{
		return m_vertexSourceCode.c_str();
	}

	const std::string& Shader::GetFragmnetSource() const
	{
		return m_fragmentSourceCode;
	}
	const char* Shader::GetFragmentSourceCStyle() const
	{
		return m_fragmentSourceCode.c_str();
	}

	void Shader::Init()
	{
		if (m_vertexSourceCode.empty() || m_fragmentSourceCode.empty())
		{
			LogWarning(std::format("Exiting from shader initialization due to empty vertex "
				"and/or fragment source code. Vertex:{} \nFragment:{}", m_vertexSourceCode, m_fragmentSourceCode));
			return;
		}

		m_id= m_platformCallbacks.m_InitFunc(GetVertexSourceCStyle(), GetFragmentSourceCStyle(), m_uniformBlockData);
		if (m_id == INVALID_OBJ_ID)
		{
			LogError(std::format("Attempted to init shader with invalid render object id:{}", m_id));
		}
	}

	void Shader::BindActive()
	{
		if (!HasAllUniformBlocksBounds())
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

	bool Shader::TrySetUniform(const UniformType type, const char* uniformName, const void* valuePtr)
	{
		if (!m_platformCallbacks.m_TrySetUniformFunc(*this, type, uniformName, valuePtr))
		{
			LogError(std::format("Attempted to set uniform of name:{} in shader but resulted in error", uniformName));
			return false;
		}
		return true;
	}
	bool Shader::TrySetUniformArray(const UniformType arrayType, const char* uniformName, const void* arrPtr, const size_t elementCount)
	{
		if (!m_platformCallbacks.m_TrySetArrayUniformFunc(*this, arrayType, uniformName, arrPtr, elementCount))
		{
			LogError(std::format("Attempted to set uniform array of name:{} in shader but resulted in error", uniformName));
			return false;
		}
		return true;
	}
	bool Shader::TryGetUniform(const UniformType type, const char* uniformName, void* outputValue) const
	{
		if (!m_platformCallbacks.m_TryGetUniformFunc(*this, type, uniformName, outputValue))
		{
			LogError(std::format("Attempted to get uniform of name:{} in shader but resulted in error", uniformName));
			return false;
		}
		return true;
	}
	
	bool Shader::TryBindUniformBlock(const char* blockName, const UniformBufferBindIndex index)
	{
		if (m_uniformBlockData.empty())
		{
			LogError(std::format("Attempted to bind uniform block of named:{} but shader:{} has no uniform blocks", 
				blockName, ToString()));
			return false;
		}

		//Note: we do not check if valid name because that is essnetially done in bind callback
		if (!m_platformCallbacks.m_TryBindUniformBlockFunc(*this, blockName, index))
		{
			LogError(std::format("Attempted to bind uniform block of name:{} in shader but resulted in error", blockName));
			return false;
		}

		auto& blockData = m_uniformBlockData[std::string(blockName)];
		//If the previous index was invalid it means this is a new block that is bound
		if (blockData.m_BufferBindIndex == INVALID_BUFFER_BIND_INDEX)
			m_boundUniformBlocksCount++;
		else blockData.m_BufferBindIndex = index;

		return true;
	}
	bool Shader::BindUniformBlockIfNeeded(const std::string& name, const UniformBufferBindIndex index)
	{
		if (HasAllUniformBlocksBounds())
			return false;

		return TryBindUniformBlock(name.c_str(), index);
	}
	bool Shader::TryGetUniformBlockMembers(const char* blockName, std::vector<UniformBlockMember>& members, size_t* fullSize) const
	{
		if (!m_platformCallbacks.TryGetUniformBlockMembers(*this, blockName, members, fullSize))
		{
			LogError(std::format("Attempted to get uniform block members of name:{} in shader but resulted in error", blockName));
			return false;
		}
		return true;
	}

	std::string Shader::ToString() const
	{
		return std::format("[Shader Id:{} Vertex:\n{}\nFragment:{}]", m_id, m_vertexSourceCode, m_fragmentSourceCode);
	}

	Shader CreateShader(const std::string& vertexSource, const std::string& fragmentSource)
	{
#if defined(OPENGL)
		return OpenGl::CreateShader(vertexSource, fragmentSource);
#endif
	}
}