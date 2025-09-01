#include "Core/Rendering/Texture.hpp"
#include "Utils/Debug.hpp"

#if defined(OPENGL)
#include "Platform/OpenGl/OpenGlTexture.hpp"
#endif

namespace Rendering
{
	ChannelFormat GetChannelFormatFromStorage(const InternalStorage storage)
	{
		return static_cast<ChannelFormat>(static_cast<InternalStorageIntegralType>(storage));
	}

	std::string TextureData::ToString() const
	{
		return std::format("[TextureData]");
	}
	TextureData& TextureData::operator=(TextureData&& other) noexcept
	{
		m_slotIndex = std::exchange(other.m_slotIndex, INVALID_TEXTURE_SLOT_INDEX);
		//NOTE: this is the most important part because if we do default move cosntructor
		//the id does not become invalid -> deallocation called on destructor
		m_id = std::exchange(other.m_id, INVALID_OBJ_ID);

		m_internalStorage = other.m_internalStorage;
		m_wrapBehavior = other.m_wrapBehavior;
		m_minFilter = other.m_minFilter;
		m_magFilter = other.m_magFilter;
		m_size = std::exchange(other.m_size, {});
		return *this;
	}

	Texture::Texture() : Texture(nullptr, {}) {}
	Texture::Texture(const std::byte* data, const Vec2Int& size, const InternalStorage internalStorage,
		const AxesWrapBehavior wrap, const MinFilter min, const MagFilter mag, const TextureCallbacks& callbacks)
		: m_callbacks(callbacks), m_data{ INVALID_OBJ_ID, INVALID_TEXTURE_SLOT_INDEX,size, internalStorage, wrap, min, mag}
	{
		if (size.m_X == 0 || size.m_Y == 0)
			return;

		Allocate();
		if (data != nullptr)
			SetData(data);
	}

	Texture::~Texture()
	{
		Deallocate();
	}
	void Texture::Allocate()
	{
		Deallocate();
		m_data.m_id = m_callbacks.m_AllocateFunc(m_data);
	}
	void Texture::Deallocate()
	{
		if (m_data.m_id != INVALID_OBJ_ID)
		{
			m_callbacks.m_DeallocateFunc(m_data.m_id);
			m_data.m_id = INVALID_OBJ_ID;
		}
	}

	//RenderObjectId Texture::GetId() const { return m_id; }
	//TextureSlotIndex Texture::GetSlot() const { return m_slotIndex; }

	//Vec2Int Texture::GetSize() const { return m_size; }
	//int Texture::GetWidth() const { return m_size.m_X; }
	//int Texture::GetHeight() const { return m_size.m_Y; }
	//bool Texture::IsValid() const { return m_id != INVALID_OBJ_ID; }

	//InternalStorage Texture::GetInternalStorage() const { return m_internalStorage; }
	//AxesWrapBehavior Texture::GetAxesWrapBehavior() const { return m_wrapBehavior; }
	//MinFilter Texture::GetMinFilter() const { return m_minFilter; }
	//MagFilter Texture::GetMagFilter() const { return m_magFilter; }

	void Texture::SetData(const std::byte* data)
	{
		m_callbacks.m_SetData(m_data.m_id, m_data.m_size, m_data.m_internalStorage, data);
	}
	bool Texture::IsValid() const { return m_data.m_id != INVALID_OBJ_ID; }

	void Texture::BindToSlot(const TextureSlotIndex slotIndex)
	{
		m_callbacks.m_SetBindStatusFunc(m_data.m_id,slotIndex, true);
		m_data.m_slotIndex = slotIndex;
		//LogWarning(std::format("texture slot now:{} for:{}", m_slotIndex, m_id));
	}
	void Texture::UnbindFromSlot()
	{
		if (m_data.m_slotIndex == INVALID_TEXTURE_SLOT_INDEX)
		{
			LogError(std::format("Attempted to unbind texture:{} from current texture "
				"slot but it is not in any slot", ToString()));
			return;
		}

		m_callbacks.m_SetBindStatusFunc(m_data.m_id, m_data.m_slotIndex, false);
		m_data.m_slotIndex = INVALID_TEXTURE_SLOT_INDEX;
	}
	const TextureData& Texture::GetData() const { return m_data; }
	bool Texture::IsBoundToSlot() const
	{
		return m_data.m_slotIndex != INVALID_TEXTURE_SLOT_INDEX;
	}
	std::string Texture::ToString() const
	{
		return std::format("[Texture Data:{}]", m_data.ToString());
	}
	Texture& Texture::operator=(Texture&& other) noexcept
	{
		m_callbacks = std::exchange(other.m_callbacks, {});
		m_data = std::move(other.m_data);
		return *this;
	}

	Texture CreateTexture(const std::byte* data, const Vec2Int& size,
		const InternalStorage storage, const AxesWrapBehavior wrap, const MinFilter min, const MagFilter mag)
	{
#if defined(OPENGL)
		return OpenGl::CreateTexture(data, size, storage, wrap, min, mag);
#endif
	}

	TextureCube::TextureCube() : TextureCube(Vec2Int{}) {}
	TextureCube::TextureCube(const Vec2Int& size, const InternalStorage internalStorage,
		const AxesWrapBehavior wrap, const MinFilter min, const MagFilter mag, const TextureCubeCallbacks& callbacks)
		: m_callbacks(callbacks), m_data{ INVALID_OBJ_ID, INVALID_TEXTURE_SLOT_INDEX,size, internalStorage, wrap, min, mag }
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
		m_data.m_id = m_callbacks.m_AllocateFunc(m_data);
	}
	void TextureCube::Deallocate()
	{
		if (m_data.m_id != INVALID_OBJ_ID)
		{
			m_callbacks.m_DeallocateFunc(m_data.m_id);
			m_data.m_id = INVALID_OBJ_ID;
		}
	}
	const TextureData& TextureCube::GetData() const { return m_data; }
	void TextureCube::SetData(const TextureCubeFace face, const std::byte* data)
	{
		m_callbacks.m_SetData(face, m_data.m_id, m_data.m_size, m_data.m_internalStorage, data);
	}

	void TextureCube::BindToSlot(const TextureSlotIndex slotIndex)
	{
		m_callbacks.m_SetBindStatusFunc(m_data.m_id,slotIndex, true);
		m_data.m_slotIndex = slotIndex;
		//LogWarning(std::format("texture slot now:{} for:{}", m_slotIndex, m_id));
	}
	void TextureCube::UnbindFromSlot()
	{
		if (m_data.m_slotIndex == INVALID_TEXTURE_SLOT_INDEX)
		{
			LogError(std::format("Attempted to unbind texture:{} from current texture "
				"slot but it is not in any slot", ToString()));
			return;
		}

		m_callbacks.m_SetBindStatusFunc(m_data.m_id, m_data.m_slotIndex, false);
		m_data.m_slotIndex = INVALID_TEXTURE_SLOT_INDEX;
	}
	bool TextureCube::IsBoundToSlot() const
	{
		return m_data.m_slotIndex != INVALID_TEXTURE_SLOT_INDEX;
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

	TextureCube CreateTextureCube(const Vec2Int& size, const InternalStorage storage, 
		const AxesWrapBehavior wrap, const MinFilter min, const MagFilter mag)
	{
#if defined(OPENGL)
		return OpenGl::CreateTextureCube(size, storage, wrap, min, mag);
#endif
	}
}