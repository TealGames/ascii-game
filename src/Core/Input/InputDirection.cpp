#include "pch.hpp"
#include "Core/Input/InputDirection.hpp"
#include "Utils/StringUtil.hpp"
#include "Utils/Debug.hpp"
#include "Conventions.hpp"

namespace Engine::Input
{
	std::string ToString(const InputDirection& dir)
	{
		if (dir == InputDirection::Up) return "Up";
		else if (dir == InputDirection::Down) return "Down";
		else if (dir == InputDirection::Left) return "Left";
		else if (dir == InputDirection::Right) return "Right";
		else if (dir == InputDirection::Forward) return "Foward";
		else if (dir == InputDirection::Back) return "Back";

		LogError(std::format("Tried to convert INPUT direction to string but no actions could be found"));
		return "";
	}

	std::optional<InputDirection> TryConvertStringToDirection(const std::string& str)
	{
		std::string formattedStr = ::Utils::StringUtil(str).ToLowerCase().ToString();
		if (formattedStr == "up") return InputDirection::Up;
		else if (formattedStr == "down") return InputDirection::Down;
		else if (formattedStr == "left") return InputDirection::Left;
		else if (formattedStr == "right") return InputDirection::Right;
		else if (formattedStr == "forward") return InputDirection::Forward;
		else if (formattedStr == "back") return InputDirection::Back;

		LogError(std::format("Tried to convert invvalid string: '{}' to an input direction", str));
		return std::nullopt;
	}

	void AddDirectionToVector(Vec3Int& vec, InputDirection dir)
	{
		if (dir == InputDirection::Up) vec += ENGINE_UP_DIR.AsInt();
		else if (dir == InputDirection::Down) vec -= ENGINE_UP_DIR.AsInt();
		else if (dir == InputDirection::Right) vec += ENGINE_RIGHT_DIR.AsInt();
		else if (dir == InputDirection::Left) vec -= ENGINE_RIGHT_DIR.AsInt();
		else if (dir == InputDirection::Forward) vec += ENGINE_FORWARD_DIR.AsInt();
		else if (dir == InputDirection::Back) vec -= ENGINE_FORWARD_DIR.AsInt();
		else
		{
			LogError(std::format("Tried to add input direction:{} with no actions to vector:{}", 
				ToString(dir), vec.ToString()));
		}
	}
}
