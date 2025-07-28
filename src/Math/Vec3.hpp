#pragma once
#include "glm/vec3.hpp"
#include "glm/gtc/constants.hpp"
#include <glm/gtc/epsilon.hpp>
#include "Utils/Data/VectorEnums.hpp"

namespace Math
{
	using Vec3 = glm::vec3;
	namespace Vec3Consts
	{
		inline constexpr Vec3 DOWN = { 0, -1, 1 };
		inline constexpr Vec3 UP = { 0, 1, 0 };
		inline constexpr Vec3 LEFT = { -1, 0, 0 };
		inline constexpr Vec3 RIGHT = { 1, 0, 0 };
		inline constexpr Vec3 BACKWARD = { 0, 0, -1 };
		inline constexpr Vec3 FORWARD = { 0, 0, 1 };

		inline constexpr Vec3 ZERO = { 0, 0, 0 };
		inline constexpr Vec3 ONE = { 1, 1, 1 };
	}

	bool Vec3Equal(const Vec3& v1, const Vec3& v2);

	std::string ToString(const Vec3& vec, const std::uint8_t decimalPlaces = 5);
}

