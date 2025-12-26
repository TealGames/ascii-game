#pragma once
#include <cstdint>
#include <array>
#include <deque>
#include <vector>
#include <tuple>
#include <variant>
#include <unordered_map>
#include "Core/Rendering/Texture.hpp"
#include "Core/Rendering/Shader/Shader.hpp"
#include "Core/Rendering/Vertex.hpp"
#include "Utils/Data/Vec2Type.hpp"
#include "Core/Rendering/GpuFence.hpp"
#include "Utils/Debug.hpp"

namespace Rendering
{
	struct RenderBufferPlatformCallbacks
	{
		RenderObjectId(*m_AllocateFunc)(const TexelStorageType storage, const Vec2Int size);
		void(*m_DeallocateFunc)(const RenderObjectId id);
	};
	inline constexpr TexelStorageType DEFAULT_RENDER_BUFFER_STORAGE = TexelStorageType::RGBA8;
	class RenderBuffer
	{
	private:
		RenderBufferPlatformCallbacks m_callbacks;
		RenderObjectId m_id;
		TexelStorageType m_attachmentStorage;
		Vec2Int m_size;
	public:

	private:
	public:  
		RenderBuffer();
		RenderBuffer(const TexelStorageType storage, const Vec2Int size, const RenderBufferPlatformCallbacks& callbacks);
		RenderBuffer(const RenderBuffer&) = delete;
		RenderBuffer(RenderBuffer&&) noexcept;
		~RenderBuffer();

		const RenderObjectId GetId() const;
		
		RenderBuffer& operator=(const RenderBuffer&) = delete;
		RenderBuffer& operator=(RenderBuffer&&) noexcept;
	};

	/// <summary>
	/// The type of attachment to add to a framebuffer to determine what kind of data
	/// it needs to write to the render target. NOTE: render target format must coincide with the 
	/// attachment type
	/// </summary>
	using FrameBufferAttachmentTypeIntegralType = std::uint8_t;
	enum class FrameBufferAttachmentType : FrameBufferAttachmentTypeIntegralType
	{
		/// <summary>
		/// Color0- ColorMAX are the different color outputs from the fragment shader
		/// (in fragment shader specify different layout(location= i) for the
		/// location of the color, where i is number used in enum
		/// Color0 -> layout(location =0) *DEFAULT USED IN MAIN RENDER TARGET*
		/// Color1 -> layout(locaton = 1) ... etc
		/// Can also be used to output non-color data since this is just a vec4 data slot
		/// </summary>
		Color0			= 0,
		Color1			= 1,
		Color2			= 2,
		Color3			= 3,
		/// <summary>
		/// Per pixel/fragment, stores the distance from the camera/viewer
		/// </summary>
		Depth			= 4,
		/// <summary>
		/// Per pixel/fragment, stores a stencil value (usually 0-255)
		/// which can then be used for drawing later. 
		/// Is most often used for things like masking/culling to discard
		/// unwanted pixels based on a baseline stencil value
		/// </summary>
		Stencil			= 5,
		DepthAndStencil = 6,
	};
	inline constexpr FrameBufferAttachmentType MIN_COLOR_ATTACHMENT = FrameBufferAttachmentType::Color0;
	inline constexpr FrameBufferAttachmentType MAX_COLOR_ATTACHMENT = FrameBufferAttachmentType::Color3;
	inline constexpr std::uint8_t MAX_SUPPORTED_COLOR_ATTACHMENTS = 
		static_cast<FrameBufferAttachmentTypeIntegralType>(MAX_COLOR_ATTACHMENT) -
		static_cast<FrameBufferAttachmentTypeIntegralType>(MIN_COLOR_ATTACHMENT) + 1;

	inline constexpr FrameBufferAttachmentTypeIntegralType ATTACHMENT_TYPES_COUNT = 
		static_cast<FrameBufferAttachmentTypeIntegralType>(FrameBufferAttachmentType::DepthAndStencil) + 1;

