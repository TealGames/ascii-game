#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
#include "Utils/Data/FixedString.hpp"
#include "Core/Rendering/RenderObjectId.hpp"

namespace Rendering
{
	using ShaderTypeIntegralType = std::uint8_t;
	enum class ShaderType : ShaderTypeIntegralType
	{
		Vertex =	0,
		Fragment =	1,
		Compute	 =  2,
	};
	std::string ToString(const ShaderType type);

	enum class ShaderProgramType : ShaderTypeIntegralType
	{
		VertexFragment = 0,
		Compute		   = 1,
	};

	enum class UniformType : std::uint8_t
	{
		Single	=0,
		Array	=1,
		Buffer	=2,
	};
	std::string ToString(const UniformType type);

	enum class UniformDataType : std::uint8_t
	{
		Bool  = 0,
		Float = 1,
		Int = 2,
		Vector2 = 3,
		Vector3 = 4,
		Vector4 = 5,
		Matrix4x4 = 6,
		/// <summary>
		/// Represents a special value that holds the 
		/// texture slot index that the sampler uses for the
		/// texture. NOTE: sampler uniform must be get/set with int*
		/// </summary>
		Sampler2D	  = 7,
		/// <summary>
		/// Similar to sampler2D, but instead represents a cube map,
		/// 6 textures managed by one object
		/// </summary>
		CubeSampler	  = 8
	};
	struct UniformDataTypeInfo
	{
		bool m_IsArray;
		UniformDataType m_Type;
		//const void* m_ValuePtr;
	};

	struct UniformBlockMemberMemoryInfo
	{
		std::string m_Name;
		/// <summary>
		/// The offset for this block member from the block start.
		/// NOTE: primitive arrays (int[2]) would have one member data
		/// with offset of the first array element into the block.
		/// struct arrays (struct[2]) would have one data for EACH ELEMENT AND MEMBER
		/// so struct[0].member1, struct[0].member2, struct[1].member1, struct[1].member2 would 
		/// all be separate member data and would have offset= offsetof(struct)+offsetof(memberName)+index*arrayStride
		/// meaning offset is the offset for the exact member and index, not just into first element
		/// </summary>
		size_t m_ByteOffset;
		/// <summary>
		/// If not array type, this value is 1, otherwise the number of elements in the array
		/// NOTE: struct arrays are flattened and no longer considered as array types so for struct[2] 
		/// with struct[0].member1, struct[0].member2, struct[1].member1, etc...
		/// would all have array size 1 -> only basic type arrays like int[2] would have array size
		/// </summary>
		size_t m_ArraySize;
		/// <summary>
		/// The byte size of each element in the array (same for primitive and struct arrays). 
		/// None arrays have this value as 0
		/// NOTE: Primitive arrays (int[2]) and 
		/// members of struct arrays (struct[2]) would both have the their respective array strides
		/// (so struct[0].member1, struct[0].member2, struct[1].member1, struct[1].member2...
		/// would all have the same array stride since they are *technically* part of an array)
		/// </summary>
		size_t m_ArrayByteStride;
		size_t m_MatrixBytStride;

		std::string ToString() const;
	};

	struct UniformReflectionInfo
	{
		//UniformDataType m_DataType = UniformDataType::Float;
		UniformType m_Type = UniformType::Single;
		UniformBufferBindIndex m_BufferBindIndex = INVALID_BUFFER_BIND_INDEX;

		std::string ToString() const;
	};

	struct ShaderSourceDefines
	{
		const std::string_view* m_DefinesArr = nullptr;
		size_t m_DefinesSize = 0;
	};

	struct ShaderInitData
	{
		ShaderSourceDefines m_Defines = {};
		std::string m_Source = "";
	};
	struct TypedShaderInitData
	{
		ShaderType m_Type = ShaderType::Compute;
		ShaderInitData m_Data = {};
	};

