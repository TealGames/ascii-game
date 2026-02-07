#include "Core/Rendering/Texture.hpp"
#include "Utils/AnsiCodes.hpp"

#if defined(OPENGL)
#include "Platform/OpenGl/OpenGlTexture.hpp"
#endif

namespace Rendering
{
	std::uint8_t GetChannelCount(const TexelChannelFormat format)
	{
		if (format == TexelChannelFormat::Depth || format == TexelChannelFormat::Depth_Stencil)
			return 1;

		return static_cast<TexelChannelFormatIntegralType>(format) + 1;
	}
	std::uint16_t GetChannelByteSize(const TexelChannelDataType dataType)
	{
		if (dataType == TexelChannelDataType::Byte)
			return 1;
		else if (dataType == TexelChannelDataType::Float16)
			return 2;
		else if (dataType == TexelChannelDataType::Float32 || 
			dataType == TexelChannelDataType::UInt32 || dataType == TexelChannelDataType::UInt24_8)
			return 4;
		
		LogError(std::format("Failed to convert channel data type to channel byte size due to no actions"));
		return 0;
	}

	std::string ToString(const TexelStorageType storage)
	{
		if (storage == TexelStorageType::R8)
			return "R8";

		else if (storage == TexelStorageType::RGB8)
			return "RGB8";
		else if (storage == TexelStorageType::RGB16F)
			return "RGB16F";
		else if (storage == TexelStorageType::RGB32F)
			return "RGB32F";

		else if (storage == TexelStorageType::RGBA8)
			return "RGBA8";
		else if (storage == TexelStorageType::RGBA16F)
			return "RGBA16F";
		else if (storage == TexelStorageType::RGBA32F)
			return "RGBA32F";

		else if (storage == TexelStorageType::Depth24)
			return "Depth24";
		else if (storage == TexelStorageType::Depth24_Stencil8)
			return "Deoth32Stencil8";

		LogError(std::format("Failed to convert internal storage to string due to no actions"));
		return "";
	}
	TexelChannelFormat GetChannelFormatFromStorage(const TexelStorageType storage)
	{
		return static_cast<TexelChannelFormat>(
			static_cast<TexelStorageIntegralType>(storage)/ INTERNAL_STORAGE_PER_FORMAT);
	}
	std::uint8_t GetChannelCount(const TexelStorageType storage)
	{
		return static_cast<TexelStorageIntegralType>(storage) / INTERNAL_STORAGE_PER_FORMAT + 1;
	}
	std::uint16_t GetChannelByteSize(const TexelStorageType storage)
	{
		if (storage == TexelStorageType::R8 || storage == TexelStorageType::RGB8 || storage == TexelStorageType::RGBA8)
			return 1;
		else if (storage == TexelStorageType::RGB16F || storage == TexelStorageType::RGBA16F)
			return 2;
		else if (storage == TexelStorageType::RGB32F || storage == TexelStorageType::RGBA32F || 
			storage == TexelStorageType::Depth24 || storage == TexelStorageType::Depth24_Stencil8)
			return 4;

		LogError(std::format("Failed to convert texel storage to channel byte size due to no actions"));
		return 0;
	}
	TexelChannelDataType GetChannelDataTypeFromStorage(const TexelStorageType storage)
	{
		if (storage == TexelStorageType::R8 || storage == TexelStorageType::RGB8 || storage == TexelStorageType::RGBA8)
			return TexelChannelDataType::Byte;
		else if (storage == TexelStorageType::RGB16F || storage == TexelStorageType::RGBA16F)
			return TexelChannelDataType::Float16;
		else if (storage == TexelStorageType::RGB32F || storage == TexelStorageType::RGBA32F)
			return TexelChannelDataType::Float32;
		else if (storage == TexelStorageType::Depth24)
			return TexelChannelDataType::UInt32;
		else if (storage == TexelStorageType::Depth24_Stencil8)
			return TexelChannelDataType::UInt24_8;

		LogError(std::format("Failed to convert texel storage to channel byte size due to no actions"));
		return TexelChannelDataType::Byte;
	}
	std::uint16_t GetStorageByteSize(const TexelStorageType storage)
	{
		if (storage == TexelStorageType::R8)
			return 1;

		else if (storage == TexelStorageType::RGB8)
			return 3;
		else if (storage == TexelStorageType::RGB16F)
			return 6;
		else if (storage == TexelStorageType::RGB32F)
			return 12;

		else if (storage == TexelStorageType::RGBA8)
			return 4;
		else if (storage == TexelStorageType::RGBA16F)
			return 8;
		else if (storage == TexelStorageType::RGBA32F)
			return 16;

		else if (storage == TexelStorageType::Depth24)
			return 4;
		else if (storage == TexelStorageType::Depth24_Stencil8)
			return 4;

		LogError(std::format("Failed to convert internal storage to texel storage type due to no actions"));
		return 0;
	}
	bool IsHDRStorage(const TexelStorageType storage)
	{
		return GetChannelByteSize(storage) > 1;
	}
	AxesWrapBehavior CreateXYZWrapBehavior(const WrapBehavior xyzBehavior)
	{
		return AxesWrapBehavior({ xyzBehavior, xyzBehavior, xyzBehavior });
	}

