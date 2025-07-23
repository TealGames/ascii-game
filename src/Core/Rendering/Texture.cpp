#include "Core/Rendering/Texture.hpp"

namespace Rendering
{
	Texture::Texture() : Texture(INVALID_TEXTURE_ID, Vec2Int()) {}
	Texture::Texture(const std::uint32_t id, const Vec2Int& size): m_id(id), m_size(size) {}

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
		return m_id != INVALID_TEXTURE_ID;
	}
}