	enum class FrameBufferOutputType : std::uint8_t
	{
		Texture		= 0,
		TextureCube	= 1,
		RenderBuffer =2
	};
	struct FrameBufferTextureTarget
	{
		Texture* m_Texture = nullptr;
	};
	struct FrameBufferTextureCubeTarget
	{
		TextureCube* m_CubeTexture = nullptr;
		TextureCubeFace m_Face = TextureCubeFace::Front;
	};
	struct FrameBufferRenderBufferTarget
	{
		RenderBuffer* m_RenderBuffer = nullptr;
	};
	using PossibleTargetTypes = std::variant<FrameBufferTextureTarget, FrameBufferTextureCubeTarget, FrameBufferRenderBufferTarget>;
	struct FrameBufferOutputTarget
	{
		FrameBufferAttachmentType m_Type = FrameBufferAttachmentType::Color0;
		FrameBufferOutputType m_TargetType = FrameBufferOutputType::Texture;
		PossibleTargetTypes m_Targets = {};
		bool m_HasOutput = false;

		void SetTextureTarget(const FrameBufferTextureTarget& target);
		void SetTextureCubeTarget(const FrameBufferTextureCubeTarget& target);
		void SetRenderBufferTarget(const FrameBufferRenderBufferTarget& target);
		void RemoveTarget();
	};
	struct FrameBufferPlatformCallbacks
	{
		RenderObjectId(*m_AllocateFunc)();
		void(*m_DeallocateFunc)(const RenderObjectId id);
		void(*m_BindActiveFunc)(const RenderObjectId id, const size_t* colorAttachmentsArr, const size_t colorAttachmentsSize);
		void(*m_UnbindActiveFunc)();
		void(*m_SetOutputTargetFunc)(const FrameBufferOutputTarget& target, const RenderObjectId id);
		void(*m_RemoveOutputTargetFunc)(const FrameBufferOutputTarget& target, const RenderObjectId id);
	};
	class FrameBuffer
	{
	private:
		FrameBufferPlatformCallbacks m_callbacks;
		RenderObjectId m_id;
		std::array<FrameBufferOutputTarget, ATTACHMENT_TYPES_COUNT> m_outputTargets;
		/// <summary>
		/// The size of the output targets. Note: 
		/// all targets must have the same texture size, and the first output target set,
		/// initializes the required size
		/// </summary>
		Vec2Int m_outputTargetSize;
		bool m_isBoundActive;
	public:

	private:
		void Deallocate();
		void RemoveOutputAt(const size_t i);
		FrameBufferOutputTarget& GetTargetFromType(const FrameBufferAttachmentType type);
	public:
		FrameBuffer();
		FrameBuffer(const FrameBufferPlatformCallbacks& callbacks);
		FrameBuffer(const FrameBuffer&) = delete;
		FrameBuffer(FrameBuffer&&) noexcept;
		~FrameBuffer();

		void BindActive();
		void UnbindActive();
		bool IsBoundActive() const;

		void SetOutputTexture(const FrameBufferAttachmentType type, Texture* tex);
		void SetOutputTextureCube(const FrameBufferAttachmentType type, TextureCube* cube, const TextureCubeFace face);
		void SetOutputRenderBuffer(const FrameBufferAttachmentType type, RenderBuffer* buffer);
		void RemoveOutput(const FrameBufferAttachmentType type);
		void RemoveAllOutputs();
		//const Texture* GetOutputTarget(const FrameBufferAttachmentType tpye) const;

		FrameBuffer& operator=(const FrameBuffer&) = delete;
		FrameBuffer& operator=(FrameBuffer&&) noexcept;
	};

	struct BufferSegment
	{
		size_t m_ByteOffset;
		size_t m_ByteSize;
	};

	struct FencedBufferSegment
	{
		size_t m_ByteOffset;
		size_t m_ByteSize;
		GpuFence m_Fence;