	std::string TextureInfo::ToString() const
	{
		return std::format("[TextureData]");
	}
	bool TextureCallbacks::IsValid() const { return m_AllocateFunc != nullptr; }

	Texture::Texture() : Texture(nullptr, {}, TextureBufferType::GPU) {}
	Texture::Texture(const std::byte* data, const Vec2Int& size, const TextureBufferType textureStorage, const TexelStorageType internalStorage,
		const AxesWrapBehavior wrap, const MinFilter min, const MagFilter mag, const TextureCallbacks& callbacks)
		: m_gpuCallbacks(callbacks), m_info{ INVALID_OBJ_ID, textureStorage, size, internalStorage, wrap, min, mag}, m_cpuPtr(nullptr)
	{
		if (size.m_X == 0 || size.m_Y == 0)
			return;

		Allocate();
		if (data != nullptr)
			WriteBytes(data);
	}
	Texture::Texture(Texture&& other) noexcept
		: m_gpuCallbacks(std::exchange(other.m_gpuCallbacks, {})), m_info(std::exchange(other.m_info, {})), m_cpuPtr(nullptr)
	{
		if (other.HasCPUBuffer())
		{
			m_cpuPtr = other.m_cpuPtr;
			other.m_cpuPtr = nullptr;
		}
	}

	Texture::~Texture()
	{
		Deallocate();
	}
	void Texture::AllocateUnsafeToCPU()
	{
		m_cpuPtr = new std::byte[CalculateTotalByteSize()];
	}
	void Texture::AllocateUnsafeToGPU()
	{
		m_info.m_Id = m_gpuCallbacks.m_AllocateFunc(m_info);
	}
	void Texture::Allocate()
	{
		Deallocate();
		if (m_info.m_BufferType == TextureBufferType::CPU ||
			m_info.m_BufferType == TextureBufferType::GPUThreadSafeRead)
		{
			AllocateUnsafeToCPU();
		}
		if (m_info.m_BufferType == TextureBufferType::GPU ||
			m_info.m_BufferType == TextureBufferType::GPUThreadSafeRead)
		{
			AllocateUnsafeToGPU();
		}
	}
	void Texture::DeallocateUnsafeFromCPU()
	{
		delete[] m_cpuPtr;
		m_cpuPtr = nullptr;
	}
	void Texture::DeallocateUnsafeFromGPU()
	{
		m_gpuCallbacks.m_DeallocateFunc(m_info.m_Id);
		m_info.m_Id = INVALID_OBJ_ID;
	}
	void Texture::Deallocate()
	{
		if (!IsValid())
			return;

		if (HasCPUBuffer()) DeallocateUnsafeFromCPU();
		if (HasGPUBuffer()) DeallocateUnsafeFromGPU();

		m_info = {};
	}

	size_t Texture::CalculateTexelByteOffset(const Vec2Int texel) const
	{
		//NOTE: this assumes the (0,0) is at the data pointer at the BOTTOM LEFT corner of the texture
		return (texel.m_Y * m_info.m_TexelSize.m_X + texel.m_X) * GetTexelByteSize();
	}

