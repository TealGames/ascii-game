#pragma once
#include "Utils/Math.hpp"
#include "Core/Primitives/AABB.hpp"
#include "Math/Quaternion.hpp"
#include "Math/Ray.hpp"
#include "Conventions.hpp"

namespace Engine::Math
{
	Mat4 CalculatePerspectiveProjMatrix(const NdcRange ndcZRange, const ZForwardSign sign,
		const float fovY, const float aspectRatio, const float zNear, const float zFar);
	Mat4 CalculateOrthographicProjMatrix(const NdcRange ndcZRange, const ZForwardSign sign,
		const float maxWorldX, const float minWorldX, const float maxWorldY, const float minWorldY, const float zNear, const float zFar);

	template<size_t ALIGN = 0>
	bool RayIntersectsBounds(const AABB3DBase<ALIGN>& bounds, const Ray3D& ray, float* outTEnter, float* outTExit)
	{
		ENGINE_ASSERT(Math::IsUnitVector(ray.m_Dir), "Attempted to invoke RayIntersectsBounds but ray dir is not unit vector:{}", ray.m_Dir.ToString());
		Vec3 tMin, tMax;
		if (fabs(ray.m_Dir.m_X) < ::Math::EPSILON)
		{
			// Ray is parallel and outside the slab
			if (ray.m_Origin.m_X < bounds.m_MinPos.m_X || ray.m_Origin.m_X > bounds.m_MaxPos.m_X)
				return false;
			tMin.m_X = -std::numeric_limits<float>::infinity();
			tMax.m_X = std::numeric_limits<float>::infinity();
		}
		else
		{
			float inv = 1.0f / ray.m_Dir.m_X;
			tMin.m_X = (bounds.m_MinPos.m_X - ray.m_Origin.m_X) * inv;
			tMax.m_X = (bounds.m_MaxPos.m_X - ray.m_Origin.m_X) * inv;
			if (tMin.m_X > tMax.m_X) std::swap(tMin.m_X, tMax.m_X);
		}

		if (fabs(ray.m_Dir.m_Y) < ::Math::EPSILON)
		{
			if (ray.m_Origin.m_Y < bounds.m_MinPos.m_Y || ray.m_Origin.m_Y > bounds.m_MaxPos.m_Y)
				return false;
			tMin.m_Y = -std::numeric_limits<float>::infinity();
			tMax.m_Y = std::numeric_limits<float>::infinity();
		}
		else
		{
			float inv = 1.0f / ray.m_Dir.m_Y;
			tMin.m_Y = (bounds.m_MinPos.m_Y - ray.m_Origin.m_Y) * inv;
			tMax.m_Y = (bounds.m_MaxPos.m_Y - ray.m_Origin.m_Y) * inv;
			if (tMin.m_Y > tMax.m_Y) std::swap(tMin.m_Y, tMax.m_Y);
		}

		if (fabs(ray.m_Dir.m_Z) < ::Math::EPSILON)
		{
			if (ray.m_Origin.m_Z < bounds.m_MinPos.m_Z || ray.m_Origin.m_Z > bounds.m_MaxPos.m_Z)
				return false;
			tMin.m_Z = -std::numeric_limits<float>::infinity();
			tMax.m_Z = std::numeric_limits<float>::infinity();
		}
		else
		{
			float inv = 1.0f / ray.m_Dir.m_Z;
			tMin.m_Z = (bounds.m_MinPos.m_Z - ray.m_Origin.m_Z) * inv;
			tMax.m_Z = (bounds.m_MaxPos.m_Z - ray.m_Origin.m_Z) * inv;
			if (tMin.m_Z > tMax.m_Z) std::swap(tMin.m_Z, tMax.m_Z);
		}

		float tEnter = tMin.GetMaxComponentValue();
		float tExit = tMax.GetMinComponentValue();

		if (outTEnter) *outTEnter = tEnter;
		if (outTExit)  *outTExit = tExit;

		return tExit >= std::max(tEnter, 0.0f);
	}
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
	template<size_t ALIGN = 0>
	bool RayIntersectsBounds(const AABB3DBase<ALIGN>& bounds, const Ray3D& ray, float* outMargin)
	{
		float tEnter = 0;
		float tExit = 0;
		bool intersects = RayIntersectsBounds<ALIGN>(bounds, ray, &tEnter, &tExit);
		if (outMargin != nullptr)
			*outMargin = tExit - tEnter;

		return intersects;
	}
	bool RayIntersectsTriangle(const Vec3& v0, const Vec3& v1, const Vec3& v2, const Ray3D& ray, float* outTEnter = nullptr);
	bool RayIntersectsTriangleInterpolated(const Vec3& v0, const Vec3& v1, const Vec3& v2,
		const Vec3& normal0, const Vec3 normal1, const Vec3 normal2, const Ray3D& ray,float* outTEnter, Vec3& outTriangleNormal);
	bool RayIntersectsSphere(const Vec3& sphereCenter, const float radius, const Ray3D& ray, float* outTEnter = nullptr);

	template<size_t ALIGN = 0>
	bool IsWithinBounds(const AABB3DBase<ALIGN>& aabb, const Vec3& pos)
	{
		if (pos.AnyAxisGreaterThan(aabb.m_MaxPos))
			return false;
		if (pos.AnyAxisLessThan(aabb.m_MinPos))
			return false;

		return true;
	}

