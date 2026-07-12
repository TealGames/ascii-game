#pragma once
#include <cstdint>
#include <string>

enum class RotationDirection : std::uint8_t
{
	Clockwise = 0,
	CounterClockwise = 1
};

enum class AngleMode : std::uint8_t
{
	Degrees	= 0,
	Radians	= 1,
};
std::string ToString(const AngleMode& mode);

enum class TreeTraversalType : std::uint8_t
{
	Pre0rder	= 0,
	InOrder		= 1,
	PostOrder	= 2,
};