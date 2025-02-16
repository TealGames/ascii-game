#include "pch.hpp"
#include "Direction.hpp"
#include "Debug.hpp"
#include "StringUtil.hpp"

Vec2 GetVectorFromDirection(const Direction& dir)
{
	if (dir == Direction::Up) return Vec2::UP;
	else if (dir == Direction::Down) return Vec2::DOWN;
	else if (dir == Direction::Right) return Vec2::RIGHT;
	else if (dir == Direction::Left) return Vec2::LEFT;

	LogError(std::format("Tried to get vector from direction but no actions could be found"));
	return {};
}

std::string ToString(const Direction& dir)
{
	if (dir == Direction::Up) return "Up";
	else if (dir == Direction::Down) return "Down";
	else if (dir == Direction::Right) return "Right";
	else if (dir == Direction::Left) return "Left";

	LogError(std::format("Tried to convert direction to string but no actions could be found"));
	return "";
}
std::optional<Direction> TryConvertStringToDirection(const std::string& str)
{
	std::string formattedStr = Utils::StringUtil(str).ToLowerCase().ToString();
	if (formattedStr == "up") return Direction::Up;
	else if (formattedStr == "down") return Direction::Down;
	else if (formattedStr == "left") return Direction::Left;
	else if (formattedStr == "right") return Direction::Right;

	return std::nullopt;
}