#pragma once
#include <cstdint>
#include "Utils/Data/Vec2Int.hpp"

namespace Rendering
{
	using TextureIdType = std::uint32_t;
	inline constexpr TextureIdType INVALID_TEXTURE_ID = 0;

	class Texture
	{
	private:
		TextureIdType m_id;
		Vec2Int m_size;
	public:

	private:
	public:
		Texture();
		Texture(const std::uint32_t id, const Vec2Int& size);

		Vec2Int GetSize() const;
		int GetWidth() const;
		int GetHeight() const;

		bool IsValid() const;
	};
}