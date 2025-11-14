#include "Core/Rendering/Texture.hpp"
#include "Utils/Debug.hpp"

#if defined(OPENGL)
#include "Platform/OpenGl/OpenGlTexture.hpp"
#endif

namespace Rendering
{
	ChannelFormat GetChannelFormatFromStorage(const TexelStorageType storage)
	{
		return static_cast<ChannelFormat>(static_cast<InternalStorageIntegralType>(
			static_cast<InternalStorageIntegralType>(storage)/ INTERNAL_STORAGE_PER_FORMAT));
	}
	std::uint16_t GetStorageByteSize(const TexelStorageType storage)
	{
		if (storage == TexelStorageType::R8)
			return 1;
		else if (storage == TexelStorageType::RGB8)
			return 3;
		else if (storage == TexelStorageType::RGBA8)
			return 4;
		else if (storage == TexelStorageType::RGB16F)
			return 12;
		//NOTE: since c++ has no half floats, we use 4 channels * 4 bytes = 16 bytes
		else if (storage == TexelStorageType::RGBA16F)
			return 16;
		else if (storage == TexelStorageType::Depth24)
			return 4;
		else if (storage == TexelStorageType::Depth24_Stencil8)
			return 4;

		LogError(std::format("[OPENGL]: Attempted to convert internal storage to texel storage type"));
		return 0;
	}
	AxesWrapBehavior CreateXYZWrapBehavior(const WrapBehavior xyzBehavior)
	{
		return AxesWrapBehavior({ xyzBehavior, xyzBehavior, xyzBehavior });
	}

	std::string TextureInfo::ToString() const
	{
		return std::format("[TextureData]");
	}
	TextureInfo& TextureInfo::operator=(TextureInfo&& other) noexcept
	{
		//m_slotIndex = std::exchange(other.m_slotIndex, INVALID_TEXTURE_SLOT_INDEX);
		//NOTE: this is the most important part because if we do default move cosntructor
		//the id does not become invalid -> deallocation called on destructor
		m_id = std::exchange(other.m_id, INVALID_OBJ_ID);

		m_internalStorage = other.m_internalStorage;
		m_wrapBehavior = other.m_wrapBehavior;
		m_minFilter = other.m_minFilter;
		m_magFilter = other.m_magFilter;
		m_texelSize = std::exchange(other.m_texelSize, {});
		return *this;
	}

	Texture::Texture() : Texture(nullptr, {}) {}
	Texture::Texture(const std::byte* data, const Vec2Int& size, const TexelStorageType internalStorage,
		const AxesWrapBehavior wrap, const MinFilter min, const MagFilter mag, const TextureCallbacks& callbacks)
		: m_callbacks(callbacks), m_info{ INVALID_OBJ_ID,size, internalStorage, wrap, min, mag}
	{
		if (size.m_X == 0 || size.m_Y == 0)
			return;

		Allocate();
		if (data != nullptr)
			SetByteData(data);
	}

