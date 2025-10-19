#include "Core/Rendering/Buffers.hpp"
#include "StaticGlobals.hpp"
#include "Utils/ToStringFunctions.hpp"
#include "Utils/HelperFunctions.hpp"

namespace Rendering
{
	RenderBuffer::RenderBuffer() : RenderBuffer(DEFAULT_RENDER_BUFFER_STORAGE, {}, {}) {}
	RenderBuffer::RenderBuffer(const TexelStorageType storage, const Vec2Int size, const RenderBufferPlatformCallbacks& callbacks)
		: m_callbacks(callbacks), m_id(INVALID_OBJ_ID), m_size(size), m_attachmentStorage(storage)
	{
		if (m_callbacks.m_AllocateFunc == nullptr)
			return;

		m_id = m_callbacks.m_AllocateFunc(storage, size);
	}
	RenderBuffer::RenderBuffer(RenderBuffer&& other) noexcept
		: m_callbacks(std::exchange(other.m_callbacks, {})), m_id(std::exchange(other.m_id, INVALID_OBJ_ID)),
		m_size(std::exchange(other.m_size, {})), m_attachmentStorage(other.m_attachmentStorage) {}

	RenderBuffer::~RenderBuffer()
	{
		if (m_id != INVALID_OBJ_ID)
		{
			m_callbacks.m_DeallocateFunc(m_id);
			m_id = INVALID_OBJ_ID;
		}
	}

	const RenderObjectId RenderBuffer::GetId() const { return m_id; }
	RenderBuffer& RenderBuffer::operator=(RenderBuffer&& other) noexcept
	{
		m_callbacks = std::exchange(other.m_callbacks, {});
		m_id = std::exchange(other.m_id, INVALID_OBJ_ID);
		m_size = std::exchange(other.m_size, {});
		m_attachmentStorage = other.m_attachmentStorage;
		return *this;
	}

	void FrameBufferOutputTarget::SetTextureTarget(const FrameBufferTextureTarget& target)
	{
		m_TargetType = FrameBufferOutputType::Texture;
		m_Targets = target;
		m_HasOutput = true;
	}
	void FrameBufferOutputTarget::SetTextureCubeTarget(const FrameBufferTextureCubeTarget& target)
	{
		m_TargetType = FrameBufferOutputType::TextureCube;
		m_Targets = target;
		m_HasOutput = true;
	}
	void FrameBufferOutputTarget::SetRenderBufferTarget(const FrameBufferRenderBufferTarget& target)
	{
		m_TargetType = FrameBufferOutputType::RenderBuffer;
		m_Targets = target;
		m_HasOutput = true;
	}
	void FrameBufferOutputTarget::RemoveTarget()
	{
		m_HasOutput = false;
	}

	FrameBuffer::FrameBuffer() : FrameBuffer(FrameBufferPlatformCallbacks{}) {}
	FrameBuffer::FrameBuffer(const FrameBufferPlatformCallbacks& callbacks)
		: m_callbacks(callbacks), m_id(INVALID_OBJ_ID), m_outputTargets({}), m_outputTargetSize(), m_isBoundActive(false)
	{
		if (m_callbacks.m_AllocateFunc == nullptr)
			return;

		m_id = m_callbacks.m_AllocateFunc();

		for (size_t i = 0; i < m_outputTargets.size(); i++)
		{
			m_outputTargets[i].m_Type = static_cast<FrameBufferAttachmentType>(i);
		}
	}
	FrameBuffer::FrameBuffer(FrameBuffer&& other) noexcept
		: m_callbacks(std::exchange(other.m_callbacks, {})),
		m_id(std::exchange(other.m_id, INVALID_OBJ_ID)),
		m_outputTargets(std::exchange(other.m_outputTargets, {})),
		m_outputTargetSize(std::exchange(other.m_outputTargetSize, {})),
		m_isBoundActive(std::exchange(other.m_isBoundActive, false)) {}

	FrameBuffer::~FrameBuffer()
	{
		Deallocate();
	}
	void FrameBuffer::Deallocate()
	{
		if (m_id != INVALID_OBJ_ID)
		{
			m_callbacks.m_DeallocateFunc(m_id);
			m_id = INVALID_OBJ_ID;
		}
	}

