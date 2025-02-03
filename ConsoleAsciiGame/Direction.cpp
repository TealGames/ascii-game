#include "pch.hpp"
#include "Direction.hpp"
#include "HelperFunctions.hpp"

Vec2 GetVectorFromDirection(const Direction& dir)
{
	if (dir == Direction::Up) return Vec2::UP;
	else if (dir == Direction::Down) return Vec2::DOWN;
	else if (dir == Direction::Right) return Vec2::RIGHT;
	else if (dir == Direction::Left) return Vec2::LEFT;

	Utils::LogError(std::format("Tried to get vector from direction but no actions could be found"));
	return {};
}