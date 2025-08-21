#pragma once
#include <cstdint>
#include "Utils/Data/Vec3Type.hpp"

namespace Input
{
	enum class InputDirection : std::uint8_t
	{
		//2D DIRECTIONS (Y, X)

		Up		= 0,
		Down	= 1,
		Left	= 2,
		Right	= 3,

		//3D DIRECTIONS (Z)
		Forward = 4,
		Back	= 5,
	};
	std::string ToString(const InputDirection& dir);
	std::optional<InputDirection> TryConvertStringToDirection(const std::string& str);
	void AddDirectionToVector(Vec3Int& vec, InputDirection dir);
}


