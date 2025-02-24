#include "pch.hpp"
#include "InputDirection.hpp"
#include "StringUtil.hpp"
#include "Debug.hpp"

namespace Input
{
	std::string ToString(const InputDirection& dir)
	{
		if (dir == InputDirection::Up) return "Up";
		else if (dir == InputDirection::Down) return "Down";
		else if (dir == InputDirection::Left) return "Left";
		else if (dir == InputDirection::Right) return "Right";

		LogError(std::format("Tried to convert INPUT direction to string but no actions could be found"));
		return "";
	}

	std::optional<InputDirection> TryConvertStringToDirection(const std::string& str)
	{
		std::string formattedStr = Utils::StringUtil(str).ToLowerCase().ToString();
		if (formattedStr == "up") return InputDirection::Up;
		else if (formattedStr == "down") return InputDirection::Down;
		else if (formattedStr == "left") return InputDirection::Left;
		else if (formattedStr == "right") return InputDirection::Right;

		LogError(std::format("Tried to convert invvalid string: '{}' to an input direction", str));
		return std::nullopt;
	}
}
