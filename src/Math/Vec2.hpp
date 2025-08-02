#pragma once

#ifdef GLM
#include "glm/vec2.hpp"

namespace Math
{
	namespace Vec2Consts
	{
		inline constexpr glm::vec2 DOWN = { 0, -1 };
		inline constexpr glm::vec2 UP = { 0, 1 };
		inline constexpr glm::vec2 LEFT = { -1, 0 };
		inline constexpr glm::vec2 RIGHT = { 1, 0 };

		inline constexpr glm::vec2 NORTH = UP;
		inline constexpr glm::vec2 NORTHEAST = { 0.5f, 0.5f };
		inline constexpr glm::vec2 EAST = RIGHT;
		inline constexpr glm::vec2 SOUTHEAST = { 0.5f, -0.5f };
		inline constexpr glm::vec2 SOUTH = DOWN;
		inline constexpr glm::vec2 SOUTHWEST = { -0.5f, -0.5f };
		inline constexpr glm::vec2 WEST = LEFT;
		inline constexpr glm::vec2 NORTHWEST = { -0.5f, 0.5f };

		inline constexpr glm::vec2 ZERO = { 0, 0 };
		inline constexpr glm::vec2 ONE = { 1, 1 };
	}
}
#endif
