#pragma once
#include <cstdint>

namespace Input
{
	enum class InputDirection : std::uint8_t
	{
		Up		= 0,
		Down	= 1,
		Left	= 2,
		Right	= 3
	};
	std::string ToString(const InputDirection& dir);
	std::optional<InputDirection> TryConvertStringToDirection(const std::string& str);
}


