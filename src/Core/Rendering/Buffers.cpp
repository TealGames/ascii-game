#include "Core/Rendering/Buffers.hpp"
#include "Core/Analyzation/Debug.hpp"

namespace Rendering
{
	/// <summary>
	/// Will create a vertex buffer. 
	/// </summary>
	/// <param name="vertexArray"></param>
	/// <param name="size"></param>
	/// <param name="callbacks"></param>
	VertexBuffer::VertexBuffer(const void* vertexArray, const size_t& elementSize, const size_t& arraySize, const VertexAttributeAdvance advanceType,
		const VertexBufferPlatformCallbacks callbacks)
		: m_id(INVALID_OBJ_ID), m_callbacks(callbacks), m_dataUsed(), m_MaxVertexCount(elementSize), 
		m_AdvanceType(advanceType), m_elementSize(elementSize)
	{
		m_id = m_callbacks.m_AllocateFunc(vertexArray, arraySize * m_elementSize);
		m_dataUsed = vertexArray == nullptr ? 0 : arraySize;
	}
	VertexBuffer::~VertexBuffer()
	{
		Deallocate();
	}
	void VertexBuffer::Deallocate()
	{
		m_callbacks.m_DeallocateFunc(m_id);
		m_id = INVALID_OBJ_ID;
	}

	void VertexBuffer::WriteData(const size_t& elementOffset, const void* vertexArray, const size_t& elementCount)
	{
		m_callbacks.m_WriteFunc(m_id, elementOffset * m_elementSize, vertexArray, elementCount * m_elementSize);
		//If we have already uploaded max data, then it means we are overriding existing data,
		//which would not change the total data used
		if (m_dataUsed < m_MaxVertexCount) m_dataUsed += elementCount;
	}
	size_t VertexBuffer::GetUploadedSize() const { return m_dataUsed; }
	bool VertexBuffer::HasFilledMaxSize() const { return m_dataUsed >= m_MaxVertexCount; }

	size_t VertexBuffer::GetElementSize() const { return m_elementSize; }
	RenderObjectId VertexBuffer::GetId() const { return m_id; }


	IndexBuffer::IndexBuffer(const IndexType* indexArray, const size_t arraySize, const IndexBufferPlatformCallbacks& callbacks)
		: m_id(INVALID_OBJ_ID), m_callbacks(callbacks), m_dataUsed(), m_MaxIndexCount(arraySize)
	{
		m_id= m_callbacks.m_AllocateFunc(indexArray, arraySize * sizeof(IndexType));
		m_dataUsed = indexArray == nullptr ? 0 : arraySize;
	}
	IndexBuffer::~IndexBuffer()
	{
		Deallocate();
	}
	void IndexBuffer::WriteData(const size_t elementOffset, const IndexType* indexArray, const size_t elementCount)
	{
		m_callbacks.m_WriteFunc(m_id, elementOffset * sizeof(IndexType), indexArray, elementCount * sizeof(IndexType));
		//If we have already uploaded max data, then it means we are overriding existing data,
		//which would not change the total data used
		if (m_dataUsed < m_MaxIndexCount) m_dataUsed += elementCount;
	}
	size_t IndexBuffer::GetUploadedSize() const
	{
		return m_dataUsed;
	}
	bool IndexBuffer::HasFilledMaxSize() const
	{
		return m_dataUsed >= m_MaxIndexCount;
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


	VertexLayout::VertexLayout(const VertexLayoutCallbacks& callbacks) : m_layout(), m_callbacks(callbacks), m_implState()
	{
		m_callbacks.m_InitFunc(m_implState);
	}

	void VertexLayout::AddAttribute(const VertexAttribute& attribute)
	{
		m_callbacks.m_AddAttributeFunc(m_implState, attribute);
	}
	void VertexLayout::LinkToBuffer(const RenderObjectId id, const size_t elementSize, const BindIndex bindIndex)
	{
		m_callbacks.m_BindVertexBufferFunc(m_implState, id, elementSize, bindIndex);
	}
	void VertexLayout::LinkToBuffer(const VertexBuffer& buffer, const BindIndex bindIndex)
	{
		LinkToBuffer(buffer.GetId(), buffer.GetElementSize(), bindIndex);
	}

	const VertexAttribute* VertexLayout::GetAttributeByLocation(const std::uint8_t shaderLocation) const
	{
		for (const auto& attribute : m_layout)
		{
			if (attribute.m_ShaderLocation == shaderLocation)
				return &attribute;
		}
		return nullptr;
	}
	const VertexAttribute* VertexLayout::GetAttributeByBindIndex(const std::uint8_t bindIndex) const
	{
		for (const auto& attribute : m_layout)
		{
			if (attribute.m_BindIndex == bindIndex)
				return &attribute;
		}
		return nullptr;
	}


	BufferController::BufferController(VertexLayout* vertexLayout) 
		: m_layout(vertexLayout), m_bufferData(), m_currentBindIndex(0) {}

	BindIndex BufferController::AddVertexBuffer(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer)
	{
		m_layout->LinkToBuffer(*vertexBuffer, m_currentBindIndex);
		m_bufferData.emplace_back(m_currentBindIndex, vertexBuffer, indexBuffer);

		if (m_currentBindIndex == std::numeric_limits<BindIndex>::max())
		{
			LogError(std::format("Reached the max limit of binding indices in buffer controller"));
			return 0;
		}

		return m_currentBindIndex++;
	}
	BufferData* BufferController::GetBufferDataMutable(const BindIndex bindIndex)
	{
		if (m_bufferData.empty()) 
			return nullptr;

		for (auto& data : m_bufferData)
		{
			if (data.m_VertexBufferBindIndex == bindIndex)
				return &data;
		}
		return nullptr;
	}

	void BufferController::AddVertexBufferAttributes(const BindIndex bufferBindIndex, std::vector<VertexAttribute>& attributes)
	{
		BufferData* bufferData = GetBufferDataMutable(bufferBindIndex);
		for (auto& attribute : attributes)
		{
			attribute.m_AdvanceType = bufferData->m_VertexBuffer->m_AdvanceType;
			attribute.m_BindIndex = bufferBindIndex;

			m_layout->AddAttribute(attribute);
		}
	}
	void BufferController::AddVertexBufferMatrix4Attribute(const BindIndex bufferBindIndex, const ShaderLocation startLocation,
		const bool normalize, const size_t matrixColumnTypeSize, const ByteOffset initialByteOffset)
	{
		BufferData* bufferData = GetBufferDataMutable(bufferBindIndex);
		const VertexAttributeAdvance advanceType = bufferData->m_VertexBuffer->m_AdvanceType;
		for (std::uint8_t i = 0; i < 4; i++)
		{
			m_layout->AddAttribute(VertexAttribute(startLocation + i, 4, VertexAttributeBaseType::Float, normalize,
				//For the offset, we assume it is tightly packed with no alignment
				initialByteOffset + matrixColumnTypeSize * i, bufferBindIndex, advanceType));
		}
	}
}