	Texture::~Texture()
	{
		Deallocate();
	}
	void Texture::Allocate()
	{
		Deallocate();
		m_info.m_id = m_callbacks.m_AllocateFunc(m_info);
	}
	void Texture::Deallocate()
	{
		if (m_info.m_id != INVALID_OBJ_ID)
		{
			m_callbacks.m_DeallocateFunc(m_info.m_id);
			m_info.m_id = INVALID_OBJ_ID;
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

	void Texture::SetByteData(const std::byte* data)
	{
		m_callbacks.m_SetData(m_info.m_id, m_info.m_texelSize, m_info.m_internalStorage, data);
	}
	void Texture::SetByteData(const Texture& texture)
	{
		if (GetTotalByteSize() != texture.GetTotalByteSize() || GetStorageType() != texture.GetStorageType())
		{
			LogError(std::format("Attempted to set texture:{} data from texture:{} "
				"but textures do not have matching size and/or internal storage types", ToString(), texture.ToString()));
			return;
		}

		m_callbacks.m_CopyData(m_info.m_id, m_info.m_texelSize, texture);
		/*LogWarning(std::format("Total size:{} per channel:{} size:{} calc:{}", texture.GetTotalByteSize(), 
			GetStorageByteSize(m_info.m_internalStorage), texture.GetInfo().m_texelSize.ToString(), 
			(texture.GetInfo().m_texelSize * GetStorageByteSize(m_info.m_internalStorage)).ToString()));
		std::byte* byteData = (std::byte*)alloca(texture.GetTotalByteSize());
		texture.GetByteData(byteData);
		SetByteData(byteData);*/
	}
	bool Texture::IsValid() const { return m_info.m_id != INVALID_OBJ_ID; }
	std::uint32_t Texture::GetTotalTexels() const { return m_info.m_texelSize.m_X * m_info.m_texelSize.m_Y; }
	size_t Texture::GetTotalByteSize(const Vec2Int texels) const
	{
		return texels.m_X * texels.m_Y * GetStorageByteSize(GetStorageType());
	}
	size_t Texture::GetTotalByteSize() const { return GetTotalByteSize(m_info.m_texelSize); }

	//void Texture::BindToSlot(const TextureSlotIndex slotIndex)
	//{
	//	m_callbacks.m_SetBindStatusFunc(m_data.m_id,slotIndex, true);
	//	m_data.m_slotIndex = slotIndex;
	//	//LogWarning(std::format("texture slot now:{} for:{}", m_slotIndex, m_id));
	//}
	//void Texture::UnbindFromSlot()
	//{
	//	if (m_data.m_slotIndex == INVALID_TEXTURE_SLOT_INDEX)
	//	{
	//		LogError(std::format("Attempted to unbind texture:{} from current texture "
	//			"slot but it is not in any slot", ToString()));
	//		return;
	//	}

	//	m_callbacks.m_SetBindStatusFunc(m_data.m_id, m_data.m_slotIndex, false);
	//	m_data.m_slotIndex = INVALID_TEXTURE_SLOT_INDEX;
	//}

	const TextureInfo& Texture::GetInfo() const { return m_info; }
	RenderObjectId Texture::GetId() const { return m_info.m_id; }
	TexelStorageType Texture::GetStorageType() const { return m_info.m_internalStorage; }
	void Texture::GetByteData(const Vec2Int textureOffset, const Vec2Int size, std::byte* writeLocationPointer) const
	{
		const Vec2Int maxOffset = textureOffset + size;
		if (maxOffset.m_X > m_info.m_texelSize.m_X || maxOffset.m_Y > m_info.m_texelSize.m_Y)
		{
			LogError(std::format("Attempted to get byte data of texture: {} but offset and size: {} > texture size:{}", 
				ToString(), maxOffset.ToString(), m_info.m_texelSize.ToString()));
			return;
		}

		m_callbacks.m_GetData(m_info.m_id, textureOffset, size, m_info.m_internalStorage, 
			writeLocationPointer, GetTotalByteSize(size));
	}
	void Texture::GetByteData(std::byte* writeLocationPointer) const
	{
		m_callbacks.m_GetData(m_info.m_id, Vec2Int::Zero(), m_info.m_texelSize, 
			m_info.m_internalStorage, writeLocationPointer, GetTotalByteSize());
	}
	//bool Texture::IsBoundToSlot() const
	//{
	//	return m_data.m_slotIndex != INVALID_TEXTURE_SLOT_INDEX;
	//}
	std::string Texture::ToString() const
	{
		return std::format("[Texture Data:{}]", m_info.ToString());
	}
	Texture& Texture::operator=(Texture&& other) noexcept
	{
		m_callbacks = std::exchange(other.m_callbacks, {});
		m_info = std::move(other.m_info);
		return *this;
	}

	Texture CreateTexture(const std::byte* data, const Vec2Int& size,
		const TexelStorageType storage, const AxesWrapBehavior wrap, const MinFilter min, const MagFilter mag)
	{
#if defined(OPENGL)
		return OpenGl::CreateTexture(data, size, storage, wrap, min, mag);
#endif
	}

	TextureCube::TextureCube() : TextureCube(Vec2Int{}) {}
	TextureCube::TextureCube(const Vec2Int& size, const TexelStorageType internalStorage,
		const AxesWrapBehavior wrap, const MinFilter min, const MagFilter mag, const TextureCubeCallbacks& callbacks)
		: m_callbacks(callbacks), m_data{ INVALID_OBJ_ID,size, internalStorage, wrap, min, mag }
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
	const TextureInfo& TextureCube::GetData() const { return m_data; }
	RenderObjectId TextureCube::GetId() const { return m_data.m_id; }
	TexelStorageType TextureCube::GetStorageType() const { return m_data.m_internalStorage; }
	void TextureCube::SetData(const TextureCubeFace face, const std::byte* data)
	{
		m_callbacks.m_SetData(face, m_data.m_id, m_data.m_texelSize, m_data.m_internalStorage, data);
	}

	//void TextureCube::BindToSlot(const TextureSlotIndex slotIndex)
	//{
	//	m_callbacks.m_SetBindStatusFunc(m_data.m_id,slotIndex, true);
	//	m_data.m_slotIndex = slotIndex;
	//	//LogWarning(std::format("texture slot now:{} for:{}", m_slotIndex, m_id));
	//}
	//void TextureCube::UnbindFromSlot()
	//{
	//	if (m_data.m_slotIndex == INVALID_TEXTURE_SLOT_INDEX)
	//	{
	//		LogError(std::format("Attempted to unbind texture:{} from current texture "
	//			"slot but it is not in any slot", ToString()));
	//		return;
	//	}

	//	m_callbacks.m_SetBindStatusFunc(m_data.m_id, m_data.m_slotIndex, false);
	//	m_data.m_slotIndex = INVALID_TEXTURE_SLOT_INDEX;
	//}
	//bool TextureCube::IsBoundToSlot() const
	//{
	//	return m_data.m_slotIndex != INVALID_TEXTURE_SLOT_INDEX;
	//}
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