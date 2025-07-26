#include "Core/Rendering/Buffers.hpp"

namespace Rendering
{
	VertexBuffer::VertexBuffer(const VertexType* vertexArray, const size_t arraySize, const VertexBufferPlatformCallbacks callbacks) 
		: m_id(INVALID_OBJ_ID), m_callbacks(callbacks)
	{
		m_id= m_callbacks.m_AllocateFunc(vertexArray, arraySize);
	}
	VertexBuffer::~VertexBuffer()
	{
		Deallocate();
	}
	void VertexBuffer::WriteData(const size_t elementOffset, const VertexType* vertexArray, const size_t elementCount)
	{
		m_callbacks.m_WriteFunc(m_id, elementOffset, vertexArray, elementCount);
	}
	void VertexBuffer::Deallocate()
	{
		m_callbacks.m_DeallocateFunc(m_id);
		m_id = INVALID_OBJ_ID;
	}
	RenderObjectId VertexBuffer::GetId() const
	{
		return m_id;
	}


	IndexBuffer::IndexBuffer(const IndexType* indexArray, const size_t arraySize, const IndexBufferPlatformCallbacks& callbacks)
		: m_id(INVALID_OBJ_ID), m_callbacks(callbacks)
	{
		m_id= m_callbacks.m_AllocateFunc(indexArray, arraySize);
	}
	IndexBuffer::~IndexBuffer()
	{
		Deallocate();
	}
	void IndexBuffer::WriteData(const size_t elementOffset, const IndexType* indexArray, const size_t elementCount)
	{
		m_callbacks.m_WriteFunc(m_id, elementOffset, indexArray, elementCount);
	}
	void IndexBuffer::Deallocate()
	{
		m_callbacks.m_DeallocateFunc(m_id);
		m_id = INVALID_OBJ_ID;
	}
	RenderObjectId IndexBuffer::GetId() const
	{
		return m_id;
	}
}