	void FrameBuffer::BindActive()
	{
		if (m_isBoundActive)
			return;

		const FrameBufferAttachmentTypeIntegralType startColorAttachmentValue = 
			(FrameBufferAttachmentTypeIntegralType)MIN_COLOR_ATTACHMENT;
		const FrameBufferAttachmentTypeIntegralType totalColorAttachments =
			(FrameBufferAttachmentTypeIntegralType)MAX_COLOR_ATTACHMENT - (FrameBufferAttachmentTypeIntegralType)MIN_COLOR_ATTACHMENT + 1;

		size_t* usedColorAttachmentsArr = static_cast<size_t*>(alloca(sizeof(size_t) * totalColorAttachments));
		size_t usedColorIndex = 0;
		for (FrameBufferAttachmentTypeIntegralType i = 0; i < totalColorAttachments; i++)
		{
			if (!m_outputTargets[i + startColorAttachmentValue].m_HasOutput)
				continue;

			usedColorAttachmentsArr[usedColorIndex] = i;
			usedColorIndex++;
		}

		m_callbacks.m_BindActiveFunc(m_id, usedColorAttachmentsArr, usedColorIndex);
		m_isBoundActive = true;
	}
	void FrameBuffer::UnbindActive()
	{
		if (!m_isBoundActive)
			return;

		m_callbacks.m_UnbindActiveFunc();
		m_isBoundActive = false;
	}
	bool FrameBuffer::IsBoundActive() const
	{
		return m_isBoundActive;
	}
	FrameBufferOutputTarget& FrameBuffer::GetTargetFromType(const FrameBufferAttachmentType type)
	{
		return m_outputTargets[static_cast<FrameBufferAttachmentTypeIntegralType>(type)];
	}
	void FrameBuffer::SetOutputTexture(const FrameBufferAttachmentType type, Texture* tex)
	{
		FrameBufferOutputTarget& target = GetTargetFromType(type);
		target.SetTextureTarget(FrameBufferTextureTarget{ tex });
		m_callbacks.m_SetOutputTargetFunc(target, m_id);
	}
	void FrameBuffer::SetOutputTextureCube(const FrameBufferAttachmentType type, TextureCube* cube, const TextureCubeFace face)
	{
		FrameBufferOutputTarget& target = GetTargetFromType(type);
		target.SetTextureCubeTarget(FrameBufferTextureCubeTarget{ cube, face });
		m_callbacks.m_SetOutputTargetFunc(target, m_id);
	}
	void FrameBuffer::SetOutputRenderBuffer(const FrameBufferAttachmentType type, RenderBuffer* buffer)
	{
		FrameBufferOutputTarget& target = GetTargetFromType(type);
		target.SetRenderBufferTarget(FrameBufferRenderBufferTarget{ buffer });
		m_callbacks.m_SetOutputTargetFunc(target, m_id);
	}
	void FrameBuffer::RemoveOutputAt(const size_t i)
	{
		FrameBufferOutputTarget& target = m_outputTargets[i];
		m_callbacks.m_RemoveOutputTargetFunc(target, m_id);
		target.RemoveTarget();
	}
	void FrameBuffer::RemoveOutput(const FrameBufferAttachmentType type)
	{
		RemoveOutputAt(static_cast<FrameBufferAttachmentTypeIntegralType>(type));
	}
	
	void FrameBuffer::RemoveAllOutputs()
	{
		for (size_t i = 0; i < m_outputTargets.size(); i++)
		{
			if (m_outputTargets[i].m_HasOutput)
				RemoveOutputAt(i);
		}
	}

	size_t FencedBufferSegment::GetNextOffset() const
	{
		return m_ByteOffset + m_ByteSize;
	}
	std::string FencedBufferSegment::ToString() const
	{
		return std::format("[FencedSeg Off:{} Size:{} Fence:{}]", 
			m_ByteOffset, m_ByteSize, m_Fence.ToString());
	}

