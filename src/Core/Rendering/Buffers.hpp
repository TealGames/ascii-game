#pragma once
#include <cstdint>
#include "Core/Rendering/RenderObjectId.hpp"
#include "glm/vec3.hpp"

namespace Rendering
{
	//TODO: If the code for index and vertex buffers is similar in other rendering frameworks, condense down buffers into
	//one buffer class since most code is duplicated

	enum BufferType : std::uint8_t
	{
		//Buffer is created 
		Static	= 0,
		Dynamic	= 1,
	};

	struct Vertex
	{
		glm::vec3 m_Pos;
	};

	using VertexType = Vertex;
	struct VertexBufferPlatformCallbacks
	{
		RenderObjectId(*m_AllocateFunc) (const VertexType*, const size_t);
		void(*m_WriteFunc) (const RenderObjectId, const size_t, const VertexType*, const size_t);
		void(*m_DeallocateFunc)(const RenderObjectId);
	};

	class VertexBuffer
	{
	private:
		VertexBufferPlatformCallbacks m_callbacks;
		RenderObjectId m_id;
	public:

	private:
		void Deallocate();
	public:
		/// <summary>
		/// Will create a vertex buffer. 
		/// Note: size is in element count
		/// </summary>
		/// <param name="vertexArray"></param>
		/// <param name="size"></param>
		/// <param name="callbacks"></param>
		VertexBuffer(const VertexType* vertexArray, const size_t elementCount, 
			const VertexBufferPlatformCallbacks callbacks);
		~VertexBuffer();

		void WriteData(const size_t elementOffset, const VertexType* vertexArray, const size_t elementCount);

		RenderObjectId GetId() const;
	};


	using IndexType = std::uint32_t;
	struct IndexBufferPlatformCallbacks
	{
		RenderObjectId(*m_AllocateFunc) (const IndexType*, const size_t);
		void(*m_WriteFunc) (const RenderObjectId, const size_t, IndexType*, const size_t);
		void(*m_DeallocateFunc)(const RenderObjectId);
	};

	class IndexBuffer
	{
	private:
		IndexBufferPlatformCallbacks m_callbacks;
		RenderObjectId m_id;
	public:

	private:
		void Deallocate();
	public:
		/// <summary>
		/// Will create an index buffer. 
		/// Note: size is in element count
		/// </summary>
		/// <param name="indexArray"></param>
		/// <param name="size"></param>
		/// <param name="callbacks"></param>
		IndexBuffer(const IndexType* indexArray, const size_t elementCount,
			const IndexBufferPlatformCallbacks& callbacks);
		~IndexBuffer();

		void WriteData(const size_t elementOffset, const IndexType* indexArray, const size_t elementCount);

		RenderObjectId GetId() const;
	};
}