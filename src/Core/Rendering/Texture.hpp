#pragma once
#include <cstdint>
#include <array>
#include <string>
#include "Core/Rendering/RenderObject.hpp"
#include "Core/Primitives/Vector.hpp"
#include "Core/Primitives/Color.hpp"
#include "Utils/Debug.hpp"

namespace Engine::Rendering
{
	/// <summary>
	/// The format of the uploaded pixel data for a texture
	/// By default red is used for single channel.
	/// Used when updating a texture to determine how the cpu data is set up
	/// </summary>
	using TexelChannelFormatIntegralType = std::uint8_t;
	enum class TexelChannelFormat : TexelChannelFormatIntegralType
	{
		Single				= 0,
		Double				= 1,
		RGB					= 2,
		RGBA				= 3,
		Depth				= 4,
		Depth_Stencil		= 5,
	};
	std::uint8_t GetChannelCount(const TexelChannelFormat);

	enum class TexelChannelDataType : std::uint8_t
	{
		Byte		= 0,
		Float16		= 1,
		Float32		= 2,
		UInt32		= 3,
		UInt24_8	= 4
	};
	std::uint16_t GetChannelByteSize(const TexelChannelDataType);

	/// <summary>
	/// The storage types and space used for storage
	/// upon creating the texture. Ex. RGB16F -> 3 Channels (RGB) with 16F (2 bytes) for each
	/// </summary>
	using TexelStorageIntegralType = std::uint8_t;
	inline constexpr std::uint8_t INTERNAL_STORAGE_PER_FORMAT = 5;
	enum class TexelStorageType : TexelStorageIntegralType
	{
		//---------------- R CHANNEL ------------------------------
		R8					= 0,
		//---------------- RG CHANNEL -----------------------------
		RG8					= INTERNAL_STORAGE_PER_FORMAT,
		//---------------- RGB CHANNEL ----------------------------
		RGB8				= INTERNAL_STORAGE_PER_FORMAT * 2,
		RGB16F				= INTERNAL_STORAGE_PER_FORMAT * 2 + 1,
		RGB32F				= INTERNAL_STORAGE_PER_FORMAT * 2 + 2,
		//---------------- RGBA CHANNEL ---------------------------
		RGBA8				= INTERNAL_STORAGE_PER_FORMAT * 3,
		RGBA16F				= INTERNAL_STORAGE_PER_FORMAT * 3 + 1,
		RGBA32F				= INTERNAL_STORAGE_PER_FORMAT * 3 + 2,
		//---------------- DEPTH CHANNEL --------------------------
		Depth24				= INTERNAL_STORAGE_PER_FORMAT * 4,
		//------------ DEPTH + STENCIL CHANNEL --------------------
		Depth24_Stencil8	= INTERNAL_STORAGE_PER_FORMAT * 5
	};
	std::string ToString(const TexelStorageType storage);
	TexelChannelFormat GetChannelFormatFromStorage(const TexelStorageType storage);
	std::uint8_t GetChannelCount(const TexelStorageType storage);
	std::uint16_t GetChannelByteSize(const TexelStorageType storage);
	TexelChannelDataType GetChannelDataTypeFromStorage(const TexelStorageType storage);
	std::uint16_t GetStorageByteSize(const TexelStorageType storage);
	bool IsHDRStorage(const TexelStorageType storage);

	/// <summary>
	/// The type texture behavior when a texture is scaled down (minification)
	/// </summary>
	enum class MinFilter : std::uint8_t
	{
		/// <summary>
		/// Will use nearest texture pixel (texel)
		/// resulting in a pixelated look
		/// </summary>
		Nearest					= 0,
		/// <summary>
		/// Will interpolate nearby pixels
		/// resulting in a smooth, but blurry look
		/// </summary>
		Linear					= 1,

		//The following are variants of linear/nearest but also with mipmaps
		NearestMipmapNearest	= 2,
		NearestMipmapLinear		= 3,
		LinearMipmapNearest		= 4,
		LinaerMipmapLinear		= 5,
	};