	FrameBuffer& FrameBuffer::operator=(FrameBuffer&& other) noexcept
	{
		m_callbacks = std::exchange(other.m_callbacks, {});
		m_id = std::exchange(other.m_id, INVALID_OBJ_ID);
		m_outputTargets = std::exchange(other.m_outputTargets, {});
		m_outputTargetSize = std::exchange(other.m_outputTargetSize, {});
		m_isBoundActive = std::exchange(other.m_isBoundActive, false);
		return *this;
	}

	RingBufferAllocator::RingBufferAllocator(const size_t allocatedByteSize)
		: m_segments(), m_fixedByteSize(allocatedByteSize), m_head(0), m_tail(0), m_usedSize(0) {}

	bool RingBufferAllocator::TryRemoveFinishedHeadSegments()
	{
		bool removedAny = false;

		//Segments at front-> oldest and should be checked first for popping
		//NOTE: we check if either the fence has finished -> the status gets destroyed when object 
		//has destructor invoked when the segment is popped
		while (!m_segments.empty() && (!m_segments.front().m_Fence.HasInserted() || m_segments.front().m_Fence.IsSignaled()))
		{
			m_usedSize -= m_segments.front().m_ByteSize;
			//LogWarning(std::format("Removing segment:{}", m_segments.front().ToString()));
			//if (m_segments.front().m_Fence.GetStatus(0) == GpuFenceStatus::Inactive) LogError(std::format("Removed inactive fence"));

			m_head = m_segments.front().GetNextOffset() % m_fixedByteSize;
			m_segments.pop_front();
			removedAny = true;
		}

		if (m_segments.empty())
		{
			m_head = 0;
			m_tail = 0;
		}

		return removedAny;
	}
	void RingBufferAllocator::ReserveSegment(const size_t offset, const size_t size, FencedBufferSegment** outSeg)
	{
		FencedBufferSegment& seg = m_segments.emplace_back(offset, size, CreateGpuFence(false));
		if (outSeg!=nullptr) *outSeg = &seg;

		m_tail = seg.GetNextOffset() % m_fixedByteSize;
		m_usedSize += size;
	}