	void Texture::WriteBytesUnsafeToCPU(const Vec2Int texelOffset, const Vec2Int texelSize, 
		const std::byte* readLocationPointer, const std::optional<TexelChannelDataType>& overrideSourceDataType)
	{
		if (overrideSourceDataType == std::nullopt)
		{
			const size_t byteOffset = CalculateTexelByteOffset(texelOffset);
			memcpy(m_cpuPtr + byteOffset, readLocationPointer, CalculateTotalByteSize(texelSize));
		}
		else
		{
			//TODO: implement
			LogError("NOT SUPPORTED");
		}
	}
	void Texture::WriteBytesUnsafeToGPU(const Vec2Int texelOffset, const Vec2Int texelSize, 
		const std::byte* readLocationPointer, const std::optional<TexelChannelDataType>& overrideSourceDataType)
	{
		m_gpuCallbacks.m_WriteBytes(m_info.m_Id, texelOffset, texelSize, GetChannelFormatFromStorage(m_info.m_InternalStorage), 
			overrideSourceDataType.value_or(GetChannelDataTypeFromStorage(m_info.m_InternalStorage)), readLocationPointer);
	}
	void Texture::WriteBytesUnsafe(const Vec2Int texelOffset, const Vec2Int texelSize, 
		const std::byte* readLocationPointer, const std::optional<TexelChannelDataType>& overrideSourceDataType)
	{
		ENGINE_ASSERT(!texelOffset.AnyAxisLessThan(Vec2Int::Zero()) && !texelSize.AnyAxisLessThan(Vec2Int::One()),
			"Attempted to read bytes unsafe but either texel offset:{} is negative and/or texel size:{} is less than 1:{}",
			texelOffset.ToString(), texelSize.ToString());

		//If we have GPU thread safe read, we still want to write to both so most up to date data is stored in CPU for reads
		if (HasCPUBuffer()) WriteBytesUnsafeToCPU(texelOffset, texelSize, readLocationPointer, overrideSourceDataType);
		if (HasGPUBuffer()) WriteBytesUnsafeToGPU(texelOffset, texelSize, readLocationPointer, overrideSourceDataType);
	}
	void Texture::WriteBytes(const Vec2Int texelOffset, const Vec2Int texelSize, const std::byte* readLocationPointer, 
		const std::optional<TexelChannelDataType>& overrideSourceDataType)
	{
		ENGINE_ASSERT(readLocationPointer != nullptr, "Attempted to write bytes to texture from null read pointer");
		ENGINE_ASSERT(IsValid(), "Attempted to write bytes to invalid texture:{}", ToString());

		const Vec2Int maxOffset = texelOffset + texelSize;
		ENGINE_ASSERT(maxOffset.m_X <= m_info.m_TexelSize.m_X && maxOffset.m_Y <= m_info.m_TexelSize.m_Y,
			"Attempted to WRITE BYTES of texture: {} but offset and size: {} > texture size:{}",
			ToString(), maxOffset.ToString(), m_info.m_TexelSize.ToString());

		WriteBytesUnsafe(texelOffset, texelSize, readLocationPointer, overrideSourceDataType);
	}
	void Texture::WriteBytes(const std::byte* readLocationPointer)
	{
		ENGINE_ASSERT(readLocationPointer != nullptr, "Attempted to write bytes to texture from null read pointer");
		ENGINE_ASSERT(IsValid(), "Attempted to write bytes to invalid texture:{}", ToString());
		//NOTE: since we just use the full texture size, the size/offset is guaranteed to be correct
		WriteBytesUnsafe(Vec2Int::Zero(), m_info.m_TexelSize, readLocationPointer, std::nullopt);
	}
	void Texture::WriteTexel(const Vec2Int& texel, const std::byte* readLocationPointer, 
		const std::optional<TexelChannelDataType>& overrideSourceDataType)
	{
		ENGINE_ASSERT(readLocationPointer != nullptr, "Attempted to write texel:{} to texture from null read pointer", texel.ToString());
		ENGINE_ASSERT(IsValid(), "Attempted to write texel to invalid texture:{}", ToString());
		WriteBytesUnsafe(texel, Vec2Int::One(), readLocationPointer, overrideSourceDataType);
	}