	/// <summary>
	/// The type texture behavior when a texture is scaled up (magnification)
	/// </summary>
	enum class MagFilter : std::uint8_t
	{
		Nearest		= 0,
		Linear		= 1,
	};

	/// <summary>
	/// The type of behavior when texture coordinates are outside the acceptable
	/// range of [0, 1]
	/// </summary>
	enum class WrapBehavior : std::uint8_t
	{	
		/// <summary>
		/// Texture repeats infinitely in the specified direction and is 
		/// often used for repeating and tiling patterns
		/// Example: 1.3 -> 0.3
		/// </summary>
		Repeat			= 0,
		/// <summary>
		/// Similar to repeat, but every other repeat is mirrored
		/// which can be used to reduce visible seams
		/// Example: 0: repeat 1: mirrored 2:repeat 3: mirrored
		/// </summary>
		MirroredRepeat	= 1,
		/// <summary>
		/// Coordinates are clamped to the nearest edge texel
		/// which is often used to prevent seams
		/// Example 2 -> 1, 3-> 1
		/// </summary>
		ClampEdge		= 2,
		/// <summary>
		/// Coordinates are sampled from a set border color
		/// (which must be done separately) and is often used for 
		/// shadow mapping
		/// </summary>
		ClampBorder		= 3
	};
	using AxesWrapBehavior = std::array<WrapBehavior, 3>;
	AxesWrapBehavior CreateXYZWrapBehavior(const WrapBehavior xyzBehavior);

	/// <summary>
	/// The behavior that determines how UV coordinates should be computed for vertices
	/// Similar to WrapBehavior, but as WrapBehavior is on the texture side of texture behavior,
	/// TextureFitBehavior is on the calling side of the mesh which uses the texture. AS A RESULT,
	/// TEXTURING IS A CONTRACT BETWEEN WHAT THE CALLER DEMANDS AND HOW THE TEXTURE RESPONDS
	/// </summary>
	enum class TextureFitBehavior : std::uint8_t
	{
		/// <summary>
		/// The UV coords will be based on texture size and how
		/// many textures can fit within the space. 
		/// Simply: MESH SIZE / TEXTURE_SIZE (The result will determine how many times 
		/// the texture will repeat) as a float with NO ROUNDING.
		/// </summary>
		RepeatExact		= 0,
		/// <summary>
		/// Same as RepeatExact except the floating result is truncated
		/// to get perfect repeating design
		/// </summary>
		RepeatTruncate	= 1,
		/// <summary>
		/// Will fit the full texture across the mesh no matter its size
		/// </summary>
		BestFit			= 2
	};

	enum class TextureBufferType : std::uint8_t
	{
		/// <summary>
		/// Will store the texture entirely on the CPU 
		/// READ: thread safe on simultaneous texel access as long as writePtr differs on separate threads
		/// WRITE: thread unsafe on simultanous texel access
		/// </summary>
		CPU = 0,
		/// <summary>
		/// Will store the texture entirely on the GPU
		/// READ: inherently thread unsafe/undefined on simultanous texel access
		/// WRITE: inherently thread unsafe/undefined on simultanous texel access
		/// </summary>
		GPU = 1,
		/// <summary>
		/// Will store the texture BOTH on the CPU AND GPU to allow for thread safe read
		/// by keeping a copy buffer that is written to alongside the GPU in order to use for reads
		/// rather than retrieving from GPU
		/// READ: thread safe on simultaneous access as long as writePtr differs on separate threads
		/// WRITE: inherently thread unsafe/undefined on simultanous texel access
		/// </summary>
		GPUThreadSafeRead = 2
	};

	struct TextureInfo
	{
		RenderObjectId m_Id = INVALID_OBJ_ID;
		TextureBufferType m_BufferType;

		/// <summary>
		/// Total texture size in texture pixels (texels) in (WIDTH, HEIGHT)
		/// </summary>
		Vec2Int m_TexelSize = {};
		TexelStorageType m_InternalStorage;
		AxesWrapBehavior m_WrapBehavior;
		MinFilter m_MinFilter;
		MagFilter m_MagFilter;

		std::string ToString() const;
	};