	bool RingBufferAllocator::TryReserveSegment(const size_t size, const bool doStall, FencedBufferSegment** outSeg)
	{
		if (size == 0)
		{
			LogError(std::format("Cannot reserve fenced ring buffer size of 0"));
			return false;
		}

		size_t allocOffset = 0;
		size_t freeSpace = GetUnusedByteSize();
		size_t contiguousFromTail= GetContiguousSizeFromTail();
		if (freeSpace >= size )
		{
			if (contiguousFromTail >= size)
			{
				//LogError(std::format("reserving segment when status:{}", ToString()));
				ReserveSegment(m_tail, size, outSeg);
				return true;
			}
			else if (m_head >= size)
			{
				//LogError(std::format("reserving segment when status:{}", ToString()));
				ReserveSegment(0, size, outSeg);
				return true;
			}
		}

		while (true)
		{
			TryRemoveFinishedHeadSegments();

			freeSpace = GetUnusedByteSize();
			if (freeSpace < size)
			{
				if (doStall && !m_segments.empty())
				{
					m_segments.front().m_Fence.WaitUntilSignal();
					continue;
				}
				return false;
			}

			// Compute contiguous space starting at m_tail
			// Case A: tail >= head -> used = [head..tail), free is [tail..end) + [0..head)
			// Contiguous from tail without wrap is [tail..end)
			// Case B: tail < head -> used = [head..end)+[0..tail), free (contiguous) is [tail..head)
			allocOffset = m_tail;
			contiguousFromTail = GetContiguousSizeFromTail();

			if (contiguousFromTail >= size)
			{
				ReserveSegment(allocOffset, size, outSeg);
				return true;
			}

			// Doesn’t fit at the end -> consider wrapping to 0.
			// After wrapping, the contiguous free region is [0 .. head)
			// This is safe because m_head marks the start of the oldest live region,
			// and TryRemoveFinishedHeadSegments only advances head after its fence is signaled.
			if (m_head >= size)
			{
				allocOffset = 0;
				ReserveSegment(allocOffset, size, outSeg);
				return true;
			}

			// We have enough total free bytes, but not enough contiguous bytes
			// at tail nor at start -> must wait for the oldest fence to move head.
			if (doStall && !m_segments.empty())
			{
				m_segments.front().m_Fence.WaitUntilSignal();
				continue;
			}
			LogWarning(std::format("Free space, but contiguous for:{} Buff: {}", size, ToString()));
			return false;
		}
	}
	size_t RingBufferAllocator::GetFixedAllocatedByteSize() const
	{
		return m_fixedByteSize;
	}
	size_t RingBufferAllocator::GetUnusedByteSize() const
	{
		return m_fixedByteSize - m_usedSize;
	}
	size_t RingBufferAllocator::GetUsedByteSize() const
	{
		return m_usedSize;
	}
	size_t RingBufferAllocator::GetContiguousSizeFromTail() const
	{
		return (m_tail >= m_head)? (m_fixedByteSize - m_tail) : (m_head - m_tail);
	}
	std::array<Vec2Int, 2> RingBufferAllocator::GetUnusedRanges() const
	{
		std::array<Vec2Int, 2> unusedRanges = {INVALID_RANGE, INVALID_RANGE};
		if (m_head==0 && m_tail == m_head)
			unusedRanges[0] = Vec2Int(0, m_fixedByteSize);
		else if (m_tail >= m_head)
		{
			if (m_head > 0) unusedRanges[0] = Vec2Int(0, m_head);
			if (m_tail < m_fixedByteSize) unusedRanges[1] = Vec2Int(m_tail, m_fixedByteSize - m_tail);
		}
		else
		{
			unusedRanges[0] = Vec2Int(m_tail, m_head- m_tail);
		}

		return unusedRanges;
	}
	const std::deque<FencedBufferSegment>& RingBufferAllocator::GetSegments() const
	{
		return m_segments;
	}
	std::string RingBufferAllocator::ToString() const
	{
		return std::format("[BufferFence segments({}):{} head:{} tail:{} allocatedSize:{} used:{}]", 
			m_segments.size(), Utils::ToStringIterable<std::deque<FencedBufferSegment>, FencedBufferSegment>(m_segments),
			m_head, m_tail, m_fixedByteSize, m_usedSize);
	}

	VertexBuffer::VertexBuffer() : VertexBuffer(nullptr, 0, 0, VertexAttributeAdvance::Vertex, {}) {}

