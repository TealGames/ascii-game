#pragma once
#include "glm/vec2.hpp"

namespace Math
{
	using Vec2 = glm::vec2;
	namespace Vec2Consts
	{
		inline constexpr Vec2 DOWN = { 0, -1 };
		inline constexpr Vec2 UP = { 0, 1 };
		inline constexpr Vec2 LEFT = { -1, 0 };
		inline constexpr Vec2 RIGHT = { 1, 0 };

		inline constexpr Vec2 NORTH = UP;
		inline constexpr Vec2 NORTHEAST = { 0.5f, 0.5f };
		inline constexpr Vec2 EAST = RIGHT;
		inline constexpr Vec2 SOUTHEAST = { 0.5f, -0.5f };
		inline constexpr Vec2 SOUTH = DOWN;
		inline constexpr Vec2 SOUTHWEST = { -0.5f, -0.5f };
		inline constexpr Vec2 WEST = LEFT;
		inline constexpr Vec2 NORTHWEST = { -0.5f, 0.5f };

		inline constexpr Vec2 ZERO = { 0, 0 };
		inline constexpr Vec2 ONE = { 1, 1 };
	}

	Vec2 GenerateRandomVec2(const Vec2& minVec, const Vec2 maxVec);
	Vec2 GenerateRandomDir();

	bool IsWithinBounds(const Vec2& targetPos, const Vec2& minPos, const Vec2& maxPos);
}
