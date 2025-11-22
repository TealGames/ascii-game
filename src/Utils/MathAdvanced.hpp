#pragma once
#include "Utils/Data/Vec3Type.hpp"
#include "Utils/Data/AABB.hpp"

namespace Utils
{
	bool RayIntersectsBoundsInverseDir(const AABB3D& bounds, const Vec3& rayOrigin, const Vec3& inverseRayDir);
	bool RayIntersectsTriangle(const Vec3& v0, const Vec3& v1, const Vec3& v2, const Vec3& rayOrigin, const Vec3& rayDir, float* outHitDistance = nullptr);

	float CalculateSurfaceArea(const Vec3& minPos, const Vec3& maxPos);
	float CalculateSurfaceArea(const AABB3D& bounds);
}