#include "Utils/MathAdvanced.hpp"
#include "Utils/Math.hpp"

namespace Utils
{
	bool RayIntersectsBoundsInverseDir(const AABB3D& bounds, const Vec3& rayOrigin, const Vec3& inverseRayDir)
	{
		//NOTE: we use inverse dir since multiply is faster than divide
		Vec3 tMin = (bounds.m_MinPos - rayOrigin) * inverseRayDir;
		Vec3 tMax = (bounds.m_MaxPos - rayOrigin) * inverseRayDir;

		if (inverseRayDir.m_X < 0) std::swap(tMin.m_X, tMax.m_X);
		if (inverseRayDir.m_Y < 0) std::swap(tMin.m_Y, tMax.m_Y);
		if (inverseRayDir.m_Z < 0) std::swap(tMin.m_Z, tMax.m_Z);

		float tEnter = tMin.GetMaxComponentValue();
		float tExit = tMax.GetMinComponentValue();

		return tEnter <= tExit && tExit >= 0;
	}
	bool RayIntersectsTriangle(const Vec3& v0, const Vec3& v1, const Vec3& v2, const Vec3& rayOrigin, const Vec3& rayDir, float* outHitDistance)
	{
		Vec3 e1 = v1 - v0;
		Vec3 e2 = v2 - v0;
		Vec3 p = CrossProduct(rayDir, e2);
		float determinant = DotProduct(e1, p);
		if (fabs(determinant) < EPSILON)
			return false;

		float inverseDeterminant = 1.0f / determinant;
		Vec3 tvec = rayOrigin - v0;
		float u = DotProduct(tvec, p) * inverseDeterminant;
		if (u < 0 || u > 1)
			return false;

		Vec3 q = CrossProduct(tvec, e1);
		float v = DotProduct(rayDir, q) * inverseDeterminant;
		if (v < 0 || u + v > 1)
			return false;

		float t = DotProduct(e2, q) * inverseDeterminant;
		if (t < 0)
			return false;

		if (outHitDistance != nullptr)
			*outHitDistance = t;
		return true;
	}

	float CalculateSurfaceArea(const Vec3& minPos, const Vec3& maxPos)
	{
		const float xDiff = maxPos.m_X - minPos.m_X;
		const float yDiff = maxPos.m_Y - minPos.m_Y;
		const float zDiff = maxPos.m_Z - minPos.m_Z;
		return 2 * (xDiff * yDiff + yDiff * zDiff + xDiff * zDiff);
	}
	float CalculateSurfaceArea(const AABB3D& bounds)
	{
		return CalculateSurfaceArea(bounds.m_MinPos, bounds.m_MaxPos);
	}
}