	class Texture;
	struct TextureCallbacks
	{
		RenderObjectId(*m_AllocateFunc)(const TextureInfo& data);
		void(*m_WriteBytes)(const RenderObjectId, const Vec2Int offset, const Vec2Int size, 
			const TexelChannelFormat sourceFormat, const TexelChannelDataType sourceType, const std::byte*);
		void(*m_SetWrapBehavior)(const RenderObjectId, const AxesWrapBehavior);
		void (*m_SetMinFilter)(const RenderObjectId id, const MinFilter filter);
		void (*m_SetMagFilter)(const RenderObjectId id, const MagFilter filter);
		void(*m_CopyData)(const RenderObjectId, const Vec2Int size, const Texture& otherTexture);
		void(*m_ReadBytes)(const RenderObjectId, const Vec2Int offset, const Vec2Int size,
			const TexelChannelFormat sourceFormat, const TexelChannelDataType sourceType, std::byte* writePtr, const size_t bufferSize);
		void(*m_DeallocateFunc)(const RenderObjectId);

		bool IsValid() const;
	};

	constexpr TexelStorageType DEFAULT_INTERNAL_STORAGE = TexelStorageType::RGBA8;
	constexpr AxesWrapBehavior DEFAULT_AXES_WRAP = { WrapBehavior::Repeat, WrapBehavior::Repeat, WrapBehavior::Repeat };
	constexpr MinFilter DEFAULT_MIN_FILTER = MinFilter::Linear;
	constexpr MagFilter DEFAULT_MAG_FILTER = MagFilter::Linear;

	enum class TextureType : std::uint8_t
	{
		Texture		= 0,
		TextureCube	= 1,
	};

	class Texture
	{
	private:
		TextureCallbacks m_gpuCallbacks;
		TextureInfo m_info;
		std::byte* m_cpuPtr;
	public:

	private:
		size_t CalculateTexelByteOffset(const Vec2Int texel) const;

		void AllocateUnsafeToCPU();
		void AllocateUnsafeToGPU();
		void Allocate();

		void DeallocateUnsafeFromCPU();
		void DeallocateUnsafeFromGPU();
		void Deallocate();

		void WriteBytesUnsafeToCPU(const Vec2Int texelOffset, const Vec2Int texelSize, 
			const std::byte* readLocationPointer, const std::optional<TexelChannelDataType>& overrideSourceDataType = std::nullopt);
		void WriteBytesUnsafeToGPU(const Vec2Int texelOffset, const Vec2Int texelSize, 
			const std::byte* readLocationPointer, const std::optional<TexelChannelDataType>& overrideSourceDataType = std::nullopt);
		void WriteBytesUnsafe(const Vec2Int texelOffset, const Vec2Int texelSize, 
			const std::byte* readLocationPointer, const std::optional<TexelChannelDataType>& overrideSourceDataType = std::nullopt);

		void ReadBytesUnsafeFromCPU(const Vec2Int texelOffset, const Vec2Int texelSize, 
			std::byte* writeLocationPointer, const std::optional<TexelChannelDataType>& overrideSourceDataType = std::nullopt) const;
		void ReadBytesUnsafeFromGPU(const Vec2Int texelOffset, const Vec2Int texelSize, 
			std::byte* writeLocationPointer, const std::optional<TexelChannelDataType>& overrideSourceDataType = std::nullopt) const;
		void ReadBytesUnsafe(const Vec2Int texelOffset, const Vec2Int texelSize, 
			std::byte* writeLocationPointer, const std::optional<TexelChannelDataType>& overrideSourceDataType = std::nullopt) const;

	public:
		Texture();
		Texture(const std::byte* data, const Vec2Int& texelSize, const TextureBufferType textureStorage, const TexelStorageType storage= DEFAULT_INTERNAL_STORAGE,
			const AxesWrapBehavior wrap= DEFAULT_AXES_WRAP, const MinFilter min= DEFAULT_MIN_FILTER,  
			const MagFilter mag = DEFAULT_MAG_FILTER, const TextureCallbacks& callbacks = {});
		Texture(const Texture&) = delete;
		Texture(Texture&&) noexcept;
		~Texture();

