#pragma once
#include "Vec3Type.hpp"
#include <cstdint>

//using WorldPosition = Utils::Point2D;
using WorldPosition3D = Vec3;
using WorldPosition2D = Vec2;

enum class WorldAxis : std::uint8_t
{
	X	= 0,
	Y	= 1,
	Z	= 2,
};