	void Texture::CopyBytes(const Texture& texture)
	{
		ENGINE_ASSERT(IsValid(), "Attempted to copy bytes from texture:{} to invalid texture: {}", texture.ToString(), ToString());
		ENGINE_ASSERT(CalculateTotalByteSize() == texture.CalculateTotalByteSize() && GetStorageType() == texture.GetStorageType(), 
			"Attempted to set texture:{} data from texture:{} "
			"but textures do not have matching size and/or internal storage types", ToString(), texture.ToString());
	
		//If we need CPU buffer and other texture has:
		// -> OTHER TEX CPU BUFFER: we can directly copy data in other cpuPtr to this
		// -> OTHER TEX GPU BUFFER ONLY: read bytes from texture and copy them into this cpuPtr
		if (HasCPUBuffer())
		{
			if (texture.HasCPUBuffer()) WriteBytesUnsafeToCPU(Vec2Int::Zero(), m_info.m_TexelSize, texture.m_cpuPtr);
			else if (texture.HasGPUBuffer()) texture.ReadBytesUnsafeFromGPU(Vec2Int::Zero(), m_info.m_TexelSize, m_cpuPtr);
		}
		//If we need GPU buffer and:
		// -> OTHER TEX GPU BUFFER: we can do a copy from gpu to gpu with no cpu middleman
		// -> OTHER TEX CPU BUFFER: if we dont have CPU buffer just use the other texture CPU data to copy to GPU
		if (HasGPUBuffer())
		{
			if (texture.HasGPUBuffer()) m_gpuCallbacks.m_CopyData(m_info.m_Id, m_info.m_TexelSize, texture);
			else if (texture.HasCPUBuffer()) WriteBytesUnsafeToGPU(Vec2Int::Zero(), m_info.m_TexelSize, texture.m_cpuPtr);
		}
	}
	void Texture::CopyTexture(const Texture& texture, const std::optional<TextureBufferType>& overrideType)
	{
		Deallocate();
			
		m_info = texture.m_info;
		//NOTE: we ALWAYS want to have invalid ID since that is either required for CPU only buffer OR
		//it should be reassigned a new ID after allocate
		m_info.m_Id = INVALID_OBJ_ID;
		m_info.m_BufferType = overrideType.value_or(texture.m_info.m_BufferType);
		m_gpuCallbacks = texture.m_gpuCallbacks;
		ENGINE_ASSERT(m_gpuCallbacks.IsValid(), "Attempted to copy texture with invalid callbacks:{} to this texture:{} which is not allowed. "
			"All textures, including only CPU textures must have GPU callbacks in case the buffer type is changed", texture.ToString(), ToString());

		Allocate();
		CopyBytes(texture);
	}
	void Texture::ClearBytes()
	{
		ENGINE_ASSERT(IsValid(), "Attempted to clear bytes from invalid texture:{}", ToString());
		const bool hasCpuBuffer = HasCPUBuffer();
		if (hasCpuBuffer)
		{
			memset(m_cpuPtr, 0, CalculateTotalByteSize());
		}
		if (HasGPUBuffer())
		{
			std::byte* emptyDataPtr = nullptr;
			//If we have a CPU buffer AND a GPU buffer, the CPU buffer should have been cleared
			//so we can just use it for the clear data ptr for GPU, otherwise we allocate on heap emtpy bytes
			if (hasCpuBuffer) emptyDataPtr = m_cpuPtr;
			else emptyDataPtr = new std::byte[CalculateTotalByteSize()];
			WriteBytesUnsafeToGPU(Vec2Int::Zero(), m_info.m_TexelSize, emptyDataPtr);

			if (!hasCpuBuffer) delete[] emptyDataPtr;
		}
	}

	bool Texture::IsValid() const
	{
		if (m_info.m_BufferType == TextureBufferType::CPU)
			return HasCPUBuffer() && !HasGPUBuffer();
		else if (m_info.m_BufferType == TextureBufferType::GPU)
			return !HasCPUBuffer() && HasGPUBuffer();
		else if (m_info.m_BufferType == TextureBufferType::GPUThreadSafeRead)
			return HasCPUBuffer() && HasGPUBuffer();

		LogError(std::format("Attempted to check texture validity:{} but the buffer location type has no actions", ToString()));
		return false;
	}
	bool Texture::HasEmptyData() const
	{
		std::byte* dataPtr = nullptr;
		const bool hasCpuBuffer = HasCPUBuffer();
		const size_t totalByteSize = CalculateTotalByteSize();

		if (hasCpuBuffer) dataPtr = m_cpuPtr;
		else
		{
			dataPtr = new std::byte[totalByteSize];
			ReadBytesUnsafeFromGPU(Vec2Int::Zero(), m_info.m_TexelSize, dataPtr);
		}

		const bool isEmpty = IsEmptyTexture(dataPtr, totalByteSize);
		if (!hasCpuBuffer && dataPtr != nullptr) delete[] dataPtr;
		return isEmpty;
	}
	bool Texture::IsHDRTexture() const { return IsHDRStorage(m_info.m_InternalStorage); }
	bool Texture::HasCPUBuffer() const { return m_cpuPtr != nullptr; }
	bool Texture::HasGPUBuffer() const { return m_info.m_Id != INVALID_OBJ_ID; }

