#pragma once
#include <cstdint>
#include <array>
#include <string>
#include "RenderObjectId.hpp"
#include "Utils/Data/Vec2Type.hpp"

namespace Rendering
{
	/// <summary>
	/// The number/type of channels used for a texture.
	/// By default red is used for single channel.
	/// </summary>
	enum class ChannelFormat : std::uint8_t
	{
		Single	= 0,
		RGB		= 1,
		RGBA	= 2,
	};

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
	using TextureSlotIndex = int;
	inline constexpr TextureSlotIndex INVALID_TEXTURE_SLOT_INDEX = -1;

	struct TextureCallbacks
	{
		RenderObjectId(*m_AllocateFunc)(const unsigned char*, const Vec2Int&, ChannelFormat, 
			AxesWrapBehavior, MinFilter, MagFilter);
		void(*m_BindFunc)(const RenderObjectId, const TextureSlotIndex);
		void(*m_UnbindFunc)(const TextureSlotIndex);
		void(*m_DeallocateFunc)(const RenderObjectId);
	};

	class Texture
	{
	private:
		TextureCallbacks m_callbacks;
		TextureSlotIndex m_slotIndex;
		RenderObjectId m_id;

		ChannelFormat m_format;
		AxesWrapBehavior m_wrapBehavior;
		MinFilter m_minFilter;
		MagFilter m_magFilter;
		Vec2Int m_size;
	public:

	private:
		void Allocate(const unsigned char* data);
		void Deallocate();
	public:
		Texture(const unsigned char* data, const Vec2Int& size, const ChannelFormat channelFormat, 
			const AxesWrapBehavior wrap, const MinFilter min, const MagFilter mag, const TextureCallbacks& callbacks);
		Texture(const Texture&) = delete;
		Texture(Texture&&) noexcept = delete;
		~Texture();

		RenderObjectId GetId() const;

		Vec2Int GetSize() const;
		int GetWidth() const;
		int GetHeight() const;
		bool IsValid() const;

		void BindToSlot(const TextureSlotIndex slotIndex);
		void UnbindFromSlot();
		bool IsBoundToSlot() const;

		Texture& operator=(const Texture&) = delete;
		Texture& operator=(Texture&&) noexcept;

		std::string ToString() const;
	};

	constexpr ChannelFormat DEFAULT_CHANNEL_FORMAT = ChannelFormat::RGBA;
	constexpr AxesWrapBehavior DEFAULT_AXES_WRAP = { WrapBehavior::Repeat, WrapBehavior::Repeat, WrapBehavior::Repeat };
	constexpr MinFilter DEFAULT_MIN_FILTER = MinFilter::Linear;
	constexpr MagFilter DEFAULT_MAG_FILTER = MagFilter::Linear;

	Texture CreateTexture(const unsigned char* data, const Vec2Int& size, 
		const ChannelFormat channelFormat= DEFAULT_CHANNEL_FORMAT, const AxesWrapBehavior wrap= DEFAULT_AXES_WRAP, 
		const MinFilter min= DEFAULT_MIN_FILTER, const MagFilter mag= DEFAULT_MAG_FILTER);
	Texture CreateTexture();
}