	VertexBuffer::VertexBuffer(const void* vertexArray, const size_t& elementSize, const size_t& arraySize, const VertexAttributeAdvance advanceType,
		const VertexBufferPlatformCallbacks callbacks)
		: m_id(INVALID_OBJ_ID), m_callbacks(callbacks), m_fence(arraySize * elementSize),
		m_AdvanceType(advanceType), m_elementSize(elementSize), m_writePtr(nullptr)
	{
		if (arraySize == 0)
			return;

		std::tie(m_id, m_writePtr) = m_callbacks.m_AllocateFunc(vertexArray, arraySize * m_elementSize, !PRODUCTION_BUILD);
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
	void VertexBuffer::DefaultUninitValues()
	{
		std::array<Vec2Int, 2> unusedRanges = m_fence.GetUnusedRanges();
		for (const auto& range : unusedRanges)
		{
			if (range == RingBufferAllocator::INVALID_RANGE)
				continue;

			std::memset(m_writePtr + (size_t)(range.m_X), 0, range.m_Y);
		}
	}

	void VertexBuffer::WriteDataUnsafeBytes(const size_t offsetBytes, const void* vertexArray, const size_t& writeByteSize) 
	{
		if (offsetBytes + writeByteSize > GetAllocatedByteSize())
		{
			LogError(std::format("Attempted to write data to vertex buffer with element offset:{} + count:{} "
				"that is greater than reserved size:{}", offsetBytes, writeByteSize, GetVertexCapacity()));
			return;
		}

		if (m_writePtr != nullptr)
		{
			/*LogError(std::format("og addr:{} offset:{} newptr:{}", Utils::ToStringPointerAddress(m_writePtr), 
				offsetBytes, Utils::ToStringPointerAddress(m_writePtr + offsetBytes)));*/
			memcpy(m_writePtr + offsetBytes, vertexArray, writeByteSize);
			return;
		}
		m_callbacks.m_WriteFunc(m_id, offsetBytes, vertexArray, writeByteSize);
	}
	void VertexBuffer::WriteDataUnsafe(const size_t& elementOffset, const void* vertexArray, const size_t& elementCount)
	{
		WriteDataUnsafeBytes(elementOffset * m_elementSize, vertexArray, elementCount * m_elementSize);
	}
	bool VertexBuffer::TryWriteDataFenced(const void* vertexArray, const size_t& elementCount, FencedBufferSegment** outSeg)
	{
		FencedBufferSegment* segment = nullptr;
		if (!m_fence.TryReserveSegment(elementCount * m_elementSize, false, &segment))
			return false;

#ifdef GRAPHICS_VERBOSE_LOG
		LogWarning(std::format("Writing vertex buffer fenced at offset:{} size:{} alloc:{} fence:{}", 
			segment->m_ByteOffset, segment->m_ByteSize, m_fence.GetFixedAllocatedByteSize(), m_fence.ToString()));
#endif
		WriteDataUnsafeBytes(segment->m_ByteOffset, vertexArray, segment->m_ByteSize);
		
		if (outSeg != nullptr) *outSeg = segment;
		return true;
	}
	size_t VertexBuffer::GetAllocatedByteSize() const { return m_fence.GetFixedAllocatedByteSize(); }
	size_t VertexBuffer::GetVertexCapacity() const { return m_fence.GetFixedAllocatedByteSize() / m_elementSize; }

	size_t VertexBuffer::GetElementSize() const { return m_elementSize; }
	RenderObjectId VertexBuffer::GetId() const { return m_id; }
	bool VertexBuffer::HasPersistentReadWritePointer() const { return m_writePtr != nullptr; }

	VertexBuffer& VertexBuffer::operator=(VertexBuffer&& other) noexcept
	{
		m_callbacks = std::exchange(other.m_callbacks, {});
		m_id = std::exchange(other.m_id, INVALID_OBJ_ID);
		m_elementSize = std::exchange(other.m_elementSize, 0);
		m_fence = std::move(other.m_fence);
		m_AdvanceType = other.m_AdvanceType;
		m_writePtr = std::exchange(other.m_writePtr, nullptr);
		return *this;
	}

	std::string VertexBuffer::ToString() const
	{
		return std::format("[VertexBuff id:{} writePtr:{} elementSize:{} fence:{}]", 
			m_id, m_writePtr!=nullptr, m_elementSize, m_fence.ToString());
	}

	IndexBuffer::IndexBuffer() : IndexBuffer(nullptr, 0, {}) {}
	IndexBuffer::IndexBuffer(const IndexType* indexArray, const size_t arraySize, const IndexBufferPlatformCallbacks& callbacks)
		: m_id(INVALID_OBJ_ID), m_callbacks(callbacks), m_fence(arraySize * GetElementSize()), m_writePtr(nullptr)
	{
		if (arraySize == 0)
			return;

		std::tie(m_id, m_writePtr) = m_callbacks.m_AllocateFunc(indexArray, arraySize * sizeof(IndexType), !PRODUCTION_BUILD);
	}
	IndexBuffer::~IndexBuffer()
	{
		Deallocate();
	}
	void IndexBuffer::WriteDataUnsafeBytes(const size_t offsetBytes, const IndexType* indexArray, const size_t& writeByteSize)
	{
		if (offsetBytes + writeByteSize > GetAllocatedByteSize())
		{
			LogError(std::format("Attempted to write data to index buffer with element offset:{} + count:{} "
				"that is greater than reserved size:{}", offsetBytes, writeByteSize, GetIndexCapacity()));
			return;
		}

		if (m_writePtr != nullptr)
		{
			memcpy(m_writePtr + offsetBytes, indexArray, writeByteSize);
			return;
		}

		m_callbacks.m_WriteFunc(m_id, offsetBytes, indexArray, writeByteSize);
	}
	void IndexBuffer::WriteDataUnsafe(const size_t elementOffset, const IndexType* indexArray, const size_t elementCount)
	{
		WriteDataUnsafeBytes(elementOffset * sizeof(IndexType), indexArray, elementCount * sizeof(IndexType));
	}
	bool IndexBuffer::TryWriteDataFenced(const IndexType* indexArray, const size_t elementCount, FencedBufferSegment** outSeg)
	{
		FencedBufferSegment* segment = nullptr;
		if (!m_fence.TryReserveSegment(elementCount * GetElementSize(), false, &segment))
			return false;

#ifdef GRAPHICS_VERBOSE_LOG
		LogWarning(std::format("Writing index buffer fenced at offset:{} size:{} alloc:{} fence:{}", 
			segment->m_ByteOffset, segment->m_ByteSize, m_fence.GetFixedAllocatedByteSize(), m_fence.ToString()));
#endif
		WriteDataUnsafeBytes(segment->m_ByteOffset, indexArray, segment->m_ByteSize);

		if (outSeg != nullptr) *outSeg = segment;
		return true;
	}
	size_t IndexBuffer::GetAllocatedByteSize() const { return m_fence.GetFixedAllocatedByteSize(); }
	size_t IndexBuffer::GetIndexCapacity() const { return m_fence.GetFixedAllocatedByteSize() / GetElementSize(); }
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
		m_fence = std::move(other.m_fence);
		m_writePtr = std::exchange(other.m_writePtr, nullptr);
		return *this;
	}