	size_t Texture::GetTexelByteSize(const TexelChannelFormat format, const TexelChannelDataType dataType) const
	{
		return GetChannelCount(format) * GetChannelByteSize(dataType);
	}
	size_t Texture::GetTexelByteSize(const TexelStorageType storage) const
	{
		return GetChannelCount(storage) * GetChannelByteSize(storage);
	}
	size_t Texture::GetTexelByteSize() const { return GetTexelByteSize(m_info.m_InternalStorage); }
	std::uint32_t Texture::CalculateTotalTexels() const { return m_info.m_TexelSize.m_X * m_info.m_TexelSize.m_Y; }
	size_t Texture::CalculateTotalByteSize(const Vec2Int texels) const { return texels.m_X * texels.m_Y * GetTexelByteSize(); }
	size_t Texture::CalculateTotalByteSize(const Vec2Int texels, const TexelChannelFormat format, const TexelChannelDataType dataType) const
	{
		return texels.m_X * texels.m_Y * GetTexelByteSize(format, dataType);
	}
	size_t Texture::CalculateTotalByteSize() const { return CalculateTotalByteSize(m_info.m_TexelSize); }

	const std::byte* Texture::GetCPUMemPtr() const { return m_cpuPtr; }
	const TextureInfo& Texture::GetInfo() const { return m_info; }
	RenderObjectId Texture::GetId() const { return m_info.m_Id; }
	TexelStorageType Texture::GetStorageType() const { return m_info.m_InternalStorage; }

	void Texture::OverrideBufferType(const TextureBufferType type)
	{
		ENGINE_ASSERT(IsValid(), "Attempted to override buffer type for invalid texture:{}", ToString());
		if (type == m_info.m_BufferType)
			return;

		//If we have CPU ONLY buffer and we go to:
		// -> GPU ONLY: we allocate space on GPU, write bytes from cpu pointer to GPU and delete CPU buffer
		// -> GPU+CPU: same as GPU only, but we still keep the CPU buffer
		if (m_info.m_BufferType == TextureBufferType::CPU)
		{
			ENGINE_ASSERT(m_gpuCallbacks.IsValid(), "Attempted to override CPU buffer type into a type which requires GPU buffer, "
				"but GPU callback func pointers are NULL! Use the Rendering API CreateTexture() function instead of constructor to ensure valid callbacks");

			AllocateUnsafeToGPU();
			WriteBytesUnsafeToGPU(Vec2Int::Zero(), m_info.m_TexelSize, m_cpuPtr);

			if (type == TextureBufferType::GPU)
				DeallocateUnsafeFromCPU();
		}
		//If we have GPU ONLY buffer and we go to:
		// -> CPU ONLY: we allocate space on CPU, read bytes from GPU to CPU to GPU and delete GPU buffer
		// -> GPU+CPU: same as CPU only, but we still keep the GPU buffer
		else if (m_info.m_BufferType == TextureBufferType::GPU)
		{
			AllocateUnsafeToCPU();
			ReadBytesUnsafeFromGPU(Vec2Int::Zero(), m_info.m_TexelSize, m_cpuPtr);

			if (type == TextureBufferType::CPU)
				DeallocateUnsafeFromGPU();
		}
		//If we have CPU + GPU buffer and we go to:
		// -> CPU ONLY: we keep CPU and delete GPU buffer
		// -> GPU ONLY: we keep GPU and delete CPU buffer
		else if (m_info.m_BufferType == TextureBufferType::GPUThreadSafeRead)
		{
			if (type == TextureBufferType::CPU)
				DeallocateUnsafeFromGPU();
			else if (type == TextureBufferType::GPU)
				DeallocateUnsafeFromCPU();
		}
		else
		{
			LogError(std::format("Attempted to override texture:{} buffer type but type has no actions", ToString()));
		}
	}
	void Texture::SetWrapBehavior(const AxesWrapBehavior behavior)
	{
		ENGINE_ASSERT(IsValid(), "Attempted to set texture wrap behavior on Invalid texture:{}", ToString());
		if (HasCPUBuffer())
		{
			//TODO: implement
			LogError("NOT SUPPORTED");
		}
		else
		{
			m_gpuCallbacks.m_SetWrapBehavior(m_info.m_Id, behavior);
			m_info.m_WrapBehavior = behavior;
		}
	}
	void Texture::SetMinFilter(const MinFilter filter)
	{
		ENGINE_ASSERT(IsValid(), "Attempted to set texture min filter on Invalid texture:{}", ToString());
		if (HasCPUBuffer())
		{
			//TODO: implement
			LogError("NOT SUPPORTED");
		}
		else
		{
			m_info.m_MinFilter = filter;
			m_gpuCallbacks.m_SetMinFilter(m_info.m_Id, filter);
		}
	}
	void Texture::SetMagFilter(const MagFilter filter)
	{
		ENGINE_ASSERT(IsValid(), "Attempted to set texture mag filter on Invalid texture:{}", ToString());

		if (HasCPUBuffer())
		{
			//TODO: implement
			LogError("NOT SUPPORTED");
		}
		else
		{
			m_info.m_MagFilter = filter;
			m_gpuCallbacks.m_SetMagFilter(m_info.m_Id, filter);
		}
	}