	template<size_t ALIGN = 0>
	bool IsFullyOutsideBounds(const AABB3DBase<ALIGN>& aabb, const Vec3& pos)
	{
		if (pos > aabb.m_MinPos && pos > aabb.m_MaxPos)
			return true;
		if (pos < aabb.m_MinPos && pos < aabb.m_MaxPos)
			return true;

		return false;
	}

	float CalculateSurfaceArea(const Vec3& minPos, const Vec3& maxPos);
	template<size_t ALIGN = 0>
	float CalculateSurfaceArea(const AABB3DBase<ALIGN>& bounds)
	{
		return CalculateSurfaceArea(bounds.m_MinPos, bounds.m_MaxPos);
	}

	float CalculateParallelogramArea(const Vec3& v0, const Vec3& v1, const Vec3& v2);
	float CalculateTriangleArea(const Vec3& v0, const Vec3& v1, const Vec3& v2);
	Vec3 CalculateBarycentricWeight(const Vec3& targetPos, const Vec3& v0, const Vec3& v1, const Vec3& v2);

	template<size_t IN_ALIGN, size_t OUT_ALIGN = IN_ALIGN>
	AABB3DBase<OUT_ALIGN> ApplyMatrixToAABB(const AABB3DBase<IN_ALIGN>& localBounds, const Mat4& matrix)
	{
		WorldPosition3D newMin = (matrix * Vec4(localBounds.m_MinPos, 1.0f)).GetXYZ();
		WorldPosition3D newMax = (matrix * Vec4(localBounds.m_MaxPos, 1.0f)).GetXYZ();
		//Since some matrices may end up flipping min and max pos, 
		//we ensure they stay in the correct order
		if (newMin.m_X > newMax.m_X) std::swap(newMin.m_X, newMax.m_X);
		if (newMin.m_Y > newMax.m_Y) std::swap(newMin.m_Y, newMax.m_Y);
		if (newMin.m_Z > newMax.m_Z) std::swap(newMin.m_Z, newMax.m_Z);

		return AABB3DBase<OUT_ALIGN>(newMin, newMax);
	}

	Mat4 CalculateTranslationMatrix(const Vec3& pos);
	Mat4 CalculateScaleMatrix(const Vec3& scale);
	Mat4 CalculateRotationMatrix(const Quat& rotation);
	Mat4 CalculateRotationMatrix(const Vec3& forwardDir, const Vec3& upDir, const Vec3& rightDir);
	Mat4 CalculateModelMatrix(const Mat4* parentMatrix, const Vec3& pos, const Vec3& scale, const Quat& rotation);
	Mat4 CalculateModelMatrix(const Mat4* parentMatrix, const Vec3& pos, const Vec3& scale, const Mat4& rotation);
	Mat4 CalculateModelMatrix(const Mat4* parentMatrix, const Mat4& pos, const Mat4& scale, const Mat4& rotation);
	Mat4 CalculateInverseModelMatrix(const Mat4& matrix);

	Mat3 CalculateTranslationMatrix(const Vec2& pos);
	Mat3 CalculateScaleMatrix(const Vec2& scale);
	Mat3 CalculateUIModelMatrix(const Mat3* parentMatrix, const Vec2& topLeftPos, const Vec2& size, const Vec2& pivot);
	Mat3 CalculateUIModelMatrix(const Mat3* parentMatrix, const Mat3& pivotToOriginMoveMatrix, const Mat3& scaleMatrix, 
		const Mat3& scaledOriginToPivotMoveMatrix, const Mat3& posMatrix);

	float CalculateTransformMatrixDeterminant(const Mat4& matrix);
	float CalculateTransformMatrixDeterminant(const Vec3& matrixCol0, const Vec3& matrixCol1, const Vec3& matrixCol2);

	Vec3 ExtractTranslationFromMatrix(const Mat4& matrix);
	Quat ExtractRotationFromMatrix(const Mat4& matrix, Vec3* outScale = nullptr);
	Vec3 ExtractScaleFromMatrix(const Mat4& matrix);
	void ExtractTransformFromMatrix(const Mat4& matrix, Vec3& outPosition, Quat& outRotation, Vec3& outScale);

	/// <summary>
	/// Converts a vec3 normal to a vec2 in range [0, 1] using octahedral encoding.
	/// NOTE: this only works because normals are normalized and lie within a unit sphere
	/// NOTE: we could also use latitude/longitude encoding OR spherical coordiantes (theta, psi)
	/// but those result in high error at the poles due to discontinuities
	/// </summary>
	/// <param name="normal"></param>
	/// <returns></returns>
	Vec2 OctahedralEncodeNormal(const Vec3& normal);
	/// <summary>
	/// Converts a vec2 in range [0, 1] to normalized vec3 normal using octahedral decoding
	/// </summary>
	/// <param name="octahedral"></param>
	/// <returns></returns>
	Vec3 OctahedralDecodeNormal(const Vec2& octahedral);

	Vec3 ReflectAcrossNormal(const Vec3& incidenceVec, const Vec3& normal);
}