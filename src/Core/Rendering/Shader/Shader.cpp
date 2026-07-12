#include "Core/Rendering/Shader/Shader.hpp"
#include "Utils/Debug.hpp"
#include "StaticGlobals.hpp"

#if defined(OPENGL)
#include "Platform/OpenGl/OpenGlShader.hpp"
#endif

namespace Engine::Rendering
{
	static constexpr const std::string_view VERSION_PREFIX = "#version";
	static constexpr const std::string_view VERSION_HEADER = "#version 430 core\n";

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
	bool IsShaderGlobalVarBoundableBuffer(const ShaderGlobalVarType type)
	{
		return type == ShaderGlobalVarType::StorageBuffer || type == ShaderGlobalVarType::UniformBuffer;
	}
	bool IsShaderGlobalVarUniform(const ShaderGlobalVarType type)
	{
		return type == ShaderGlobalVarType::UniformSingle ||
			type == ShaderGlobalVarType::UniformArray || type == ShaderGlobalVarType::UniformBuffer;
	}
	std::string ToString(const ShaderGlobalVarType type)
	{
		if (type == ShaderGlobalVarType::UniformSingle) return "UniformSingle";
		else if (type == ShaderGlobalVarType::UniformArray) return "UniformArray";
		else if (type == ShaderGlobalVarType::UniformBuffer) return "UniformBuffer";
		else if (type == ShaderGlobalVarType::StorageBuffer) return "StorageBuffer";

		LogError("Failed to convert shader global var type with no defined actions to string");
		return "";
	}

	bool ShaderBlockMemberMemoryInfo::IsDynamicArray() const
	{
		return IsArray() && m_ArraySize == 0;
	}
	bool ShaderBlockMemberMemoryInfo::IsFixedSizeArray() const
	{
		return IsArray() && m_ArraySize > 0;
	}
	bool ShaderBlockMemberMemoryInfo::IsArray() const
	{
		return m_ArrayByteStride > 0;
	}
	bool ShaderBlockMemberMemoryInfo::IsMatrix() const
	{
		return m_MatrixBytStride > 0;
	}

	std::string ShaderBlockMemberMemoryInfo::ToString() const
	{
		return std::format("[Name:{} Offset:{} ArrSize:{} ArrStride:{} MatStride:{}]", 
			m_Name, m_ByteOffset, m_ArraySize, m_ArrayByteStride, m_MatrixBytStride);
	}

	std::string ShaderGlobalVarReflectionInfo::ToString() const
	{
		return std::format("[UniformInfo Type:{} BindIndex:{}]", 
			Rendering::ToString(m_Type), m_BufferBindIndex);
	}

	Shader::Shader(const std::string& vertexSource, const std::string& fragmentSource, const ShaderPlatformCallbacks& callbacks)
		: m_platformCallbacks(callbacks), m_sourceCode{ vertexSource, fragmentSource },
		m_id(INVALID_OBJ_ID), m_globalVarData({}), m_unboundBuffers(0), m_maybeProgramType(std::nullopt)
	{
		//CreateProgram();
	}
	Shader::~Shader()
	{
		if (IsValid()) DeleteProgram();
	}
	Shader::Shader(Shader&& other) noexcept
		: m_platformCallbacks(std::exchange(other.m_platformCallbacks, {})), m_sourceCode{ std::exchange(other.m_sourceCode[0], ""),
		std::exchange(other.m_sourceCode[1], "") }, m_id(std::exchange(other.m_id, INVALID_OBJ_ID)),
		m_globalVarData(std::exchange(other.m_globalVarData, {})), m_unboundBuffers(std::exchange(other.m_unboundBuffers, 0)),
		m_maybeProgramType(std::exchange(other.m_maybeProgramType, std::nullopt))
	{

	}

