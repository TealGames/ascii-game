#include "Core/Rendering/Buffers.hpp"
#include "Core/Analyzation/Debug.hpp"
#include "Utils/ToStringFunctions.hpp"
#include "Utils/HelperFunctions.hpp"

namespace Rendering
{
	VertexBuffer::VertexBuffer() : m_id(INVALID_OBJ_ID), m_callbacks(), m_dataUsed(), m_maxVertexCount(),
		m_AdvanceType(VertexAttributeAdvance::Vertex), m_elementSize() {}

	VertexBuffer::VertexBuffer(const void* vertexArray, const size_t& elementSize, const size_t& arraySize, const VertexAttributeAdvance advanceType,
		const VertexBufferPlatformCallbacks callbacks)
		: m_id(INVALID_OBJ_ID), m_callbacks(callbacks), m_dataUsed(), m_maxVertexCount(arraySize),
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
		if (m_id == INVALID_OBJ_ID)
			return;

		m_callbacks.m_DeallocateFunc(m_id);
	}

	void VertexBuffer::WriteData(const size_t& elementOffset, const void* vertexArray, const size_t& elementCount)
	{
		if (elementCount + elementOffset > m_maxVertexCount)
		{
			LogError(std::format("Attempted to write data to vertex buffer with element offset + count:{} "
				"that is greater than reserved size:{}", elementOffset + elementCount, m_maxVertexCount));
			return;
		}

		m_callbacks.m_WriteFunc(m_id, elementOffset * m_elementSize, vertexArray, elementCount * m_elementSize);
		//If we have already uploaded max data, then it means we are overriding existing data,
		//which would not change the total data used
		if (m_dataUsed < m_maxVertexCount) m_dataUsed += elementCount;
	}
	size_t VertexBuffer::GetUploadedSize() const { return m_dataUsed; }
	bool VertexBuffer::HasFilledMaxSize() const { return m_dataUsed >= m_maxVertexCount; }

	size_t VertexBuffer::GetElementSize() const { return m_elementSize; }
	RenderObjectId VertexBuffer::GetId() const { return m_id; }

	VertexBuffer& VertexBuffer::operator=(VertexBuffer&& other) noexcept
	{
		m_callbacks = std::exchange(other.m_callbacks, {});
		m_id = std::exchange(other.m_id, INVALID_OBJ_ID);
		m_dataUsed = std::exchange(other.m_dataUsed, 0);
		m_elementSize = std::exchange(other.m_elementSize, 0);
		m_maxVertexCount = std::exchange(other.m_maxVertexCount, 0);
		m_AdvanceType = other.m_AdvanceType;
		return *this;
	}

	IndexBuffer::IndexBuffer() : m_id(INVALID_OBJ_ID), m_callbacks(), m_dataUsed(), m_maxElementCount() {}
	IndexBuffer::IndexBuffer(const IndexType* indexArray, const size_t arraySize, const IndexBufferPlatformCallbacks& callbacks)
		: m_id(INVALID_OBJ_ID), m_callbacks(callbacks), m_dataUsed(), m_maxElementCount(arraySize)
	{
		m_id = m_callbacks.m_AllocateFunc(indexArray, arraySize * sizeof(IndexType));
		m_dataUsed = indexArray == nullptr ? 0 : arraySize;
	}
	IndexBuffer::~IndexBuffer()
	{
		Deallocate();
	}
	void IndexBuffer::WriteData(const size_t elementOffset, const IndexType* indexArray, const size_t elementCount)
	{
		if (elementOffset + elementCount > m_maxElementCount)
		{
			LogError(std::format("Attempted to write data to index buffer with element offset + count:{} "
				"that is greater than reserved size:{}", elementOffset + elementCount, m_maxElementCount));
			return;
		}

		m_callbacks.m_WriteFunc(m_id, elementOffset * sizeof(IndexType), indexArray, elementCount * sizeof(IndexType));
		//If we have already uploaded max data, then it means we are overriding existing data,
		//which would not change the total data used
		if (m_dataUsed < m_maxElementCount) m_dataUsed += elementCount;
	}
	size_t IndexBuffer::GetUploadedSize() const
	{
		return m_dataUsed;
	}
	bool IndexBuffer::HasFilledMaxSize() const
	{
		return m_dataUsed >= m_maxElementCount;
	}
	void IndexBuffer::Deallocate()
	{
		if (m_id == INVALID_OBJ_ID)
			return;

		m_callbacks.m_DeallocateFunc(m_id);
	}
	RenderObjectId IndexBuffer::GetId() const
	{
		return m_id;
	}
	IndexBuffer& IndexBuffer::operator=(IndexBuffer&& other) noexcept
	{
		m_callbacks = std::exchange(other.m_callbacks, {});
		m_id = std::exchange(other.m_id, INVALID_OBJ_ID);
		m_dataUsed = std::exchange(other.m_dataUsed, 0);
		m_maxElementCount = std::exchange(other.m_maxElementCount, 0);
		return *this;
	}


	UniformBuffer::UniformBuffer() : UniformBuffer(UniformBufferPlatformCallbacks{}) {}
	UniformBuffer::UniformBuffer(const UniformBufferPlatformCallbacks callbacks)
		: m_platformCallbacks(callbacks), m_bindIndex(INVALID_BUFFER_BIND_INDEX), 
		m_id(INVALID_OBJ_ID), m_members(), m_blockName(),m_allocatedByteSize(0)
	{
		
	}
	UniformBuffer::~UniformBuffer()
	{
		if (m_id == INVALID_OBJ_ID)
			return;

		m_allocatedByteSize = 0;
		m_platformCallbacks.m_DeallocateFunc(m_id);
	}
	bool UniformBuffer::IsAllocated() const
	{
		return m_id != INVALID_OBJ_ID;
	}
	void UniformBuffer::AllocateFromShaderUniformBlock(const Shader& shader, const char* blockName)
	{
		std::vector<UniformBlockMember> members = {};
		size_t fullSize = 0;
		shader.TryGetUniformBlockMembers(blockName, members, &fullSize);

		m_members.reserve(members.size());
		for (const auto& member : members)
		{
			m_members.emplace(member.m_Name, member);
		}
		m_id= m_platformCallbacks.m_AllocateFunc(fullSize);
		m_allocatedByteSize = fullSize;
		m_blockName = std::string(blockName);
	}
	void UniformBuffer::LinkToUniformBindingPoint(const UniformBufferBindIndex bindIndex)
	{
		m_platformCallbacks.m_BindFunc(m_id, bindIndex);
		m_bindIndex = bindIndex;
	}
	void UniformBuffer::WriteData(const size_t byteOffset, const size_t writeByteSize, const void* data)
	{
		if (byteOffset + writeByteSize > m_allocatedByteSize)
		{
			LogError(std::format("Attempted to write data to uniform buffer named:{} with offset:{} size:{} "
				"which is past allocated size:{}", m_blockName, byteOffset, writeByteSize, m_allocatedByteSize));
			return;
		}

		m_platformCallbacks.m_WriteFunc(m_id, byteOffset, writeByteSize, data);
	}
	bool UniformBuffer::TryWriteData(const char* name, const size_t writeSize, const void* data)
	{
		auto it = m_members.find(name);
		if (it == m_members.end())
		{
			LogError(std::format("Attempted to write to uniform buffer for member:'{}' "
				"but it could not be found out of {} members. Members:{}", name, m_members.size(), ToString()));
			return false;
		}

		WriteData(it->second.m_ByteOffset, writeSize, data);
		return true;
	}

	bool UniformBuffer::TryWriteStruct(const std::string& structName, const size_t structSize, const void* data)
	{
		std::string tempStr = "";
		size_t firstMemberOffset = -1;
		for (const auto& member : m_members)
		{
			tempStr = member.first.substr(0, std::min(member.first.size(), structName.size()));
			if (tempStr != structName)
				continue;

			//since we do not have indexing or exact sizes, we can't know the first member offset
			//without going through all members
			firstMemberOffset = std::min(firstMemberOffset, member.second.m_ByteOffset);
		}
		if (firstMemberOffset == -1)
			return false;

		WriteData(firstMemberOffset, structSize, data);
		return true;
	}
	bool UniformBuffer::TryWriteStructMember(const std::string& structName, const std::string& memberName, const size_t memberSize, const void* data)
	{
		const std::string fullMemberName = std::string(structName) + '.' + std::string(memberName);
		auto it = m_members.find(fullMemberName);
		if (it == m_members.end())
			return false;

		WriteData(it->second.m_ByteOffset, memberSize, data);
		return true;
	}
	bool UniformBuffer::TryWritePrimitiveArray(const char* arrayName, const void* data)
	{
		auto it = m_members.find(arrayName);
		if (it == m_members.end())
			return false;

		WriteData(it->second.m_ByteOffset, it->second.m_ArraySize * it->second.m_ArrayByteStride, data);
		return true;
	}
	bool UniformBuffer::TryWritePrimitiveArrayElement(const char* arrayName, const size_t index, const void* data)
	{
		auto it = m_members.find(arrayName);
		if (it == m_members.end())
			return false;

		WriteData(it->second.m_ByteOffset + it->second.m_ArrayByteStride*index, 
			it->second.m_ArrayByteStride, data);
		return true;
	}
	bool UniformBuffer::TryWriteStructArray(const std::string& arrayName, const void* data)
	{
		size_t arraySize = 0;
		size_t firstElementOffset = -1;
		std::string tempStr = "";
		int convertedIndex = -1;
		for (const auto& member : m_members)
		{
			tempStr = member.first.substr(0, std::min(member.first.size(), arrayName.size()));
			if (tempStr != arrayName)
				continue;

			//Even thoguh str might match, it may have similar start, so we ensure
			//by finding the closing indexing segment of the member
			size_t closingIndexingIndex = member.first.find(']');
			if (closingIndexingIndex == std::string::npos)
				continue;

			tempStr = member.first.substr(tempStr.size() + 1, closingIndexingIndex - (tempStr.size() + 1));
			convertedIndex = std::atoi(tempStr.c_str());
			arraySize = std::max(arraySize, (convertedIndex+1)*member.second.m_ArrayByteStride);
			//We do not need first index to get struct size, we just use its offset and remove how many bytes it is in of the array
			if (firstElementOffset==size_t(-1)) 
				firstElementOffset = member.second.m_ByteOffset - (convertedIndex *member.second.m_ArrayByteStride);
		}
		if (arraySize == 0)
			return false;

		WriteData(firstElementOffset, arraySize, data);
		return true;
	}
	bool UniformBuffer::TryWriteStructArrayElement(const std::string& arrayName, const size_t index, const void* data)
	{
		size_t structSize = 0;
		size_t firstElementOffset = 0;
		std::string tempStr = "";
		for (const auto& member : m_members)
		{
			tempStr = member.first.substr(0, std::min(member.first.size(), arrayName.size()));
			if (tempStr != arrayName)
				continue;

			//Even thoguh str might match, it may have similar start, so we ensure
			//by finding the closing indexing segment of the member
			size_t closingIndexingIndex = member.first.find(']');
			if (closingIndexingIndex == std::string::npos)
				continue;

			structSize = member.second.m_ArrayByteStride;
			tempStr = member.first.substr(tempStr.size() + 1, closingIndexingIndex- (tempStr.size()+1));
			//We do not need first index to get struct size, we just use its offset and remove how many bytes it is in of the array
			firstElementOffset = member.second.m_ByteOffset - (std::atoi(tempStr.c_str()) * member.second.m_ArrayByteStride);
			break;
		}
		if (structSize == 0)
			return false;

		WriteData(firstElementOffset, structSize, data);
		return true;
	}
	bool UniformBuffer::TryWriteStructArrayElementMember(const std::string& arrayName, const size_t index,
		const std::string& memberName, const size_t size, const void* data)
	{
		//Note: struct elements are flattened into separate uniforms in the block but are accessed via index and member name
		const std::string fullMemberName = arrayName + '[' + std::to_string(index) + "]." + memberName;
		auto it = m_members.find(fullMemberName);
		if (it == m_members.end())
			return false;

		WriteData(it->second.m_ByteOffset, size, data);
		return true;
	}
	
	UniformBufferBindIndex UniformBuffer::GetBindIndex() const { return m_bindIndex; }
	RenderObjectId UniformBuffer::GetId() const { return m_id; }
	std::string UniformBuffer::GetName() const { return m_blockName; }
	size_t UniformBuffer::GetSize() const { return m_allocatedByteSize; }

	UniformBuffer& UniformBuffer::operator=(UniformBuffer&& other) noexcept
	{
		m_platformCallbacks = std::exchange(other.m_platformCallbacks, {});
		m_bindIndex= std::exchange(other.m_bindIndex, INVALID_BUFFER_BIND_INDEX);
		m_id= std::exchange(other.m_id, INVALID_OBJ_ID);
		m_members = std::exchange(other.m_members, {});
		m_blockName = std::exchange(other.m_blockName, {});
		m_allocatedByteSize = std::exchange(other.m_allocatedByteSize, {});
		return *this;
	}
	std::string UniformBuffer::ToString() const
	{
		return std::format("[UniformBuffer members:{}]", Utils::ToStringIterable<std::vector<UniformBlockMember>, UniformBlockMember>
			(Utils::GetValuesFromMap<std::string, UniformBlockMember>(m_members.cbegin(), m_members.cend())));
	}


	VertexLayout::VertexLayout(const VertexLayoutCallbacks& callbacks) : m_layout(), m_callbacks(callbacks), m_implState()
	{
		m_callbacks.m_InitFunc(m_implState);
	}
	VertexLayout::VertexLayout() : m_layout(), m_callbacks(), m_implState() {}
	VertexLayout::~VertexLayout()
	{
		Deallocate();
	}

	void VertexLayout::Deallocate()
	{
		if (m_layout.empty())
			return;

		m_callbacks.m_DeallocateFunc(m_implState);
	}

	void VertexLayout::AddAttribute(const VertexAttribute& attribute)
	{
		m_layout.push_back(attribute);
		m_callbacks.m_AddAttributeFunc(m_implState, m_layout.back());
	}
	void VertexLayout::LinkToBuffer(const RenderObjectId id, const size_t elementSize, 
		const VertexAttributeAdvance advance, const VertexLayoutBindIndex bindIndex)
	{
		m_callbacks.m_BindVertexBufferFunc(m_implState, id, elementSize, bindIndex, advance);
	}
	void VertexLayout::LinkToBuffer(const VertexBuffer& buffer, const VertexLayoutBindIndex bindIndex)
	{
		LinkToBuffer(buffer.GetId(), buffer.GetElementSize(), buffer.m_AdvanceType, bindIndex);
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
			if (attribute.m_BufferBindIndex == bindIndex)
				return &attribute;
		}
		return nullptr;
	}
	VertexLayout& VertexLayout::operator=(VertexLayout&& other) noexcept
	{
		m_layout = std::exchange(other.m_layout, {});
		m_implState = std::exchange(other.m_implState, {});
		m_callbacks = std::exchange(other.m_callbacks, {});
		return *this;
	}


	BufferController::BufferController(VertexLayout* vertexLayout) 
		: m_layout(vertexLayout), m_bufferData(), m_uniformBufferData() {}

	VertexLayoutBindIndex BufferController::AddVertexBuffer(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer)
	{
		if (!m_bufferData.empty() && m_bufferData.back().m_VertexBufferBindIndex == std::numeric_limits<VertexLayoutBindIndex>::max())
		{
			LogError(std::format("Failed to add vertex buffer: reached the max limit of binding indices in buffer controller"));
			return 0;
		}
		const VertexLayoutBindIndex bindIndex = m_bufferData.empty() ? 0 : m_bufferData.back().m_VertexBufferBindIndex + 1;
		m_layout->LinkToBuffer(*vertexBuffer, bindIndex);
		m_bufferData.emplace_back(bindIndex, vertexBuffer, indexBuffer);

		return bindIndex;
	}
	UniformBufferBindIndex BufferController::AddUniformBuffer(UniformBuffer* buffer)
	{
		if (!m_uniformBufferData.empty() && m_uniformBufferData.back().m_BindIndex == std::numeric_limits<UniformBufferBindIndex>::max())
		{
			LogError(std::format("Failed to add uniform buffer: reached the max limit of binding indices in buffer controller"));
			return 0;
		}
		const UniformBufferBindIndex bindIndex = m_uniformBufferData.empty() ? 0 : m_uniformBufferData.back().m_BindIndex + 1;
		buffer->LinkToUniformBindingPoint(bindIndex);
		m_uniformBufferData.emplace_back(bindIndex, buffer);

		return bindIndex;
	}
	BufferProperties* BufferController::GetBufferDataMutable(const VertexLayoutBindIndex bindIndex)
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

	void BufferController::AddVertexBufferAttributes(const VertexLayoutBindIndex bufferBindIndex, std::vector<VertexAttribute>& attributes)
	{
		BufferProperties* bufferData = GetBufferDataMutable(bufferBindIndex);
		for (auto& attribute : attributes)
		{
			attribute.m_BufferBindIndex = bufferBindIndex;
			m_layout->AddAttribute(attribute);
		}
	}
	void BufferController::AddVertexBufferMatrixAttribute(const Vec2Int& matrixSize, const VertexLayoutBindIndex bufferBindIndex, const ShaderLocation startLocation,
		const bool normalize, const size_t matrixColumnTypeSize, const ByteOffset initialByteOffset)
	{
		for (std::uint8_t i = 0; i < matrixSize.m_X; i++)
		{
			m_layout->AddAttribute(VertexAttribute(startLocation + i, matrixSize.m_Y, VertexAttributeBaseType::Float, normalize,
				//For the offset, we assume it is tightly packed with no alignment
				initialByteOffset + matrixColumnTypeSize * i, bufferBindIndex));
		}
	}
}