#pragma once
#include <cstdint>
#include "Core/Rendering/RenderObjectId.hpp"
#include "Utils/Data/WorldPosition.hpp"
#include "Utils/Data/Color.hpp"
#include "Utils/Data/Matrix.hpp"

namespace Rendering
{
	//TODO: If the code for index and vertex buffers is similar in other rendering frameworks, condense down buffers into
	//one buffer class since most code is duplicated

	enum class BufferType : std::uint8_t
	{
		Vertex		= 0,
		Index		= 1,
		Instance	= 2
	};

	enum class BufferRenderType : std::uint8_t
	{
		//Buffer is created 
		Static	= 0,
		Dynamic	= 1,
	};

	/// <summary>
	/// How the attribute should advance
	/// </summary>
	enum class VertexAttributeAdvance : std::uint8_t
	{
		/// <summary>
		/// The default setting. 
		/// Will advance every vertex to the corresponding attribute for the next vertex
		/// </summary>
		Vertex = 0,
		/// <summary>
		/// Used for instanced buffers.
		/// Will advance the attribute every instance
		/// </summary>
		Instance = 1
	};

	struct VertexBufferPlatformCallbacks
	{
		RenderObjectId(*m_AllocateFunc) (const void* buffer, const size_t totalByteSize);
		void(*m_WriteFunc) (const RenderObjectId id, const size_t byteOffset, const void* buffer, const size_t bufferByteSize);
		void(*m_DeallocateFunc)(const RenderObjectId id);
	};

	class VertexBuffer
	{
	private:
		VertexBufferPlatformCallbacks m_callbacks;
		RenderObjectId m_id;
		/// <summary>
		/// How many vertices were actually uploaded to the buffer
		/// Size in bytes is m_dataUsed * sizeof(VertexType)
		/// </summary>
		size_t m_dataUsed;
		size_t m_elementSize;

		size_t m_maxVertexCount;
	public:
		VertexAttributeAdvance m_AdvanceType;

	private:
		void Deallocate();
	public:
		VertexBuffer();
		VertexBuffer(const void* vertexArray, const size_t& elementSize, const size_t& arraySize, const VertexAttributeAdvance advanceType,
			const VertexBufferPlatformCallbacks callbacks);
		VertexBuffer(const VertexBuffer&) = delete;
		VertexBuffer(VertexBuffer&&) = delete;
		~VertexBuffer();

		void WriteData(const size_t& elementOffset, const void* vertexArray, const size_t& elementCount);
		size_t GetUploadedSize() const;
		bool HasFilledMaxSize() const;

		size_t GetElementSize() const;
		RenderObjectId GetId() const;

		VertexBuffer& operator=(const VertexBuffer&) = delete;
		VertexBuffer& operator=(VertexBuffer&&) noexcept;
	};

	using IndexType = std::uint32_t;
	struct IndexBufferPlatformCallbacks
	{
		RenderObjectId(*m_AllocateFunc) (const IndexType* buffer, const size_t totalByteSize);
		void(*m_WriteFunc) (const RenderObjectId id, const size_t byteOffset, const IndexType* buffer, const size_t bufferByteSize);
		void(*m_DeallocateFunc)(const RenderObjectId id);
	};

	class IndexBuffer
	{
	private:
		IndexBufferPlatformCallbacks m_callbacks;
		RenderObjectId m_id;
		/// <summary>
		/// How many indices were actually uploaded to the buffer
		/// Size in bytes is m_dataUsed * sizeof(IndexType)
		/// </summary>
		size_t m_dataUsed;
		size_t m_maxElementCount;
	public:

	private:
		void Deallocate();
	public:
		IndexBuffer();
		IndexBuffer(const IndexType* indexArray, const size_t elementCount,
			const IndexBufferPlatformCallbacks& callbacks);
		IndexBuffer(const IndexBuffer& other) = delete;
		IndexBuffer(IndexBuffer&& other) = delete;

		~IndexBuffer();

		void WriteData(const size_t elementOffset, const IndexType* indexArray, const size_t elementCount);
		size_t GetUploadedSize() const;
		bool HasFilledMaxSize() const;
		
		inline constexpr size_t GetElementSize() const { return sizeof(IndexType); }
		RenderObjectId GetId() const;

		IndexBuffer& operator=(const IndexBuffer&) = delete;
		IndexBuffer& operator=(IndexBuffer&&) noexcept;
	};

	/// <summary>
	/// The type of data that the attribute holds.
	/// Note: a vec4 -> 4 floats
	/// Note: matrix4 -> need 4 attributes of 4 floats each
	/// </summary>
	enum class VertexAttributeBaseType : std::uint8_t
	{
		Float	= 0,
	};