	void Shader::DeleteProgram()
	{
		m_platformCallbacks.m_DeleteProgramFunc(*this);
		m_id = INVALID_OBJ_ID;

		m_unboundBuffers = 0;
		m_sourceCode[0] = "";
		m_sourceCode[1] = "";
		m_maybeProgramType = std::nullopt;
		m_globalVarData.clear();
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

	Vec3Int Shader::GetComputeShaderWorkGroupSize() const
	{
		Vec3Int groups = {};
		m_platformCallbacks.m_QueryProgramFunc(*this, 
			ShaderProgramQuery::ComputeShaderWorkGroupSize, groups.GetMemPointerMutable());
		return groups;
	}
	Vec3Int Shader::GetBestComputeShaderWorkGroups(const Vec3Int& targetWork) const
	{
		if (targetWork.m_X == 0 || targetWork.m_Y == 0 || targetWork.m_Z == 0)
		{
			LogError(std::format("Attempted to compute best compute shader group count for shader:{} "
				"but target work for at least one axis is 0: {}. Use 1 for no work in that axis instead of 0", 
				ToString(), targetWork.ToString()));
			return Vec3Int::Zero();
		}

		const Vec3Int workGroupSize = GetComputeShaderWorkGroupSize();
		if (workGroupSize.m_X == 0 || workGroupSize.m_Y == 0 || workGroupSize.m_Z == 0)
		{
			LogError(std::format("Attempted to compute best compute shader group count for shader:{} "
				"but work group size for at least one axis is 0: {}", ToString(), workGroupSize.ToString()));
			return Vec3Int::Zero();
		}
		//Dispatch one thread per pixel where each group size is defined in shader 
		//NOTE: since we do division, we may lose some texels, so we increase it to ensure we account for remainder
		return Vec3Int((targetWork.m_X + workGroupSize.m_X - 1) / workGroupSize.m_X, 
					   (targetWork.m_Y + workGroupSize.m_Y - 1) / workGroupSize.m_Y,
					   (targetWork.m_Z + workGroupSize.m_Z - 1) / workGroupSize.m_Z);
	}

	void Shader::SetSources(const std::optional<ShaderProgramType>& programType, 
		const std::string& source1, const std::string& source2)
	{
		m_sourceCode[0] = source1;
		m_sourceCode[1] = source2;
		m_maybeProgramType = programType;
	}
	void Shader::ApplyDefinesToSource(const size_t sourceIndex, const ShaderSourceDefines& defines)
	{
		std::string shaderMacroDefines = "";
		for (size_t i = 0; i < defines.m_DefinesSize; i++)
		{
			//NOTE: we use append since the defines arr uses string view which CAN NOT be concatenated with string
			shaderMacroDefines.append("#define ");
			shaderMacroDefines.append(defines.m_DefinesArr[i]);
			shaderMacroDefines.append("\n");
		}

		std::string_view mainShaderSource = m_sourceCode[sourceIndex];
		const std::size_t firstLineEndIndex = mainShaderSource.find('\n');
		const std::string_view firstLine = mainShaderSource.substr(0, firstLineEndIndex);
		//If we have a version header we strip that no matter
		if (firstLineEndIndex != std::string::npos && firstLine.substr(0,
			std::min(firstLine.size(), VERSION_PREFIX.size())) == VERSION_PREFIX)
		{
			m_sourceCode[sourceIndex] = m_sourceCode[sourceIndex].substr(firstLineEndIndex + 1);
		}

		m_sourceCode[sourceIndex] = std::string(VERSION_HEADER) + shaderMacroDefines + m_sourceCode[sourceIndex];
	}
	void Shader::CreateProgram(const FinalShaderInitData& initData1, const FinalShaderInitData* initData2)
	{
		if (IsValid())
		{
			LogError("Needed to delete old program");
			DeleteProgram();
		}

		m_id = m_platformCallbacks.m_CreateProgramFunc(initData1, initData2, &m_globalVarData);
		for (const auto& uniformData : m_globalVarData)
		{
			if (!IsShaderGlobalVarBoundableBuffer(uniformData.second.m_Type))
				continue;

			//NOTE: we make sure to increase unbound buffers if there is NONE existing data (since we have no previous bind indices so we need them)
			//AND if we do to ensure that if there were any previous unbound blocks in previous data, that remains (since by increasing one here
			//and then decreasing in bind we cancel out)
			m_unboundBuffers++;

			//NOTE: if we had existing data -> buffers existed but shader was recompiled with new id
			//so we must rebind each uniform block with the newly compiled shader id (but uniform bind index stays the same
			//assumuing nothing changed with the buffers)
			//if (!TryBindUniformBlock(uniformData.first.GetMemPointer(), uniformData.second.m_BufferBindIndex))
			//{
			//	LogError(std::format("Attempted to create shader program:{} but failed to rebind uniform block:{}"
			//		"after creating new program using old program's bindind indices", ToString(), uniformData.second.m_BufferBindIndex));
			//	return;
			//}
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
			return TryCreateVertexFragmentProgram(defines[0], defines[1]);
		else if (m_maybeProgramType == ShaderProgramType::Compute)
			return TryCreateComputeProgram(defines[0]);

		return true;
	}
	bool Shader::TryCreateProgram(const ShaderSourceDefines& globalDefines)
	{
		if (m_maybeProgramType == std::nullopt)
			return false;

		if (m_maybeProgramType == ShaderProgramType::VertexFragment)
			return TryCreateVertexFragmentProgram(globalDefines, globalDefines);
		else if (m_maybeProgramType == ShaderProgramType::Compute)
			return TryCreateComputeProgram(globalDefines);

		return true;
	}

	bool Shader::TryCreateVertexFragmentProgram(const ShaderSourceDefines& vertexDefines, const ShaderSourceDefines& fragmentDefines)
	{
		if (m_sourceCode[0].empty() || m_sourceCode[1].empty())
		{
			LogWarning(std::format("Exiting from shader initialization due to empty vertex "
				"and/or fragment source code. Vertex:{} \nFragment:{}", m_sourceCode[0], m_sourceCode[1]));
			return false;
		}
		if (vertexDefines.m_DefinesSize != 0) ApplyDefinesToSource(0, vertexDefines);
		if (fragmentDefines.m_DefinesSize != 0) ApplyDefinesToSource(1, fragmentDefines);

		FinalShaderInitData fragmentInitData{ ShaderType::Fragment, m_sourceCode[1] };
		CreateProgram(FinalShaderInitData{ ShaderType::Vertex, m_sourceCode[0]}, &fragmentInitData);
		return true;
	}
	bool Shader::TryCreateComputeProgram(const ShaderSourceDefines& computeDefines)
	{
		if (m_sourceCode[0].empty())
		{
			LogWarning(std::format("Exiting from shader initialization due to empty compute source code "
				"Compute Shader: ", m_sourceCode[0]));
			return false;
		}
		if (computeDefines.m_DefinesSize != 0) ApplyDefinesToSource(0, computeDefines);
		CreateProgram(FinalShaderInitData{ ShaderType::Compute, m_sourceCode[0] }, nullptr);
		return true;
	}

	void Shader::BindActive()
	{
		if (!PassesValidCheck())
			return;

		if (m_unboundBuffers != 0)
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
	void Shader::DispatchComputeShaderGroups(const std::uint32_t groupsX, const std::uint32_t groupsY, const std::uint32_t groupsZ)
	{
		m_platformCallbacks.m_DispatchComputeShaderGroupsFunc(groupsX, groupsY, groupsZ);
	}
	void Shader::DispatchComputeShaderGroups(const Vec3Int& targetWork)
	{
		const Vec3Int evenlyDividedWork = GetBestComputeShaderWorkGroups(targetWork);
		return DispatchComputeShaderGroups(evenlyDividedWork.m_X, evenlyDividedWork.m_Y, evenlyDividedWork.m_Z);
	}
	bool Shader::HasUniform(const std::string_view& view) const
	{
		return m_globalVarData.find(ShaderVarNameType(view)) != m_globalVarData.end();
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

		const std::string error = m_platformCallbacks.m_TrySetUniformFunc(*this, type, uniformName, valuePtr);
		if (!error.empty()) LogError(error);
		return error.empty();
	}
	bool Shader::TrySetUniformArray(const UniformDataType arrayType, const char* uniformName, const void* arrPtr, const size_t elementCount)
	{
		if (!PassesValidCheck())
			return false;

		const std::string error = m_platformCallbacks.m_TrySetArrayUniformFunc(*this, arrayType, uniformName, arrPtr, elementCount);
		if (!error.empty()) LogError(error);
		return error.empty();
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
	
	bool Shader::TryBindBufferBlock(const ShaderBufferType type, const char* blockName, const BufferBindIndex index)
	{
		if (!PassesValidCheck())
			return false;

		if (m_globalVarData.empty())
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
		if (!m_platformCallbacks.m_TryBindBufferBlockFunc(*this, type, blockName, index))
		{
			LogError(std::format("Attempted to bind uniform block of name:{} in shader but resulted in error", blockName));
			return false;
		}
		if (m_unboundBuffers>0) m_unboundBuffers--;
		m_globalVarData[ShaderVarNameType(blockName)].m_BufferBindIndex = index;
		
		return true;
	}

	bool Shader::TryGetUniformBlockMembers(const char* blockName, 
		std::vector<ShaderBlockMemberMemoryInfo>& members, size_t* outFullByteSize) const
	{
		if (!PassesValidCheck())
			return false;

		if (!m_platformCallbacks.TryGetUniformBlockMembersFunc(*this, blockName, members, outFullByteSize))
		{
			LogError(std::format("Attempted to get uniform block members of name:{} in shader but resulted in error", blockName));
			return false;
		}
		return true;
	}
	bool Shader::TryGetStorageBufferMembers(const char* blockName, 
		std::vector<ShaderBlockMemberMemoryInfo>& members, size_t* outFullByteSize) const
	{
		if (!PassesValidCheck())
			return false;

		if (!m_platformCallbacks.TryGetStorageBufferMembersFunc(*this, blockName, members, outFullByteSize))
		{
			LogError(std::format("Attempted to get uniform block members of name:{} in shader but resulted in error", blockName));
			return false;
		}
		return true;
	}
	const ShaderGlobalVarReflectionCollectionType& Shader::GetAllGlobalVarInfo() const
	{
		return m_globalVarData;
	}

	std::string Shader::ToString() const
	{
		return std::format("[Shader Id:{} Source1:\n{}\nSource2:{}\nUnboundUniforms:{}\nUniformData:{}]", 
			m_id, m_sourceCode[0], m_sourceCode[1], m_unboundBuffers, ::Utils::ToStringIterable(m_globalVarData));
	}

	Shader& Shader::operator=(Shader&& other) noexcept
	{
		m_platformCallbacks = std::exchange(other.m_platformCallbacks, {});
		m_sourceCode = { std::exchange(other.m_sourceCode[0], ""), std::exchange(other.m_sourceCode[1], "")};
		m_id = std::exchange(other.m_id, INVALID_OBJ_ID);
		m_globalVarData = std::exchange(other.m_globalVarData, {}); 
		m_unboundBuffers = std::exchange(other.m_unboundBuffers, 0);
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