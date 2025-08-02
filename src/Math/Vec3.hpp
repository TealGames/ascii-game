#pragma once

#ifdef GLM
#include "glm/vec3.hpp"
#include "glm/gtc/constants.hpp"
#include <glm/gtc/epsilon.hpp>
#include "Utils/Data/VectorEnums.hpp"

namespace Math
{
	namespace Vec3Consts
	{
		inline constexpr glm::vec3 DOWN = { 0, -1, 1 };
		inline constexpr glm::vec3 UP = { 0, 1, 0 };
		inline constexpr glm::vec3 LEFT = { -1, 0, 0 };
		inline constexpr glm::vec3 RIGHT = { 1, 0, 0 };
		inline constexpr glm::vec3 BACKWARD = { 0, 0, -1 };
		inline constexpr glm::vec3 FORWARD = { 0, 0, 1 };

		inline constexpr glm::vec3 ZERO = { 0, 0, 0 };
		inline constexpr glm::vec3 ONE = { 1, 1, 1 };
	}

	bool Vec3Equal(const glm::vec3& v1, const glm::vec3& v2);

#endif