	void Texture::ReadBytesUnsafeFromCPU(const Vec2Int texelOffset, const Vec2Int texelSize,
		std::byte* writeLocationPointer, const std::optional<TexelChannelDataType>& overrideSourceDataType) const
	{
		ENGINE_ASSERT(!texelOffset.AnyAxisLessThan(Vec2Int::Zero()) && !texelSize.AnyAxisLessThan(Vec2Int::One()), 
			"Attempted to read bytes unsafe from CPU but either texel offset:{} is negative and/or texel size:{} is less than 1:{}", 
			texelOffset.ToString(), texelSize.ToString());

		if (overrideSourceDataType == std::nullopt)
		{
			const size_t byteOffset = CalculateTexelByteOffset(texelOffset);
			memcpy(writeLocationPointer, m_cpuPtr + byteOffset, CalculateTotalByteSize(texelSize));
		}
		else
		{
			//TODO: implement
			LogError("NOT SUPPORTED");
		}
	}
	void Texture::ReadBytesUnsafeFromGPU(const Vec2Int texelOffset, const Vec2Int texelSize, 
		std::byte* writeLocationPointer, const std::optional<TexelChannelDataType>& overrideSourceDataType) const
	{
		const TexelChannelFormat channelFormat = GetChannelFormatFromStorage(m_info.m_InternalStorage);
		const TexelChannelDataType channelDataType = overrideSourceDataType.value_or(GetChannelDataTypeFromStorage(m_info.m_InternalStorage));
		m_gpuCallbacks.m_ReadBytes(m_info.m_Id, texelOffset, texelSize, channelFormat, channelDataType,
			writeLocationPointer, CalculateTotalByteSize(texelSize, channelFormat, channelDataType));
	}
	void Texture::ReadBytesUnsafe(const Vec2Int texelOffset, const Vec2Int texelSize, 
		std::byte* writeLocationPointer, const std::optional<TexelChannelDataType>& overrideSourceDataType) const
	{
		ENGINE_ASSERT(!texelOffset.AnyAxisLessThan(Vec2Int::Zero()) && !texelSize.AnyAxisLessThan(Vec2Int::One()),
			"Attempted to read bytes unsafe but either texel offset:{} is negative and/or texel size:{} is less than 1:{}",

			texelOffset.ToString(), texelSize.ToString());
		//If we have GPU thread safe read, since we should have CPU buffer, we use that
		//because we always prefer CPU read over GPU since it is faster
		if (HasCPUBuffer()) ReadBytesUnsafeFromCPU(texelOffset, texelSize, writeLocationPointer, overrideSourceDataType);
		else ReadBytesUnsafeFromGPU(texelOffset, texelSize, writeLocationPointer, overrideSourceDataType);
	}
	void Texture::ReadBytes(const Vec2Int texelOffset, const Vec2Int texelSize, std::byte* writeLocationPointer,
		const std::optional<TexelChannelDataType>& overrideSourceDataType) const
	{
		ENGINE_ASSERT(writeLocationPointer != nullptr, "Attempted to read bytes from texture to null write pointer");
		ENGINE_ASSERT(IsValid(), "Attempted to read bytes from invalid texture: {}", ToString());

		const Vec2Int maxOffset = texelOffset + texelSize;
		ENGINE_ASSERT(maxOffset.m_X <= m_info.m_TexelSize.m_X && maxOffset.m_Y <= m_info.m_TexelSize.m_Y, 
			"Attempted to READ BYTES of texture: {} but offset and size: {} > texture size:{}",
			ToString(), maxOffset.ToString(), m_info.m_TexelSize.ToString());

		ReadBytesUnsafe(texelOffset, texelSize, writeLocationPointer, overrideSourceDataType);
	}
	void Texture::ReadBytes(std::byte* writeLocationPointer) const
	{
		ENGINE_ASSERT(writeLocationPointer != nullptr, "Attempted to read bytes from texture to null write pointer");
		ENGINE_ASSERT(IsValid(), "Attempted to read bytes from invalid texture: {}", ToString());
		ReadBytesUnsafe(Vec2Int::Zero(), m_info.m_TexelSize, writeLocationPointer, std::nullopt);
	}

