#pragma once
#include <cstdint>

enum class MoveDirection : std::uint8_t
{
	North = 0,
	Northeast = 1,
	East = 2,
	Southeast = 3,
	South = 4,
	Southwest = 5,
	West = 6,
	Northwest = 7
};

enum class RotationDirection : std::uint8_t
{
	Clockwise = 0,
	CounterClockwise = 1
};