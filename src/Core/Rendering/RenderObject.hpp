#pragma once
#include <cstdint>

namespace Rendering
{
	using RenderObjectId = std::uint32_t;
	inline constexpr RenderObjectId INVALID_OBJ_ID = 0;

	using BufferBindIndex = std::uint8_t;
	inline constexpr BufferBindIndex INVALID_BUFFER_BIND_INDEX = -1;

	enum class AccessPermissions : std::uint8_t
	{
		Read		= 0,
		Write		= 1,
		ReadWrite   = 2,
	};
}
