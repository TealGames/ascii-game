#pragma once
#include <cstdint>
#include <string>

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

enum class AngleMode : std::uint8_t
{
	Degrees	= 0,
	Radians	= 1,
};
std::string ToString(const AngleMode& mode);

enum class VectorForm : std::uint8_t
{
	Component			= 0,
	Unit				= 1,
	MagnitudeDirection	= 2,
};

std::string ToString(const VectorForm& mode);

enum class TreeTraversalType : std::uint8_t
{
	Pre0rder	= 0,
	InOrder		= 1,
	PostOrder	= 2,
};