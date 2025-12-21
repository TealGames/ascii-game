#pragma once
#include "Utils/Data/AABB.hpp"
#include "Utils/Data/Quaternion.hpp"

namespace Utils
{
	bool RayIntersectsBoundsInverseDir(const AABB3D& bounds, const Vec3& rayOrigin, const Vec3& inverseRayDir);
	bool RayIntersectsTriangle(const Vec3& v0, const Vec3& v1, const Vec3& v2, const Vec3& rayOrigin, Vec3 rayDir, float* outHitDistance = nullptr);
	bool RayIntersectsSphere(const Vec3& sphereCenter, const float radius, const Vec3& rayOrigin, const Vec3& rayDir, float* outHitDistance = nullptr);

	float CalculateSurfaceArea(const Vec3& minPos, const Vec3& maxPos);
	float CalculateSurfaceArea(const AABB3D& bounds);

	template<AlignType Align>
	AABB3DBase<Align> ToWorldAABB(const AABB3DBase<Align>& localBounds, const Mat4& localToWorldMatrix)
	{
		return AABB3DBase<Align>((localToWorldMatrix * Vec4(localBounds.m_MinPos, 1)).GetXYZ(),
			(localToWorldMatrix * Vec4(localBounds.m_MaxPos, 1)).GetXYZ());
	}

	Mat4 CalculateTranslationMatrix(const Vec3& pos);
	Mat4 CalculateScaleMatrix(const Vec3& scale);
	Mat4 CalculateRotationMatrix(const Quat& rotation);
	Mat4 CalculateModelMatrix(const Mat4* parentMatrix, const Vec3& pos, const Vec3& scale, const Quat& rotation);

	float CalculateTransformMatrixDeterminant(const Mat4& matrix);
	float CalculateTransformMatrixDeterminant(const Vec3& matrixCol0, const Vec3& matrixCol1, const Vec3& matrixCol2);

	Vec3 ExtractTranslationFromMatrix(const Mat4& matrix);
	Quat ExtractRotationFromMatrix(const Mat4& matrix, Vec3* outScale);
	Vec3 ExtractScaleFromMatrix(const Mat4& matrix);
	void ExtractTransformFromMatrix(const Mat4& matrix, Vec3& outPosition, Quat& outRotation, Vec3& outScale);
}