	using BindIndex = std::uint8_t;
	using ShaderLocation = std::uint8_t;
	using ComponentCount = std::uint8_t;
	using ByteOffset = std::uint8_t;
	struct VertexAttribute
	{
		/// <summary>
		/// The location index within the shader program 
		/// layout(location =...) part
		/// </summary>
		ShaderLocation m_ShaderLocation = 0;
		/// <summary>
		/// The index corresponding to the buffer this attribute is bound to
		/// </summary>
		ComponentCount m_ComponentCount = 0;
		VertexAttributeBaseType m_Type = VertexAttributeBaseType::Float;
		/// <summary>
		/// Whether to normalize the value from [0, 1]
		/// </summary>
		bool m_Normalize = false;
		/// <summary>
		/// The offset in bytes of this attribute in the vertex data structure
		/// from the initial address
		/// </summary>
		ByteOffset m_ByteOffset = 0;

		BindIndex m_BindIndex = 0;
		VertexAttributeAdvance m_AdvanceType;
	};

	inline constexpr size_t IMPL_STATE_SIZE = 4;
	struct VertexLayoutCallbacks
	{
		void(*m_InitFunc)(std::array<std::byte, IMPL_STATE_SIZE>&);
		void(*m_AddAttributeFunc)(std::array<std::byte, IMPL_STATE_SIZE>&, const VertexAttribute&);
		void(*m_BindVertexBufferFunc)(std::array<std::byte, IMPL_STATE_SIZE>&, const RenderObjectId, const size_t, const BindIndex);
		void(*m_DeallocateFunc)(std::array<std::byte, IMPL_STATE_SIZE>&);
	};

	class VertexLayout
	{
	private:
		std::vector<VertexAttribute> m_layout;
		std::array<std::byte, IMPL_STATE_SIZE> m_implState;
		VertexLayoutCallbacks m_callbacks;
	public:

	private:
		void LinkToBuffer(const RenderObjectId id, const size_t elementSize, const BindIndex bindIndex);
		void Deallocate();
	public:
		VertexLayout();
		VertexLayout(const VertexLayoutCallbacks& callbacks);
		VertexLayout(const VertexLayout&) = delete;
		VertexLayout(VertexLayout&&) = delete;
		~VertexLayout();

		void AddAttribute(const VertexAttribute& attribute);
		//Note: index buffers are NOT linked to vertex layout explicitly
		void LinkToBuffer(const VertexBuffer& buffer, const BindIndex bindIndex);

		const VertexAttribute* GetAttributeByLocation(const std::uint8_t shaderLocation) const;
		const VertexAttribute* GetAttributeByBindIndex(const std::uint8_t bindIndex) const;

		VertexLayout& operator=(const VertexLayout&) = delete;
		VertexLayout& operator=(VertexLayout&& other) noexcept;
	};

	struct BufferData
	{
		/// <summary>
		/// The bind index that was used to associate the buffer with the 
		/// vertex layout. Note: INDEX BUFFERS DO NOT HAVE BIND INDICES
		/// SINCE THEY ARE IMPLICITLY BOUND AFTER CREATION (OPENGL)
		/// </summary>
		BindIndex m_VertexBufferBindIndex = 0;
		VertexBuffer* m_VertexBuffer = nullptr;
		IndexBuffer* m_IndexBuffer = nullptr;
	};

	class BufferController
	{
	private:
		VertexLayout* m_layout;
		std::vector<BufferData> m_bufferData;
		BindIndex m_currentBindIndex;
	public:

	private:
	public:
		BufferController(VertexLayout* vertexLayout);

		BindIndex AddVertexBuffer(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer);
		BufferData* GetBufferDataMutable(const BindIndex bindIndex);

		/// <summary>
		/// Will add all the attributes to the buffer.
		/// Note: the advance type is automatically set to match the same type as the buffer
		/// to prevent inconsistencies as well as the bindIndex
		/// </summary>
		/// <param name="bufferBindIndex"></param>
		/// <param name="attributes"></param>
		void AddVertexBufferAttributes(const BindIndex bufferBindIndex, std::vector<VertexAttribute>& attributes);

		/// <summary>
		/// Rather than creating the 4 separate attributes for every column, you can plug in some basic data and the 
		/// rest will be generated.
		/// Note: initial byte offset is the offset of the first float of the matrix from the vertex element.
		/// so you would do "offsetof(VERTEX_BUFFER_ELEMENT_TYPE, m_MATRIX_MEMBER_NAME)
		/// Note: columnTypesize is the sizeof(MATRIX_COLUMN_TYPE) -> should be a 4d vector type
		/// </summary>
		/// <param name="bufferBindIndex"></param>
		/// <param name="startLocation"></param>
		/// <param name="normalize"></param>
		/// <param name="initialByteOffset"></param>
		void AddVertexBufferMatrix4Attribute(const BindIndex bufferBindIndex, const ShaderLocation startLocation, 
			const bool normalize, const size_t matrixColumnTypeSize, const ByteOffset initialByteOffset);
	};
}