		const TextureInfo& GetInfo() const;
		RenderObjectId GetId() const;
		TexelStorageType GetStorageType() const;

		bool IsValid() const;
		bool HasEmptyData() const;
		bool IsHDRTexture() const;
		bool HasCPUBuffer() const;
		bool HasGPUBuffer() const;

		size_t GetTexelByteSize(const TexelChannelFormat format, const TexelChannelDataType dataType) const;
		size_t GetTexelByteSize(const TexelStorageType storage) const;
		size_t GetTexelByteSize() const;
		std::uint32_t CalculateTotalTexels() const;
		size_t CalculateTotalByteSize(const Vec2Int texels) const;
		size_t CalculateTotalByteSize(const Vec2Int texels, const TexelChannelFormat format, const TexelChannelDataType dataType) const;
		size_t CalculateTotalByteSize() const;

		template<typename T>
		requires (!std::is_same_v<T, std::byte>)
		const T* GetCPUMemPtr() const
		{
			ENGINE_ASSERT(HasCPUBuffer(), "Attempted to Get CPU memory pointer for non-CPU texture");
			return reinterpret_cast<const T*>(m_cpuPtr);
		}
		const std::byte* GetCPUMemPtr() const;

		void OverrideBufferType(const TextureBufferType type);
		void SetWrapBehavior(const AxesWrapBehavior behavior);
		void SetMinFilter(const MinFilter filter);
		void SetMagFilter(const MagFilter filter);

		/// <summary>
		/// Reads texels at texel offset (where (0,0) is BOTTOM LEFT) 
		/// in [X,Y] and size [WIDTH, HEIGHT] and write to the data pointer
		/// </summary>
		/// <param name="texelOffset"></param>
		/// <param name="texelSize"></param>
		/// <param name="writeLocationPointer"></param>
		void ReadBytes(const Vec2Int texelOffset, const Vec2Int texelSize, std::byte* writeLocationPointer, 
			const std::optional<TexelChannelDataType>& overrideSourceDataType = std::nullopt) const;
		/// <summary>
		/// Reads texels at offset (0,0) and full texture size and write to data pointer
		/// </summary>
		/// <param name="writeLocationPointer"></param>
		void ReadBytes(std::byte* writeLocationPointer) const;

		/// <summary>
		/// Samples a singular texel at the texel where (0,0) is BOTTOM LEFT
		/// </summary>
		/// <param name="texel"></param>
		/// <returns></returns>
		Col4 SampleAtTexel(const Vec2Int& texel) const;
		ColHDR4 SampleHDRAtTexel(const Vec2Int& texel) const;
		/// <summary>
		/// Samples a singular texel at the UV coordiante where (0,0) is 
		/// BOTTOM LEFT of the texture and (1,1) is the TOP RIGHT
		/// </summary>
		/// <param name="uv"></param>
		/// <returns></returns>
		Col4 SampleAtUV(const Vec2& uv) const;
		ColHDR4 SampleHDRAtUV(const Vec2& uv) const;

		/// <summary>
		/// Reads bytes from data pointer and write to texel at offset (where (0,0) is BOTTOM LEFT)
		/// in [X,Y] and size [WIDTH, HEIGHT]
		/// </summary>
		/// <param name="texelOffset">Offset from (0,0) in texels from BOTTOM LEFT CORNER</param>
		/// <param name="texelSize">Total texel size to write from offset</param>
		/// <param name="readLocationPointer">The data pointer from which bytes are read to be written to texture</param>
		/// <param name="overrideSourceDataType">The override data type for each channel of the read location pointer</param>
		void WriteBytes(const Vec2Int texelOffset, const Vec2Int texelSize, const std::byte* readLocationPointer, 
			const std::optional<TexelChannelDataType>& overrideSourceDataType = std::nullopt);
		void WriteBytes(const std::byte* readLocationPointer);
		void WriteTexel(const Vec2Int& texel, const std::byte* readLocationPointer, 
			const std::optional<TexelChannelDataType>& overrideSourceDataType = std::nullopt);
		
