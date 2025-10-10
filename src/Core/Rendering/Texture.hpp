#pragma once
#include <cstdint>
#include <array>
#include <string>
#include "RenderObjectId.hpp"
#include "Utils/Data/Vec2Type.hpp"

namespace Rendering
{
	/// <summary>
	/// The format of the uploaded pixel data for a texture
	/// By default red is used for single channel.
	/// Used when updating a texture to determine how the cpu data is set up
	/// </summary>
	using ChannelFormatIntegralType = std::uint8_t;
	enum class ChannelFormat : ChannelFormatIntegralType
	{
		Single				= 0,
		RGB					= 1,
		RGBA				= 2,
		Depth				= 3,
		Depth_Stencil		= 4,
	};

	/// <summary>
	/// The storage types and space used for storage
	/// upon creating the texture
	/// </summary>
	using InternalStorageIntegralType = std::uint8_t;
	inline constexpr std::uint8_t INTERNAL_STORAGE_PER_FORMAT = 5;
	enum class AttachmentStorage : InternalStorageIntegralType
	{
		R8					= 0,
		RGB8				= INTERNAL_STORAGE_PER_FORMAT,
		/// <summary>
		/// RGBA each channel with 1 byte integer precision
		/// </summary>
		RGBA8				= INTERNAL_STORAGE_PER_FORMAT*2,
		/// <summary>
		/// RGBA each channel 2 byte/16 bit float precision 
		/// </summary>
		RGBA16F				= INTERNAL_STORAGE_PER_FORMAT*2 + 1,
		Depth24				= INTERNAL_STORAGE_PER_FORMAT*3,
		Depth24_Stencil8	= INTERNAL_STORAGE_PER_FORMAT*4
	};
	ChannelFormat GetChannelFormatFromStorage(const AttachmentStorage storage);
	std::uint16_t GetStorageByteSize(const AttachmentStorage storage);

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

	using TextureSlotIndex = int;
	inline constexpr TextureSlotIndex INVALID_TEXTURE_SLOT_INDEX = -1;

	struct TextureData
	{
		RenderObjectId m_id;
		TextureSlotIndex m_slotIndex;

		Vec2Int m_size;
		AttachmentStorage m_internalStorage;
		AxesWrapBehavior m_wrapBehavior;
		MinFilter m_minFilter;
		MagFilter m_magFilter;

		std::string ToString() const;
		TextureData& operator=(TextureData&&) noexcept;
	};

	struct TextureCallbacks
	{
		RenderObjectId(*m_AllocateFunc)(const TextureData& data);
		void(*m_SetData)(const RenderObjectId, const Vec2Int size, const AttachmentStorage storage, const std::byte*);
		void(*m_GetData)(const RenderObjectId, const Vec2Int offset, const Vec2Int size, const AttachmentStorage storage, 
			std::byte* writePtr, const size_t bufferSize);
		void(*m_SetBindStatusFunc)(const RenderObjectId, const TextureSlotIndex, const bool status);
		void(*m_DeallocateFunc)(const RenderObjectId);
	};

	constexpr AttachmentStorage DEFAULT_INTERNAL_STORAGE = AttachmentStorage::RGBA8;
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
		TextureCallbacks m_callbacks;
		TextureData m_data;
	public:

	private:
		void Allocate();
		void Deallocate();

		size_t GetByteSize(const std::uint32_t texels) const;
	public:
		Texture();
		Texture(const std::byte* data, const Vec2Int& size, const AttachmentStorage storage= DEFAULT_INTERNAL_STORAGE,
			const AxesWrapBehavior wrap= DEFAULT_AXES_WRAP, const MinFilter min= DEFAULT_MIN_FILTER,  
			const MagFilter mag = DEFAULT_MAG_FILTER, const TextureCallbacks& callbacks = {});
		Texture(const Texture&) = delete;
		Texture(Texture&&) noexcept = delete;
		~Texture();

		const TextureData& GetData() const;
		/// <summary>
		/// Will get the byte data of the texture using the TOP LEFT CORNER as the origin (0, 0)
		/// </summary>
		/// <param name="textureOffset"></param>
		/// <param name="size"></param>
		/// <param name="writeLocationPointer"></param>
		void GetByteData(const Vec2Int textureOffset, const Vec2Int size, std::byte* writeLocationPointer) const;
		/// <summary>
		/// Will get byte data using (0,0) offset and full size
		/// </summary>
		/// <param name="writeLocationPointer"></param>
		void GetByteData(std::byte* writeLocationPointer) const;
		void SetData(const std::byte* data);
		bool IsValid() const;

		/// <summary>
		/// Will return the texture pixel width * height
		/// </summary>
		/// <returns></returns>
		std::uint32_t GetTotalTexels() const;
		size_t GetByteSize() const;

		void BindToSlot(const TextureSlotIndex slotIndex);
		void UnbindFromSlot();
		bool IsBoundToSlot() const;

		Texture& operator=(const Texture&) = delete;
		Texture& operator=(Texture&&) noexcept;

		std::string ToString() const;
	};

	Texture CreateTexture(const std::byte* data, const Vec2Int& size,
		const AttachmentStorage storage= DEFAULT_INTERNAL_STORAGE, const AxesWrapBehavior wrap= DEFAULT_AXES_WRAP,
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
		RenderObjectId(*m_AllocateFunc)(const TextureData& data);
		void(*m_SetData)(const TextureCubeFace face, const RenderObjectId, const Vec2Int size, const AttachmentStorage storage, const std::byte*);
		void(*m_SetBindStatusFunc)(const RenderObjectId, const TextureSlotIndex, const bool status);
		void(*m_DeallocateFunc)(const RenderObjectId);
	};

	class TextureCube
	{
	private:
		TextureCubeCallbacks m_callbacks;
		TextureData m_data;
	public:

	private:
		void Allocate();
		void Deallocate();
	public:
		TextureCube();
		TextureCube(const Vec2Int& size, const AttachmentStorage storage = DEFAULT_INTERNAL_STORAGE,
			const AxesWrapBehavior wrap = DEFAULT_AXES_WRAP, const MinFilter min = DEFAULT_MIN_FILTER,
			const MagFilter mag = DEFAULT_MAG_FILTER, const TextureCubeCallbacks& callbacks = {});
		TextureCube(const TextureCube&) = delete;
		TextureCube(TextureCube&&) noexcept = delete;
		~TextureCube();

		const TextureData& GetData() const;
		void SetData(const TextureCubeFace face, const std::byte* data);

		void BindToSlot(const TextureSlotIndex slotIndex);
		void UnbindFromSlot();
		bool IsBoundToSlot() const;

		TextureCube& operator=(const TextureCube&) = delete;
		TextureCube& operator=(TextureCube&&) noexcept;

		std::string ToString() const;
	};

	TextureCube CreateTextureCube(const Vec2Int& size,
		const AttachmentStorage storage = DEFAULT_INTERNAL_STORAGE, const AxesWrapBehavior wrap = DEFAULT_AXES_WRAP,
		const MinFilter min = DEFAULT_MIN_FILTER, const MagFilter mag = DEFAULT_MAG_FILTER);
}