		size_t GetNextOffset() const;
		std::string ToString() const;
	};
	class RingBufferAllocator
	{
	private:
		std::deque<FencedBufferSegment> m_segments;
		//int m_currSegmentIndex;
		//size_t nextOffset;
		//Vec2Int m_allocatedRange;
		size_t m_fixedByteSize;
		size_t m_usedSize;
		size_t m_head;
		size_t m_tail;
	public:
		static constexpr float STATUS_TIMEOUT_NS = 0;
		static constexpr int INVALID_RANGE_INDEX = -1;
		static constexpr Vec2Int INVALID_RANGE = Vec2Int(INVALID_RANGE_INDEX, 0);

	private:
		//void UpdateRangeFromSegment(const FencedBufferSegment& seg);
		bool TryRemoveFinishedHeadSegments();
		FencedBufferSegment& ReserveSegment(const size_t offset, const size_t size);
	public:
		RingBufferAllocator(const size_t allocatedByteSize);

		/// <summary>
		/// Will attempt to find next free segment (NOTE: DOES NOT RESERVE IT)
		/// </summary>
		/// <param name="size"></param>
		/// <param name="doStall"></param>
		/// <returns></returns>
		std::optional<BufferSegment> TryGetFreeSegment(const size_t size, const bool doStall);
		/// <summary>
		/// Finds the next available fenced segment for given size.
		/// If stall is true, will wait by doing busywait loop until the gpu is free (bad for performance)
		/// but the preferred option, false, will return false and not wait at all
		/// If reserve is true, will add to used segments
		/// </summary>
		/// <param name="size"></param>
		/// <returns></returns>
		bool TryReserveSegment(const size_t size, const bool doStall, FencedBufferSegment** outSeg);
		FencedBufferSegment& ReserveSegment(const BufferSegment& segment);
		//void InsertFenceAtCurrentSegment();
		size_t GetFixedAllocatedByteSize() const;
		size_t GetUnusedByteSize() const;
		size_t GetUsedByteSize() const;
		size_t GetContiguousSizeFromTail() const;

		/// <summary>
		/// Will get all ranges in the buffer that are not used as [OFFSET, SIZE]
		/// NOTE: based on tail and head, there should only ever be a max of 2 non-contiguous
		/// unused ranges at any given point
		/// </summary>
		/// <returns></returns>
		std::array<Vec2Int, 2> GetUnusedRanges() const;

		const std::deque<FencedBufferSegment>& GetSegments() const;

		std::string ToString() const;
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
		std::tuple<RenderObjectId, std::byte*>(*m_AllocateFunc) (const void* buffer, const size_t totalByteSize, const bool allowPersistentReading);
		void(*m_WriteFunc) (const RenderObjectId id, const size_t byteOffset, const void* buffer, const size_t bufferByteSize);
		void(*m_DeallocateFunc)(const RenderObjectId id);
	};

	template<typename T>
	class FencedBufferBase
	{
	private:
	protected:
		RenderObjectId m_id;
		std::byte* m_writePtr;
		/// <summary>
		/// How many indices were actually uploaded to the buffer
		/// Size in bytes is m_dataUsed * sizeof(IndexType)
		/// </summary>
		//size_t m_dataUsed;
		//size_t m_elementCapacity;
		RingBufferAllocator m_fence;
	public:

	private:
	protected:
		virtual void WriteDataUnsafeBytes(const size_t offsetBytes, const T* arr, const size_t& writeByteSize) = 0;

	public:
		FencedBufferBase() : FencedBufferBase(nullptr, 0) {}
		FencedBufferBase(const T* array, const size_t elementSize, const size_t elementCount)
			: m_id(INVALID_OBJ_ID), m_writePtr(nullptr), m_fence(elementCount* elementSize) {}

		/// <summary>
		/// Writes data directly into the buffer
		/// </summary>
		/// <param name="elementOffset"></param>
		/// <param name="vertexArray"></param>
		/// <param name="elementCount"></param>
		void WriteDataUnsafe(const size_t& elementOffset, const T* array, const size_t& elementCount)
		{
			WriteDataUnsafeBytes(elementOffset * GetElementSize(), array, elementCount * GetElementSize());
		}

