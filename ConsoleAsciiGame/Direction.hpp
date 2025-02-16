#pragma once
#include "Vec2.hpp"
#include <optional>

enum class Direction
{
	Up,
	Down,
	Left,
	Right,
};

Vec2 GetVectorFromDirection(const Direction& dir);

std::string ToString(const Direction& dir);
std::optional<Direction> TryConvertStringToDirection(const std::string& str);