#pragma once
#include <string>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include "Core/Rendering/RenderObjectId.hpp"

namespace Rendering
{
	using ShaderTypeIntegralType = std::uint8_t;
	enum class ShaderType : ShaderTypeIntegralType
	{
		Vertex =	0,
		Fragment =	1,
	};
	std::string ToString(const ShaderType type);

	enum class UniformType : std::uint8_t
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

	struct UniformBlockMember
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

	struct UniformBlockData
	{
		UniformBufferBindIndex m_BufferBindIndex = INVALID_BUFFER_BIND_INDEX;
	};

	class Shader;
	struct ShaderPlatformCallbacks
	{
		RenderObjectId(*m_InitFunc) (const char* vertexSource, const char* fragmentSource, std::unordered_map<std::string, UniformBlockData>& blockData);
		void(*m_BindActiveFunc) (const Shader& shader);
		void(*m_UnbindActiveFunc) (const Shader& shader);
		bool(*m_TrySetUniformFunc) (const Shader& shader, const UniformType uniform, const char* uniformName, const void* valuePtr);
		bool(*m_TrySetArrayUniformFunc) (const Shader& shader, const UniformType uniform, const char* uniformName, const void* valuePtr, const size_t size);
		bool(*m_TryGetUniformFunc) (const Shader& shader, const UniformType uniform, const char* uniformName, void* outputPtr);
		bool(*m_TryBindUniformBlockFunc) (const Shader& shader, const char* uniformBlockName, const UniformBufferBindIndex index);
		bool(*TryGetUniformBlockMembers) (const Shader& shader, const char* uniformBlockName, std::vector<UniformBlockMember>& members, size_t* fullSize);
	};

	class Shader
	{
	private:
		ShaderPlatformCallbacks m_platformCallbacks;

		RenderObjectId m_id;
		std::string m_vertexSourceCode;
		std::string m_fragmentSourceCode;

		std::unordered_map<std::string, UniformBlockData> m_uniformBlockData;
		size_t m_boundUniformBlocksCount;
	public:

	private:
		void Init();
	public:
		Shader(const std::string& vertexSource, const std::string& fragmentSource, const ShaderPlatformCallbacks& callbacks);

		void SetSources(const std::string& vertexSource, const std::string& fragmentSource);

		RenderObjectId GetId() const;
		bool IsValid() const;
		bool HasUniformBlocks() const;
		bool HasAllUniformBlocksBounds() const;
		bool NeedsUniformBlockBound(const std::string& name) const;

		const std::string& GetVertexSource() const;
		const char* GetVertexSourceCStyle() const;

		const std::string& GetFragmnetSource() const;
		const char* GetFragmentSourceCStyle() const;

		void BindActive();
		void UnbindActive();

		bool TrySetUniform(const UniformType type, const char* uniformName, const void* valuePtr);
		bool TrySetUniformArray(const UniformType arrayType, const char* uniformName, const void* arrPtr, const size_t elementCount);
		bool TryGetUniform(const UniformType type, const char* uniformName, void* outputValue) const;

		bool TryBindUniformBlock(const char* blockName, const UniformBufferBindIndex index);
		bool BindUniformBlockIfNeeded(const std::string& name, const UniformBufferBindIndex index);
		bool TryGetUniformBlockMembers(const char* blockName, std::vector<UniformBlockMember>& members, size_t* fullSize) const;

		std::string ToString() const;
		//ShaderType GetType() const;
	};

	Shader CreateShader(const std::string& vertexSource, const std::string& fragmentSource);
}