	UniformBuffer::UniformBuffer() : UniformBuffer("", UniformBufferPlatformCallbacks{}) {}
	UniformBuffer::UniformBuffer(const char* blockName, const UniformBufferPlatformCallbacks callbacks)
		: m_platformCallbacks(callbacks), m_bindIndex(INVALID_BUFFER_BIND_INDEX), 
		m_id(INVALID_OBJ_ID), m_members(), m_blockName(blockName),m_allocatedByteSize(0)
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
	void UniformBuffer::AllocateFromShaderUniformBlock(const Shader& shader)
	{
		std::vector<UniformBlockMemberMemoryInfo> members = {};
		size_t fullSize = 0;
		shader.TryGetUniformBlockMembers(m_blockName.data(), members, &fullSize);

		m_members.reserve(members.size());
		for (const auto& member : members)
		{
			m_members.emplace(member.m_Name, member);
		}
		m_id= m_platformCallbacks.m_AllocateFunc(fullSize);
		m_allocatedByteSize = fullSize;

		if (m_bindIndex != INVALID_BUFFER_BIND_INDEX)
			LinkBufferToCurrentBindingPoint();
	}
	void UniformBuffer::SetBindingPoint(const UniformBufferBindIndex bindIndex)
	{
		m_bindIndex = bindIndex;
	}
	void UniformBuffer::LinkBufferToBindingPoint(const UniformBufferBindIndex bindIndex)
	{
		if (m_id == INVALID_OBJ_ID)
		{
			LogError(std::format("Attempted to link uniform buffer to binding point:{} "
				"when its id is invalid (probably not allocated/initialized yet)", bindIndex));
			return;
		}
		SetBindingPoint(bindIndex);
		m_platformCallbacks.m_BindFunc(m_id, m_bindIndex);
	}
	void UniformBuffer::LinkBufferToCurrentBindingPoint()
	{
		LinkBufferToBindingPoint(m_bindIndex);
	}
	bool UniformBuffer::HasValidBindingPoint() const { return m_bindIndex != INVALID_BUFFER_BIND_INDEX; }
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
	std::string_view UniformBuffer::GetName() const { return std::string_view(m_blockName); }
	size_t UniformBuffer::GetAllocatedByteSize() const { return m_allocatedByteSize; }

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
		return std::format("[UniformBuffer members:{}]", Utils::ToStringIterable<std::vector<UniformBlockMemberMemoryInfo>, UniformBlockMemberMemoryInfo>
			(Utils::GetValuesFromMap<std::string, UniformBlockMemberMemoryInfo>(m_members.cbegin(), m_members.cend())));
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
		buffer->SetBindingPoint(bindIndex);
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