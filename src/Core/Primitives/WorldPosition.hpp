#pragma once
#include "Core/Primitives/Vector.hpp"
#include <cstdint>

namespace Engine
{
	//using WorldPosition = ::Utils::Point2D;
	using WorldPosition3D = Vec3;
	using WorldPosition2D = Vec2;
	using UV = Vec2;

	enum class WorldAxis : std::uint8_t
	{
		X = 0,
		Y = 1,
		Z = 2,
	};
}

