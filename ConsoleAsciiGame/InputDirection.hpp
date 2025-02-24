#pragma once

namespace Input
{
	enum class InputDirection
	{
		Up,
		Down,
		Left,
		Right,
	};
	std::string ToString(const InputDirection& dir);
	std::optional<InputDirection> TryConvertStringToDirection(const std::string& str);
}


