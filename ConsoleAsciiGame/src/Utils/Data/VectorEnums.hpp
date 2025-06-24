#pragma once
#include <string>

enum class AngleMode
{
	Degrees,
	Radians,
};
std::string ToString(const AngleMode& mode);

enum class VectorForm
{
	Component,
	Unit,
	MagnitudeDirection,
};

std::string ToString(const VectorForm& mode);