	Color Texture::SampleAtTexel(const Vec2Int& texel) const
	{
		ENGINE_ASSERT(m_info.m_InternalStorage != TexelStorageType::RGB16F && m_info.m_InternalStorage != TexelStorageType::RGBA16F,
			"Attempted to sample texel:{} for texture:{} but internal storage type "
			"has float components so it should use the HDR version named `SampleHDRAtUV", texel.ToString(), ToString());

		std::array<std::uint8_t, 4> channelBytes = {};
		//NOTE: the bytes we need are 1, 3 or 4, and ReadBytes will write as many as it needs so this should work for all of them
		ReadBytes(texel, Vec2Int::One(), reinterpret_cast<std::byte*>(&channelBytes));

		if (m_info.m_InternalStorage == TexelStorageType::R8)
			return Color(channelBytes[0], channelBytes[0], channelBytes[0]);

		else if (m_info.m_InternalStorage == TexelStorageType::RGB8)
			return Color(channelBytes[0], channelBytes[1], channelBytes[2]);

		else if (m_info.m_InternalStorage == TexelStorageType::RGBA8)
			return Color(channelBytes[0], channelBytes[1], channelBytes[2], channelBytes[3]);

		LogError(std::format("Attempted to sample texel:{} for texture:{} but internal storage type "
			"does not have any actions for sampling", texel.ToString(), ToString()));
		return {};
	}
	HDRColor Texture::SampleHDRAtTexel(const Vec2Int& texel) const
	{
		if (IsHDRTexture())
		{
			//NOTE: we allocate max amount we are going to need (16 bytes, 4 bytes per 4 channels) for 
			//all versions and in the case with no alpha, those bytes will be left 0 
			std::array<std::byte, 16> channelBytes = {};
			ReadBytes(texel, Vec2Int::One(), &channelBytes[0]);

			HDRColor color;
			//If we have float16 per channel, only up to 8 bytes written
			//which will be the ones used for converting to color
			if (GetChannelByteSize(m_info.m_InternalStorage) == 2)
				color = FromF16BytesToHDRColor(&channelBytes[0]);
			else color = FromF32BytesToHDRColor(&channelBytes[0]);

			if (Utils::ApproximateEqualsF(color.m_A, 0.0f))
				color.m_A = 1.0f;
			return color;
		}

		return ToHDRColor(SampleAtTexel(texel));
	}

	Color Texture::SampleAtUV(const Vec2& uv) const
	{
		const Vec2Int texelOffset = (m_info.m_TexelSize.AsFloat() * uv).AsInt();
		return SampleAtTexel(texelOffset);
	}
	HDRColor Texture::SampleHDRAtUV(const Vec2& uv) const
	{
		const Vec2Int texelOffset = (m_info.m_TexelSize.AsFloat() * uv).AsInt();
		return SampleHDRAtTexel(texelOffset);
	}

