#include "Core/Rendering/Texture.hpp"
#include "Core/Analyzation/Debug.hpp"

#if defined(OPENGL)
#include "Platform/OpenGl/OpenGlTexture.hpp"
#endif

namespace Rendering
{
	Texture::Texture(const unsigned char* data, const Vec2Int& size, const ChannelFormat channelFormat, 
		const AxesWrapBehavior wrap, const MinFilter min, const MagFilter mag, const TextureCallbacks& callbacks)
		: m_callbacks(callbacks), m_id(INVALID_OBJ_ID), m_size(size), m_format(channelFormat), m_wrapBehavior(wrap), 
		m_minFilter(min), m_magFilter(mag), m_slotIndex(INVALID_TEXTURE_SLOT_INDEX)
	{
		if (data != nullptr) Allocate(data);
	}

	Texture::~Texture()
	{
		Deallocate();
	}
	void Texture::Allocate(const unsigned char* data)
	{
		Deallocate();
		m_id = m_callbacks.m_AllocateFunc(data, m_size, m_format, m_wrapBehavior, m_minFilter, m_magFilter);
	}
	void Texture::Deallocate()
	{
		if (m_id != INVALID_OBJ_ID)
		{
			m_callbacks.m_DeallocateFunc(m_id);
			m_id = INVALID_OBJ_ID;
		}
	}

	RenderObjectId Texture::GetId() const
	{
		return m_id;
	}
	TextureSlotIndex Texture::GetSlot() const
	{
		return m_slotIndex;
	}

	Vec2Int Texture::GetSize() const
	{
		return m_size;
	}
	int Texture::GetWidth() const
	{
		return m_size.m_X;
	}
	int Texture::GetHeight() const
	{
		return m_size.m_Y;
	}

	bool Texture::IsValid() const
	{
		return m_id != INVALID_OBJ_ID;
	}

	void Texture::BindToSlot(const TextureSlotIndex slotIndex)
	{
		m_callbacks.m_BindFunc(m_id, slotIndex);
		m_slotIndex = slotIndex;
		//LogWarning(std::format("texture slot now:{} for:{}", m_slotIndex, m_id));
	}
	void Texture::UnbindFromSlot()
	{
		if (m_slotIndex == INVALID_TEXTURE_SLOT_INDEX)
		{
			LogError(std::format("Attempted to unbind texture:{} from current texture "
				"slot but it is not in any slot", ToString()));
			return;
		}

		m_callbacks.m_UnbindFunc(m_slotIndex);
		m_slotIndex = INVALID_TEXTURE_SLOT_INDEX;
	}
	bool Texture::IsBoundToSlot() const
	{
		return m_slotIndex != INVALID_TEXTURE_SLOT_INDEX;
	}
	std::string Texture::ToString() const
	{
		return std::format("[Texture]");
	}
	Texture& Texture::operator=(Texture&& other) noexcept
	{
		m_callbacks = std::exchange(other.m_callbacks, {});
		m_slotIndex = std::exchange(other.m_slotIndex, INVALID_TEXTURE_SLOT_INDEX);
		//NOTE: this is the most important part because if we do default move cosntructor
		//the id does not become invalid -> deallocation called on destructor
		m_id = std::exchange(other.m_id, INVALID_OBJ_ID);

		m_format = other.m_format;
		m_wrapBehavior = other.m_wrapBehavior;
		m_minFilter = other.m_minFilter;
		m_magFilter = other.m_magFilter;
		m_size = std::exchange(other.m_size, {});
		return* this;
	}

	Texture CreateTexture(const unsigned char* data, const Vec2Int& size,
		const ChannelFormat format, const AxesWrapBehavior wrap, const MinFilter min, const MagFilter mag)
	{
#if defined(OPENGL)
		return OpenGl::CreateTexture(data, size, format, wrap, min, mag);
#endif
	}
	Texture CreateTexture()
	{
		return CreateTexture(nullptr, {});
	}
}