#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
#include "Utils/Data/FixedString.hpp"
#include "Core/Rendering/RenderObject.hpp"
#include "Utils/Math/Vec3Type.hpp"

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

	enum class ShaderBufferType : std::uint8_t
	{
		Uniform = 0,
		Storage	= 1
	};

	enum class ShaderGlobalVarType : std::uint8_t
	{
		UniformSingle	= 0,
		UniformArray	= 1,
		UniformBuffer	= 2,
		StorageBuffer	= 3
	};
	bool IsShaderGlobalVarBoundableBuffer(const ShaderGlobalVarType type);
	bool IsShaderGlobalVarUniform(const ShaderGlobalVarType type);
	std::string ToString(const ShaderGlobalVarType type);

	enum class UniformDataType : std::uint8_t
	{
		Bool  = 0,
		Float = 1,
		Int = 2,
		Vector2 = 3,
		Vector3 = 4,
		Vector4 = 5,

		Uint	 = 6,
		IVector2 = 7,
		IVector3 = 8,
		IVector4 = 9,

		Matrix4x4 = 10,
		/// <summary>
		/// Represents a special value that holds the 
		/// texture slot index that the sampler uses for the
		/// texture. Allows for READ ONLY ACESSS
		/// NOTE: sampler uniform must be get/set with int*
		/// </summary>
		Sampler2D	  = 11,
		/// <summary>
		/// Similar to sampler2D, but instead represents a cube map,
		/// 6 textures managed by one object
		/// </summary>
		CubeSampler	  = 12,
		/// <summary>
		/// Similar to sampler2D and holds integer slot value BUT
		/// FOR AN IMAGE SLOT INDEX (differnet from texture slot index)
		/// that allows for READ + WRITE ACCESS (but comes at cost of 
		/// only getting raw pixel data unlike sampling)
		/// </summary>
		Image2D		  = 13
	};

	struct ShaderBlockMemberMemoryInfo
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
		/// NOTE: dynamic arrays (only allowed in shader storage buffers) will have size of 0
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

		bool IsDynamicArray() const;
		bool IsFixedSizeArray() const;
		bool IsArray() const;
		bool IsMatrix() const;
	};

	struct ShaderGlobalVarReflectionInfo
	{
		//UniformDataType m_DataType = UniformDataType::Float;
		ShaderGlobalVarType m_Type = ShaderGlobalVarType::UniformSingle;
		BufferBindIndex m_BufferBindIndex = INVALID_BUFFER_BIND_INDEX;

		std::string ToString() const;
	};

	struct ShaderSourceDefines
	{
		const std::string_view* m_DefinesArr = nullptr;
		size_t m_DefinesSize = 0;
	};

	struct RawShaderInitData
	{
		ShaderSourceDefines m_Defines = {};
		std::string_view m_Source = "";
	};
	struct FinalShaderInitData
	{
		ShaderType m_Type = ShaderType::Compute;
		std::string_view m_fullSource;
	};

	enum class ShaderProgramQuery : std::uint8_t
	{
		ComputeShaderWorkGroupSize	= 0
	};

	class Shader;
	constexpr size_t MAX_GLOBAL_VAR_NAME_SIZE = 20;
	using ShaderVarNameType = FixedString<MAX_GLOBAL_VAR_NAME_SIZE>;

	using ShaderGlobalVarReflectionCollectionType = std::unordered_map<ShaderVarNameType, ShaderGlobalVarReflectionInfo>;
	struct ShaderPlatformCallbacks
	{
		RenderObjectId(*m_CreateProgramFunc) (const FinalShaderInitData& initData1, const FinalShaderInitData* initData2, 
			ShaderGlobalVarReflectionCollectionType* blockData);
		void(*m_BindActiveFunc) (const Shader& shader);
		void(*m_DispatchComputeShaderGroupsFunc)(const std::uint32_t x, const std::uint32_t y, const std::uint32_t z);
		void(*m_UnbindActiveFunc) (const Shader& shader);
		std::string(*m_TrySetUniformFunc) (const Shader& shader, const UniformDataType uniform, const char* uniformName, const void* valuePtr);
		std::string(*m_TrySetArrayUniformFunc) (const Shader& shader, const UniformDataType uniform, 
			const char* uniformName, const void* valuePtr, const size_t size);
		bool(*m_TryGetUniformFunc) (const Shader& shader, const UniformDataType uniform, const char* uniformName, void* outputPtr);
		bool(*m_TryBindBufferBlockFunc) (const Shader& shader, const ShaderBufferType bufferType, const char* blockName, const BufferBindIndex index);
		bool(*TryGetUniformBlockMembersFunc) (const Shader& shader, const char* uniformBlockName, 
			std::vector<ShaderBlockMemberMemoryInfo>& members, size_t* outFullSize);
		bool(*TryGetStorageBufferMembersFunc) (const Shader& shader, const char* uniformBlockName,
			std::vector<ShaderBlockMemberMemoryInfo>& members, size_t* outFullSize);
		void(*m_QueryProgramFunc)(const Shader& shader, ShaderProgramQuery query, int* queryResult);
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

		ShaderGlobalVarReflectionCollectionType m_globalVarData;
		size_t m_unboundBuffers;
	public:

	private:
		void DeleteProgram();
		bool PassesValidCheck() const;

		void ApplyDefinesToSource(const size_t sourceIndex, const ShaderSourceDefines& defines);
		void CreateProgram(const FinalShaderInitData& initData1, const FinalShaderInitData* initData2 = nullptr);
		bool TryCreateVertexFragmentProgram(const ShaderSourceDefines& vertexDefines = {}, const ShaderSourceDefines& fragmentDefines = {});
		bool TryCreateComputeProgram(const ShaderSourceDefines& computeDefines = {});
	public:
		Shader(const std::string& verexSource, const std::string& fragmentSource, const ShaderPlatformCallbacks& callbacks);
		~Shader();
		Shader(const Shader&) = delete;
		Shader(Shader&&) noexcept;

		void SetSources(const std::optional<ShaderProgramType>& programType,  
			const std::string& source1, const std::string& source2= "");
		/// <summary>
		/// Will attempt to create the program if the program type has already been set with the sources
		/// NOTE: this option lets you setup defines for each source separately
		/// </summary>
		/// <returns></returns>
		bool TryCreateProgram(const std::array<ShaderSourceDefines, SHADER_SOURCES>& sourceDefines);
		/// <summary>
		/// Will attempt to create the program if the program type has already been set with the sources
		/// NOTE: this option sets the same defines for each source
		/// </summary>
		/// <param name="globalDefines"></param>
		/// <returns></returns>
		bool TryCreateProgram(const ShaderSourceDefines& globalDefines = {});

		RenderObjectId GetId() const;
		bool IsValid() const;
		//bool HasUniformBlocks() const;
		//bool HasAllUniformBlocksBounds() const;
		//bool NeedsUniformBlockBound(const std::string& name) const;

		const std::string& GetSource1() const;
		const std::string& GetSource2() const;
		std::optional<ShaderProgramType> GetProgramType() const;

		Vec3Int GetComputeShaderWorkGroupSize() const;
		Vec3Int GetBestComputeShaderWorkGroups(const Vec3Int& targetWork) const;

		void BindActive();
		void DispatchComputeShaderGroups(const std::uint32_t groupsX, const std::uint32_t groupsY, const std::uint32_t groupsZ);
		void DispatchComputeShaderGroups(const Vec3Int& targetWork);
		void UnbindActive();

		bool HasUniform(const std::string_view& view) const;
		void SetUniform(const UniformDataType type, const char* uniformName, const void* valuePtr);
		bool TrySetUniform(const UniformDataType type, const char* uniformName, const void* valuePtr);
		bool TrySetUniformArray(const UniformDataType arrayType, const char* uniformName, const void* arrPtr, const size_t elementCount);
		bool TryGetUniform(const UniformDataType type, const char* uniformName, void* outputValue) const;

		bool TryBindBufferBlock(const ShaderBufferType type, const char* blockName, const BufferBindIndex index);
		//bool BindUniformBlockIfNeeded(const std::string& name, const UniformBufferBindIndex index);
		bool TryGetUniformBlockMembers(const char* blockName, std::vector<ShaderBlockMemberMemoryInfo>& members, size_t* outFullByteSize) const;
		bool TryGetStorageBufferMembers(const char* blockName, std::vector<ShaderBlockMemberMemoryInfo>& members, size_t* outFullByteSize) const;
		const ShaderGlobalVarReflectionCollectionType& GetAllGlobalVarInfo() const;

		std::string ToString() const;

		Shader& operator=(const Shader&) = delete;
		Shader& operator=(Shader&&) noexcept;
		//ShaderType GetType() const;
	};

	Shader CreateShader(const std::string& vertexSource, const std::string& fragmentSource);
}