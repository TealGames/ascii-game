#pragma once
#include "Utils/Data/AABB.hpp"
#include "Utils/Data/Quaternion.hpp"

inline constexpr float PI = std::numbers::pi;
inline constexpr float RAD_90 = PI / 2.0f;
inline constexpr float RAD_180 = PI;
inline constexpr float RAD_270 = 3.0f * PI / 2.0f;
inline constexpr float RAD_360 = 2.0f * PI;

inline constexpr float RAD_30 = PI / 6.0f;
inline constexpr float RAD_150 = 5.0f * PI / 6.0f;
inline constexpr float RAD_210 = 7.0f * PI / 6.0f;
inline constexpr float RAD_330 = 11.0f * PI / 6.0f;

inline constexpr float RAD_60 = PI / 3.0f;
inline constexpr float RAD_120 = 2.0f * PI / 3.0f;
inline constexpr float RAD_240 = 4.0f * PI / 3.0f;
inline constexpr float RAD_300 = 5.0f * PI / 3.0f;

inline constexpr float RAD_45 = PI / 4.0f;
inline constexpr float RAD_135 = 3.0f * PI / 4.0f;
inline constexpr float RAD_225 = 5.0f * PI / 4.0f;
inline constexpr float RAD_315 = 7.0f * PI / 4.0f;

namespace Utils
{
	bool RayIntersectsBounds(const AABB3D& bounds, const Vec3& rayOrigin, const Vec3& rayDir, float* outTEnter, float* outTExit);
	/// <summary>
	/// Computes if the ray intersects the 3d bounds and returns the margin, how close the ray was to hitting the bounds
	/// AND SHOULD NOT BE USED AS SPATIAL DISTANCE (since it correlates to ray position from edge but IS NOT distance)
	/// Margin:
	///  - <0 -> ray missed
	///	 - 0  -> ray is tangent/on edge of bounds
	///  - >0 -> smaller values to zero means closer to edge
	/// </summary>
	/// <param name="bounds"></param>
	/// <param name="rayOrigin"></param>
	/// <param name="inverseRayDir"></param>
	/// <param name="outMargin"></param>
	/// <returns></returns>
	bool RayIntersectsBounds(const AABB3D& bounds, const Vec3& rayOrigin, const Vec3& rayDir, float* outTMargin);
	bool RayIntersectsTriangle(const Vec3& v0, const Vec3& v1, const Vec3& v2, const Vec3& rayOrigin, const Vec3& rayDir, float* outTEnter = nullptr);
	bool RayIntersectsSphere(const Vec3& sphereCenter, const float radius, const Vec3& rayOrigin, const Vec3& rayDir, float* outTEnter = nullptr);

	bool IsWithinBounds(const AABB3D& bounds, const Vec3& pos);
	bool IsFullyOutsideBounds(const AABB3D& bounds, const Vec3& pos);

	float CalculateSurfaceArea(const Vec3& minPos, const Vec3& maxPos);
	float CalculateSurfaceArea(const AABB3D& bounds);

	template<AlignType Align>
	AABB3DBase<Align> ApplyMatrixToAABB(const AABB3DBase<Align>& localBounds, const Mat4& matrix)
	{
		WorldPosition3D newMin = (matrix * Vec4(localBounds.m_MinPos, 1)).GetXYZ();
		WorldPosition3D newMax = (matrix * Vec4(localBounds.m_MaxPos, 1)).GetXYZ();
		//Since some matrices may end up flipping min and max pos, 
		//we ensure they stay in the correct order
		if (newMin.m_X > newMax.m_X) std::swap(newMin.m_X, newMax.m_X);
		if (newMin.m_Y > newMax.m_Y) std::swap(newMin.m_Y, newMax.m_Y);
		if (newMin.m_Z > newMax.m_Z) std::swap(newMin.m_Z, newMax.m_Z);

		return AABB3DBase<Align>(newMin, newMax);
	}

	Mat4 CalculateTranslationMatrix(const Vec3& pos);
	Mat4 CalculateScaleMatrix(const Vec3& scale);
	Mat4 CalculateRotationMatrix(const Quat& rotation);
	Mat4 CalculateRotationMatrix(const Vec3& forwardDir, const Vec3& upDir, const Vec3& rightDir);
	Mat4 CalculateModelMatrix(const Mat4* parentMatrix, const Vec3& pos, const Vec3& scale, const Quat& rotation);
	Mat4 CalculateModelMatrix(const Mat4* parentMatrix, const Vec3& pos, const Vec3& scale, const Mat4& rotation);
	Mat4 CalculateModelMatrix(const Mat4* parentMatrix, const Mat4& pos, const Mat4& scale, const Mat4& rotation);
	Mat4 CalculateInverseModelMatrix(const Mat4& matrix);

	float CalculateTransformMatrixDeterminant(const Mat4& matrix);
	float CalculateTransformMatrixDeterminant(const Vec3& matrixCol0, const Vec3& matrixCol1, const Vec3& matrixCol2);

	Vec3 ExtractTranslationFromMatrix(const Mat4& matrix);
	Quat ExtractRotationFromMatrix(const Mat4& matrix, Vec3* outScale);
	Vec3 ExtractScaleFromMatrix(const Mat4& matrix);
	void ExtractTransformFromMatrix(const Mat4& matrix, Vec3& outPosition, Quat& outRotation, Vec3& outScale);
}