		FencedBufferSegment* TryReserveFence(const size_t& elementCount)
		{
			FencedBufferSegment* segment = nullptr;
			m_fence.TryReserveSegment(elementCount * GetElementSize(), false, &segment);
			return segment;
		}
		/// <summary>
		/// Will get the next free segment WITHOUT reserving the space
		/// </summary>
		/// <param name="elementCount"></param>
		/// <returns></returns>
		std::optional<BufferSegment> TryGetFreeSegment(const size_t& elementCount)
		{
			return m_fence.TryGetFreeSegment(elementCount * GetElementSize(), false);
		}
		FencedBufferSegment& ReserveFence(const BufferSegment& segment)
		{
			return m_fence.ReserveSegment(segment);
		}
		
		/// <summary>
		/// Writes data into the next available unfenced slot or waits until one is available
		/// </summary>
		/// <param name="vertexArray"></param>
		/// <param name="elementCount"></param>
		bool TryWriteDataFenced(const T* arr, const size_t& elementCount, FencedBufferSegment** outSeg)
		{
			FencedBufferSegment* segment = TryReserveFence(elementCount * GetElementSize());
			if (segment == nullptr)
				return false;

#ifdef GRAPHICS_VERBOSE_LOG
			LogWarning(std::format("Writing fenced buffer base  at offset:{} size:{} alloc:{} fence:{}",
				segment->m_ByteOffset, segment->m_ByteSize, m_fence.GetFixedAllocatedByteSize(), m_fence.ToString()));
#endif
			WriteDataUnsafeBytes(segment->m_ByteOffset, arr, segment->m_ByteSize);

			if (outSeg != nullptr) *outSeg = segment;
			return true;
		}
		FencedBufferSegment& WriteDataFenced(const T* arr, const BufferSegment& segment)
		{
			FencedBufferSegment& fencedSegment = ReserveFence(segment);
			WriteDataUnsafeBytes(fencedSegment.m_ByteOffset, arr, fencedSegment.m_ByteSize);

			return fencedSegment;
		}

		virtual size_t GetElementSize() const = 0;

		size_t GetAllocatedByteSize() const { return m_fence.GetFixedAllocatedByteSize(); }
		size_t GetCapacity() const { return m_fence.GetFixedAllocatedByteSize() / GetElementSize(); }
		RenderObjectId GetId() const { return m_id; }
		bool HasPersistentReadWritePointer() const { return m_writePtr != nullptr; }
	};

	class VertexBuffer : public FencedBufferBase<void>
	{
	private:
		VertexBufferPlatformCallbacks m_callbacks;
		/// <summary>
		/// How many vertices were actually uploaded to the buffer
		/// Size in bytes is m_dataUsed * sizeof(VertexType)
		/// </summary>
		//size_t m_dataUsed;
		size_t m_elementSize;
	public:
		VertexAttributeAdvance m_AdvanceType;

	private:
		void WriteDataUnsafeBytes(const size_t offsetBytes, const void* vertexArray, const size_t& writeByteSize) override;
		void Deallocate();
		void DefaultUninitValues();
	public:
		VertexBuffer();
		VertexBuffer(const void* vertexArray, const size_t& elementSize, const size_t& arraySize, const VertexAttributeAdvance advanceType,
			const VertexBufferPlatformCallbacks callbacks);
		VertexBuffer(const VertexBuffer&) = delete;
		VertexBuffer(VertexBuffer&&) = delete;
		~VertexBuffer();

#if !PRODUCTION_BUILD
		template<typename T>
		requires (std::is_default_constructible_v<T>)
		void ReadDataAs(std::vector<T>& vec, const bool hasArraySegments)
		{
			if (m_writePtr == nullptr)
			{
				LogError(std::format("Attempted to read a vertex buffer "
					"but it contains no write pointer to read from"));
				return;
			}

			const auto& segments = m_fence.GetSegments();
			vec.resize(segments.size());

			size_t vecIndex = 0;

			T value = {};
			for (const auto& segment : segments)
			{
				if (hasArraySegments)
				{
					for (size_t i = segment.m_ByteOffset; i < segment.GetNextOffset(); i += sizeof(T))
					{
						value = {};
						memcpy(&value, m_writePtr + size_t(segment.m_ByteOffset), sizeof(T));
						vec[vecIndex] = value;
						vecIndex++;
					}
				}
				else
				{
					value = {};
					memcpy(&value, m_writePtr + size_t(segment.m_ByteOffset), sizeof(T));
					vec[vecIndex] = value;
					vecIndex++;
				}
			}
		}
#endif