	std::string Texture::ToString() const
	{
		return std::format("[Texture Data:{}]", m_info.ToString());
	}
	std::string Texture::ToStringBytes(const bool asHex) const
	{
		std::byte* bytePtr = nullptr;
		const bool hasCpuBuffer = HasCPUBuffer();
		if (hasCpuBuffer) bytePtr = m_cpuPtr;
		else
		{
			bytePtr = new std::byte[CalculateTotalByteSize()];
			ReadBytesUnsafeFromGPU(Vec2Int::Zero(), m_info.m_TexelSize, bytePtr);
		}
		size_t byteOffset = 0;

		std::string result = "";
		const std::uint8_t colorChannelCount = GetChannelCount(m_info.m_InternalStorage);
		const std::uint8_t channelByteSize = GetChannelByteSize(m_info.m_InternalStorage);
		for (size_t y = 0; y < m_info.m_TexelSize.m_Y; y++)
		{
			for (size_t x = 0; x < m_info.m_TexelSize.m_X; x++)
			{
				if (colorChannelCount >= 1)
				{
					result += ANSI_COLOR_RED + 
						Utils::ToStringMemory(bytePtr + byteOffset, channelByteSize, asHex);
					byteOffset += channelByteSize;
				}
				if (colorChannelCount >= 2)
				{
					result += ANSI_COLOR_GREEN + 
						Utils::ToStringMemory(bytePtr + byteOffset, channelByteSize, asHex);
					byteOffset += channelByteSize;
				}
				if (colorChannelCount >= 3)
				{
					result += ANSI_COLOR_BLUE + 
						Utils::ToStringMemory(bytePtr + byteOffset, channelByteSize, asHex);
					byteOffset += channelByteSize;
				}
				if (colorChannelCount == 4)
				{
					result += ANSI_COLOR_WHITE + 
						Utils::ToStringMemory(bytePtr + byteOffset, channelByteSize, asHex);
					byteOffset += channelByteSize;
				}
			}
			result += "\n";
		}
		if (!hasCpuBuffer) delete[] bytePtr;
		return result;
	}
	Texture& Texture::operator=(Texture&& other) noexcept
	{
		if (other.HasCPUBuffer())
		{
			m_cpuPtr = other.m_cpuPtr;
			other.m_cpuPtr = nullptr;
		}
		m_gpuCallbacks = std::exchange(other.m_gpuCallbacks, {});
		m_info = std::exchange(other.m_info, {});

		return *this;
	}

	bool IsEmptyTexture(const std::byte* dataPtr, const size_t byteSize)
	{
		for (size_t i = 0; i < byteSize; i++)
		{
			if (dataPtr[i] != std::byte(0))
				return false;
		}
		return true;
	}
	Texture CreateTexture(const std::byte* data, const Vec2Int& size, const TextureBufferType type,
		const TexelStorageType storage, const AxesWrapBehavior wrap, const MinFilter min, const MagFilter mag)
	{
		//NOTE: no matter the texture buffer type, we should create wtih callbacks in case we want to switch buffer type
#if defined(OPENGL)
		return OpenGl::CreateTexture(data, size, type, storage, wrap, min, mag);
#endif
	}

	TextureCube::TextureCube() : TextureCube(Vec2Int{}) {}
	TextureCube::TextureCube(const Vec2Int& size, const TexelStorageType internalStorage,
		const AxesWrapBehavior wrap, const MinFilter min, const MagFilter mag, const TextureCubeCallbacks& callbacks)
		: m_callbacks(callbacks), m_data{ INVALID_OBJ_ID,TextureBufferType::GPU, size, internalStorage, wrap, min, mag }
	{
		if (size.m_X == 0 || size.m_Y == 0)
			return;

		Allocate();
	}
	TextureCube::~TextureCube()
	{
		Deallocate();
	}

	void TextureCube::Allocate()
	{
		Deallocate();
		m_data.m_Id = m_callbacks.m_AllocateFunc(m_data);
	}
	void TextureCube::Deallocate()
	{
		if (m_data.m_Id != INVALID_OBJ_ID)
		{
			m_callbacks.m_DeallocateFunc(m_data.m_Id);
			m_data.m_Id = INVALID_OBJ_ID;
		}
	}
	const TextureInfo& TextureCube::GetData() const { return m_data; }
	RenderObjectId TextureCube::GetId() const { return m_data.m_Id; }
	TexelStorageType TextureCube::GetStorageType() const { return m_data.m_InternalStorage; }
	void TextureCube::SetData(const TextureCubeFace face, const std::byte* data)
	{
		m_callbacks.m_SetData(face, m_data.m_Id, m_data.m_TexelSize, m_data.m_InternalStorage, data);
	}

	std::string TextureCube::ToString() const
	{
		return std::format("[TextureCube Data:{}]", m_data.ToString());
	}
	TextureCube& TextureCube::operator=(TextureCube&& other) noexcept
	{
		m_callbacks = std::exchange(other.m_callbacks, {});
		m_data = std::move(other.m_data);
		return *this;
	}

	TextureCube CreateTextureCube(const Vec2Int& size, const TexelStorageType storage, 
		const AxesWrapBehavior wrap, const MinFilter min, const MagFilter mag)
	{
#if defined(OPENGL)
		return OpenGl::CreateTextureCube(size, storage, wrap, min, mag);
#endif
	}
}