		/// <summary>
		/// Copies bytes from texture argument into this texture
		/// AND ASSUMES BOTH TEXTURES HAVE SAME TEXTURE SETTINGS/MEMORY LAYOUT AND THIS TEXTURE IS VALID
		/// **This Copy() version should be used if this texture is valid and shares same format as other texture**
		/// </summary>
		/// <param name="tex"></param>
		void CopyBytes(const Texture& tex);
		/// <summary>
		/// Allocates bytes from new texture using its buffer type (if override is nullopt) or the override type
		/// into this texture regardless if this texture is empty, valid, invalid or shares memory layout
		/// **This Copy() version should be used if this texture state does not match copying texture state
		///   because unlike CopyBytes() this version deallocates existing bytes which may be expensive**
		/// </summary>
		/// <param name="tex"></param>
		/// <param name="overrideType"></param>
		void CopyTexture(const Texture& tex, const std::optional<TextureBufferType>& overrideType = std::nullopt);

		/// <summary>
		/// Will set all byte data in texture to 0.
		/// NOTE: this is expensive due to requiring a heap allocation 
		/// and should rarely be used. If you want to clear texture data efficiently
		/// preferably clear buffer bit when bdinging texture to frame buffer
		/// </summary>
		void ClearBytes();

		Texture& operator=(const Texture&) = delete;
		Texture& operator=(Texture&&) noexcept;

		std::string ToString() const;
		std::string ToStringBytes(const bool asHex = true) const;
	};

	bool IsEmptyTexture(const std::byte* dataPtr, const size_t totalByteSize);
	Texture CreateTexture(const std::byte* data, const Vec2Int& size, const TextureBufferType type,
		const TexelStorageType storage= DEFAULT_INTERNAL_STORAGE, const AxesWrapBehavior wrap= DEFAULT_AXES_WRAP,
		const MinFilter min= DEFAULT_MIN_FILTER, const MagFilter mag= DEFAULT_MAG_FILTER);
	
	using TextureCubeFaceIntegralType = std::uint8_t;
	enum class TextureCubeFace : TextureCubeFaceIntegralType
	{
		Right		=0,
		Left		=1,
		Top			=2,
		Bottom		=3,
		Front		=4,
		Back		=5
	};
	struct TextureCubeCallbacks
	{
		RenderObjectId(*m_AllocateFunc)(const TextureInfo& data);
		void(*m_SetData)(const TextureCubeFace face, const RenderObjectId, const Vec2Int size, 
			const TexelStorageType storage, const std::byte*);
		void(*m_DeallocateFunc)(const RenderObjectId);
	};

	class TextureCube
	{
	private:
		TextureCubeCallbacks m_callbacks;
		TextureInfo m_data;
	public:

	private:
		void Allocate();
		void Deallocate();
	public:
		TextureCube();
		TextureCube(const Vec2Int& size, const TexelStorageType storage = DEFAULT_INTERNAL_STORAGE,
			const AxesWrapBehavior wrap = DEFAULT_AXES_WRAP, const MinFilter min = DEFAULT_MIN_FILTER,
			const MagFilter mag = DEFAULT_MAG_FILTER, const TextureCubeCallbacks& callbacks = {});
		TextureCube(const TextureCube&) = delete;
		TextureCube(TextureCube&&) noexcept = delete;
		~TextureCube();

		const TextureInfo& GetData() const;
		RenderObjectId GetId() const;
		TexelStorageType GetStorageType() const;
		void SetData(const TextureCubeFace face, const std::byte* data);

		//void BindToSlot(const TextureSlotIndex slotIndex);
		//void UnbindFromSlot();
		//bool IsBoundToSlot() const;

		TextureCube& operator=(const TextureCube&) = delete;
		TextureCube& operator=(TextureCube&&) noexcept;

		std::string ToString() const;
	};

	TextureCube CreateTextureCube(const Vec2Int& size,
		const TexelStorageType storage = DEFAULT_INTERNAL_STORAGE, const AxesWrapBehavior wrap = DEFAULT_AXES_WRAP,
		const MinFilter min = DEFAULT_MIN_FILTER, const MagFilter mag = DEFAULT_MAG_FILTER);
}