		size_t GetElementSize() const override;

		VertexBuffer& operator=(const VertexBuffer&) = delete;
		VertexBuffer& operator=(VertexBuffer&&) noexcept;

		std::string ToString() const;
	};

	struct IndexBufferPlatformCallbacks
	{
		std::tuple<RenderObjectId, std::byte*>(*m_AllocateFunc) (const IndexType* buffer, const size_t totalByteSize, const bool allowPersistentReading);
		void(*m_WriteFunc) (const RenderObjectId id, const size_t byteOffset, const IndexType* buffer, const size_t bufferByteSize);
		void(*m_DeallocateFunc)(const RenderObjectId id);
	};

	class IndexBuffer : public FencedBufferBase<IndexType>
	{
	private:
		IndexBufferPlatformCallbacks m_callbacks;
	public:

	private:
		void WriteDataUnsafeBytes(const size_t offsetBytes, const IndexType* indexArray, const size_t& writeByteSize) override;
		void Deallocate();
	public:
		IndexBuffer();
		IndexBuffer(const IndexType* indexArray, const size_t elementCount,
			const IndexBufferPlatformCallbacks& callbacks);
		IndexBuffer(const IndexBuffer& other) = delete;
		IndexBuffer(IndexBuffer&& other) = delete;
		~IndexBuffer();

		size_t GetElementSize() const override;

		IndexBuffer& operator=(const IndexBuffer&) = delete;
		IndexBuffer& operator=(IndexBuffer&&) noexcept;
	};


	struct ShaderBufferPlatformCallbacks
	{
		RenderObjectId(*m_AllocateFunc)(const size_t byteSize);
		void(*m_BindFunc)(const RenderObjectId id, const BufferBindIndex index);
		void(*m_WriteFunc)(const RenderObjectId id, const size_t byteOffset, const size_t writeByteSize, const void* data);
		void(*m_ReadFunc)(const RenderObjectId id, const size_t byteOffset, const size_t readSize, void* outData);
		void(*m_DeallocateFunc)(const RenderObjectId);
	};

	class ShaderBuffer
	{
	private:
	protected:
		ShaderBufferPlatformCallbacks m_platformCallbacks;
		BufferBindIndex m_bindIndex;
		RenderObjectId m_id;

		std::string_view m_blockName;
		size_t m_allocatedByteSize;

		std::unordered_map<std::string, ShaderBlockMemberMemoryInfo> m_members;
	public:

	private:
	public:
		ShaderBuffer();
		ShaderBuffer(const char* blockName, const ShaderBufferPlatformCallbacks callbacks);
		ShaderBuffer(const ShaderBuffer& other) = delete;
		ShaderBuffer(ShaderBuffer&& other) noexcept;
		~ShaderBuffer();

		bool IsAllocated() const;
		virtual void AllocateFromShaderBlock(const Shader& shader) = 0;

		/*void AddMember(const char* name, const size_t size, const size_t alignment);
		void FinishLayout();*/
		void SetBindingPoint(const BufferBindIndex bindIndex);
		void LinkBufferToBindingPoint(const BufferBindIndex bindIndex);
		void LinkBufferToCurrentBindingPoint();
		bool HasValidBindingPoint() const;