	class Shader;
	using UniformReflectionCollectionType = std::unordered_map<String16, UniformReflectionInfo>;
	struct ShaderPlatformCallbacks
	{
		RenderObjectId(*m_CreateProgramFunc) (const TypedShaderInitData& initData1, const TypedShaderInitData* initData2, 
			UniformReflectionCollectionType* blockData);
		void(*m_BindActiveFunc) (const Shader& shader);
		void(*m_UnbindActiveFunc) (const Shader& shader);
		std::string(*m_TrySetUniformFunc) (const Shader& shader, const UniformDataType uniform, const char* uniformName, const void* valuePtr);
		std::string(*m_TrySetArrayUniformFunc) (const Shader& shader, const UniformDataType uniform, 
			const char* uniformName, const void* valuePtr, const size_t size);
		bool(*m_TryGetUniformFunc) (const Shader& shader, const UniformDataType uniform, const char* uniformName, void* outputPtr);
		bool(*m_TryBindUniformBlockFunc) (const Shader& shader, const char* uniformBlockName, const UniformBufferBindIndex index);
		bool(*TryGetUniformBlockMembers) (const Shader& shader, const char* uniformBlockName, 
			std::vector<UniformBlockMemberMemoryInfo>& members, size_t* fullSize);
		void(*m_DeleteProgramFunc) (const Shader& shader);
	};

	constexpr std::uint8_t SHADER_SOURCES = 2;
	class Shader
	{
	private:
		ShaderPlatformCallbacks m_platformCallbacks;

		RenderObjectId m_id;
		/// <summary>
		/// The source code where:
		/// Normal shader 0 -> vertex, 1 -> fragment
		/// Compute shader 0 -> full shader
		/// </summary>
		std::array<std::string, SHADER_SOURCES> m_sourceCode;
		std::optional<ShaderProgramType> m_maybeProgramType;

		UniformReflectionCollectionType m_uniformData;
		size_t m_unboundUniformBuffers;
	public:

	private:
		void DeleteProgram(const bool clearExistingData);
		bool PassesValidCheck() const;

		void CreateProgram(const TypedShaderInitData& initData1, const TypedShaderInitData* initData2 = nullptr);
		void CreateVertexFragmentProgram(const ShaderSourceDefines& vertexDefines = {}, const ShaderSourceDefines& fragmentDefines = {});
		void CreateComputeProgram(const ShaderSourceDefines& computeDefines = {});
	public:
		Shader(const std::string& verexSource, const std::string& fragmentSource, const ShaderPlatformCallbacks& callbacks);
		~Shader();
		Shader(const Shader&) = delete;
		Shader(Shader&&) noexcept;

		void SetSources(const std::optional<ShaderProgramType>& programType,  
			const std::string& source1, const std::string& source2= "");
		/// <summary>
		/// Will attempt to create the program if the program type has already been set with the sources
		/// </summary>
		/// <returns></returns>
		bool TryCreateProgram(const std::array<ShaderSourceDefines, SHADER_SOURCES>& defines = {});

		RenderObjectId GetId() const;
		bool IsValid() const;
		//bool HasUniformBlocks() const;
		//bool HasAllUniformBlocksBounds() const;
		//bool NeedsUniformBlockBound(const std::string& name) const;

		const std::string& GetSource1() const;
		const std::string& GetSource2() const;
		std::optional<ShaderProgramType> GetProgramType() const;

		void BindActive();
		void UnbindActive();

		bool HasUniform(const std::string_view& view) const;
		void SetUniform(const UniformDataType type, const char* uniformName, const void* valuePtr);
		bool TrySetUniform(const UniformDataType type, const char* uniformName, const void* valuePtr);
		void SetUniformArray(const UniformDataType arrayType, const char* uniformName, const void* arrPtr, const size_t elementCount);
		bool TrySetUniformArray(const UniformDataType arrayType, const char* uniformName, const void* arrPtr, const size_t elementCount);
		bool TryGetUniform(const UniformDataType type, const char* uniformName, void* outputValue) const;

		bool TryBindUniformBlock(const char* blockName, const UniformBufferBindIndex index);
		//bool BindUniformBlockIfNeeded(const std::string& name, const UniformBufferBindIndex index);
		bool TryGetUniformBlockMembers(const char* blockName, std::vector<UniformBlockMemberMemoryInfo>& members, size_t* fullSize) const;
		const UniformReflectionCollectionType& GetAllUniformInfo() const;

		std::string ToString() const;

		Shader& operator=(const Shader&) = delete;
		Shader& operator=(Shader&&) noexcept;
		//ShaderType GetType() const;
	};

	Shader CreateShader(const std::string& vertexSource, const std::string& fragmentSource);
}