#pragma once
#include <cstdint>

namespace Rendering
{
	using RenderObjectId = std::uint32_t;
	inline constexpr RenderObjectId INVALID_OBJ_ID = 0;

	using UniformBufferBindIndex = std::uint8_t;
	inline constexpr UniformBufferBindIndex INVALID_BUFFER_BIND_INDEX = -1;
}