		void WriteData(const size_t byteOffset, const size_t writeByteSize, const void* data);
		bool TryWriteField(const char* memberName, const size_t writeSize, const void* data);

		bool TryWriteStruct(const std::string& structName, const size_t structSize, const void* data);
		bool TryWriteStructMember(const std::string& structName, const std::string& memberName, const size_t memberSize, const void* data);
		/// <summary>
		/// Will write data as FULL ARRAY for the primitive-type array member 
		/// (primitive types are all basic types that are NOT custom structs)
		/// and can write to fixed or dyanmic array
		/// </summary>
		/// <param name="arrayName"></param>
		/// <param name="data"></param>
		/// <returns></returns>
		bool TryWritePrimitiveArray(const char* arrayName, const void* data);
		/// <summary>
		/// Similar to TryWritePrimitiveArray, but can only write to dynamic and requires total size
		/// </summary>
		/// <param name="arrayName"></param>
		/// <param name="data"></param>
		/// <param name="size"></param>
		/// <returns></returns>
		bool TryWriteDynamicArray(const char* arrayName, const void* data, const size_t elementCount);
		/// <summary>
		/// Will write a SINGULAR element within a primitive-type array
		/// (primitive types are all basic types that are NOT custom structs)
		/// </summary>
		/// <param name="arrayName"></param>
		/// <param name="index"></param>
		/// <param name="data"></param>
		/// <returns></returns>
		bool TryWritePrimitiveArrayElement(const char* arrayName, const size_t index, const void* data);

		bool TryWriteStructArray(const std::string& arrayName, const void* data);
		bool TryWriteStructDynamicArray(const std::string& arrayName, const void* data, const size_t elementCount);
		bool TryWriteStructArrayElement(const std::string& arrayName, const size_t index, const void* data);
		bool TryWriteStructArrayElementMember(const std::string& arrayName, const size_t index,
			const std::string& memberName, const size_t size, const void* data);

		BufferBindIndex GetBindIndex() const;
		RenderObjectId GetId() const;
		std::string_view GetName() const;
		size_t GetAllocatedByteSize() const;

		void ReadData(const size_t offset, const size_t size, std::byte* writeDataPtr);

		/// <summary>
		/// Creates a heap allocated pointer which data in the buffer is copied 
		/// to and then reinterpreted as the T type. 
		/// NOTE: this shoudl rarely be used as it is SLOW
		/// </summary>
		/// <typeparam name="T"></typeparam>
		/// <param name="outDataPtr"></param>
		/// <returns></returns>
		template<typename T>
		requires (std::is_trivially_copyable_v<T>)
		T ReadDataAs()
		{
			T dataT = {};
			ENGINE_ASSERT(sizeof(T) == GetAllocatedByteSize(),
				"Attempted to read shader buffer data as type: {} which size:{} does not match allocated size:{}",
				typeid(T).name(), sizeof(T), GetAllocatedByteSize());
			ReadData(0, GetAllocatedByteSize(), reinterpret_cast<std::byte*>(&dataT));
			return dataT;
		}

		template<typename T>
		requires (std::is_trivially_copyable_v<T>)
		bool TryReadField(const char* memberName, T* outValue)
		{
			auto it = m_members.find(memberName);
			if (it == m_members.end())
			{
				LogError(std::format("Attempted to read shader buffer for member:'{}' "
					"but it could not be found out of {} members. Members:{}", memberName, m_members.size(), ToString()));
				return false;
			}

			if (outValue != nullptr)
			{
				ReadData(it->second.m_ByteOffset, sizeof(T), reinterpret_cast<std::byte*>(outValue));
			}
			return true;
		}

		ShaderBuffer& operator=(const ShaderBuffer&) = delete;
		ShaderBuffer& operator=(ShaderBuffer&&) noexcept;

		virtual std::string ToString() const;
	};

	class UniformBuffer : public ShaderBuffer
	{
	private:
	public:

	private:
	public:
		UniformBuffer();
		UniformBuffer(const char* blockName, const ShaderBufferPlatformCallbacks callbacks);
		UniformBuffer(const UniformBuffer&) = delete;
		UniformBuffer(UniformBuffer&&) noexcept = default;
		~UniformBuffer() = default;

		void AllocateFromShaderBlock(const Shader& shader) override;

		UniformBuffer& operator=(const UniformBuffer&) = delete;
		UniformBuffer& operator=(UniformBuffer&&) noexcept = default;

		std::string ToString() const override;
	};

	class ShaderStorageBuffer : public ShaderBuffer
	{
	private:
	public:

	private:
	public:
		ShaderStorageBuffer();
		ShaderStorageBuffer(const char* bufferName, const ShaderBufferPlatformCallbacks& callbacks);
		ShaderStorageBuffer(const ShaderStorageBuffer&) = delete;
		ShaderStorageBuffer(ShaderStorageBuffer&&) = default;
		~ShaderStorageBuffer() = default;

		void AllocateFromShaderBlock(const Shader& shader) override;
		void DeferAllocatonFromShaderUntilWrite(Shader& shader);

		ShaderStorageBuffer& operator=(const ShaderStorageBuffer&) = delete;
		ShaderStorageBuffer& operator=(ShaderStorageBuffer&&) noexcept = default;

		std::string ToString() const override;
	};

	/// <summary>
	/// The type of data that the attribute holds.
	/// Note: a vec4 -> 4 floats
	/// Note: matrix4 -> need 4 attributes of 4 floats each
	/// </summary>
	enum class VertexAttributeBaseType : std::uint8_t
	{
		Float				= 0,
		Integer				= 1,
		UnsignedInteger		= 2,
	};

	using VertexLayoutBindIndex = std::uint8_t;
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
		/// Thee number of components for this attribute.
		/// Example: vec4 -> 4 vec2 -> 2 float -> 1
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

