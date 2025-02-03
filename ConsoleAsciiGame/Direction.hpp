#pragma once
#include "Vec2.hpp"

enum class Direction
{
	Up,
	Down,
	Left,
	Right,
};

Vec2 GetVectorFromDirection(const Direction& dir);