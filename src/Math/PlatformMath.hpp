#pragma once
#include <cstdint>
#include "Core/Primitives/Matrix.hpp"

namespace Engine::Math::Platforms
{
	Mat4 CalculatePlatformPerspectiveProjMatrix(const float fovY, const float aspectRatio, const float zNear, const float zFar);
	Mat4 CalculatePlatformOrthographicProjMatrix(const float maxWorldX, const float minWorldX,
		const float maxWorldY, const float minWorldY, const float zNear, const float zFar);
}