		/// <summary>
		/// The bind index connects the vertex attribute of a buffer to 
		/// the vertex array object
		/// </summary>
		VertexLayoutBindIndex m_BufferBindIndex = 0;
	};

	inline constexpr size_t IMPL_STATE_SIZE = 4;
	struct VertexLayoutCallbacks
	{
		void(*m_InitFunc)(std::array<std::byte, IMPL_STATE_SIZE>&);
		void(*m_AddAttributeFunc)(std::array<std::byte, IMPL_STATE_SIZE>&, const VertexAttribute&);
		void(*m_BindVertexBufferFunc)(std::array<std::byte, IMPL_STATE_SIZE>&, const RenderObjectId, const size_t elementSize, 
			const VertexLayoutBindIndex, const VertexAttributeAdvance advanceType);
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
		void LinkToBuffer(const RenderObjectId id, const size_t elementSize, 
			const VertexAttributeAdvance advance, const VertexLayoutBindIndex bindIndex);
		void Deallocate();
	public:
		VertexLayout();
		VertexLayout(const VertexLayoutCallbacks& callbacks);
		VertexLayout(const VertexLayout&) = delete;
		VertexLayout(VertexLayout&&) = delete;
		~VertexLayout();

		void AddAttribute(const VertexAttribute& attribute);
		/// <summary>
		/// Will add all the attributes to the buffer.
		/// Note: the advance type is automatically set to match the same type as the buffer
		/// to prevent inconsistencies as well as the bindIndex
		/// </summary>
		/// <param name="bufferBindIndex"></param>
		/// <param name="attributes"></param>
		void AddAttributes(const VertexLayoutBindIndex bufferBindIndex, std::vector<VertexAttribute>& attributes);
		/// <summary>
		/// Rather than creating the 4 separate attributes for every column, you can plug in some basic data and the 
		/// rest will be generated.
		/// Note: initial byte offset is the offset of the first float of the matrix from the vertex element.
		/// so you would do "offsetof(VERTEX_BUFFER_ELEMENT_TYPE, m_MATRIX_MEMBER_NAME)
		/// Note: columnTypesize is the sizeof(MATRIX_COLUMN_TYPE) -> should be a 4d vector type
		/// NOTE: matrix size is ROW, COL
		/// </summary>
		/// <param name="bufferBindIndex"></param>
		/// <param name="startLocation"></param>
		/// <param name="normalize"></param>
		/// <param name="initialByteOffset"></param>
		void AddMatrixAttribute(const Vec2Int& matrixSize, const VertexLayoutBindIndex bufferBindIndex,
			const ShaderLocation startLocation, const bool normalize, const size_t matrixColumnTypeSize, const ByteOffset initialByteOffset);

		//Note: index buffers are NOT linked to vertex layout explicitly
		void LinkToBuffer(const VertexBuffer& buffer, const VertexLayoutBindIndex bindIndex);

		const VertexAttribute* GetAttributeByLocation(const std::uint8_t shaderLocation) const;
		const VertexAttribute* GetAttributeByBindIndex(const std::uint8_t bindIndex) const;

		VertexLayout& operator=(const VertexLayout&) = delete;
		VertexLayout& operator=(VertexLayout&& other) noexcept;
	};

	struct BufferProperties
	{
		/// <summary>
		/// The bind index that was used to associate the buffer with the 
		/// vertex layout. Note: INDEX BUFFERS DO NOT HAVE BIND INDICES
		/// SINCE THEY ARE IMPLICITLY BOUND AFTER CREATION (OPENGL)
		/// </summary>
		VertexLayoutBindIndex m_VertexBufferBindIndex = 0;
		VertexLayout* m_Layout = nullptr;
		VertexBuffer* m_VertexBuffer = nullptr;
		IndexBuffer* m_IndexBuffer = nullptr;
	};

	struct ShaderBufferProperties
	{
		BufferBindIndex m_BindIndex = INVALID_BUFFER_BIND_INDEX;
		ShaderBuffer* m_ShaderBuffer = nullptr;
	};

	class BufferController
	{
	private:
		std::vector<BufferProperties> m_bufferData;
		std::vector<ShaderBufferProperties> m_shaderBufferData;
	public:

	private:
	public:
		BufferController();

		VertexLayoutBindIndex AddVertexBuffer(VertexLayout* layout, VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer);
		/// <summary>
		/// Note: the buffer must already be created BEFORE you link to ensure it exits before placing in slot
		/// </summary>
		/// <param name="buffer"></param>
		/// <returns></returns>
		BufferBindIndex AddShaderBuffer(ShaderBuffer* buffer);

		BufferProperties* GetBufferDataMutable(const VertexLayoutBindIndex bindIndex);
	};

	template<typename T>
	class LazyStaticBuffer
	{
	private:
		bool m_isInit;
		T m_buffer;
	public:

	private:
	public:
		LazyStaticBuffer(T&& buffer) : m_buffer(std::move(buffer)), m_isInit(false) {}

		T& GetBufferMutable() { return m_buffer; }
		const T& GetBuffer() { return m_buffer; }
		T& GetBufferForInit()
		{
			m_isInit = true;
			return m_buffer;
		}

		bool IsInit() const { return m_isInit; }
	};

	enum class BufferType : std::uint8_t
	{
		None		= 0,
		Vertex		= 1,
		Index		= 1<<1,
		Instance	= 1<<2,
		All			= 0xff
	};

	/*
	class RenderUnit
	{
	private:
		VertexLayout* m_layout;
		std::vector<std::byte> m_cpuVertices;
		std::vector<std::byte> m_cpuInstances;
		std::vector<std::byte> m_cpuIndices;

		IndexBuffer m_indexBufferHandle;
		VertexBuffer m_vertexBufferHandle;
		VertexBuffer m_instanceBufferHandle;
	public:

	private:
	public:
		RenderUnit(VertexLayout& layout, const BufferType createBufferFlags);
	};
	*/
}