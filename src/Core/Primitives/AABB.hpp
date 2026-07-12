#pragma once
#include "Core/Primitives/WorldPosition.hpp"
#include <string>
#include "Core/Primitives/NormalizedVec2.hpp"

namespace Engine
{
	constexpr float AABB_EPSILON = 0.0001f;
	static constexpr bool FIX_AABB_AXES_DEFAULT = true;

	struct AABB2D
	{
		WorldPosition2D m_MinPos;
		WorldPosition2D m_MaxPos;

		AABB2D();
		AABB2D(const Vec2& size);
		AABB2D(const WorldPosition2D& minPos, const WorldPosition2D& maxPos);

		WorldPosition2D GetGlobalMin(const WorldPosition2D& centerWorldPos) const;
		WorldPosition2D GetGlobalMax(const WorldPosition2D& centerWorldPos) const;

		/// <summary>
		/// Gets the size in [WIDTH, HEIGHT]
		/// </summary>
		/// <returns></returns>
		Vec2 GetSize() const;

		/// <summary>
		/// Gets the half size in [WIDTH/2, HEIGHT/2]
		/// </summary>
		/// <returns></returns>
		Vec2 GetHalfExtent() const;

		/// <summary>
		/// Will get the AABB pos based on the relative pos of the AABB
		/// where (0,0) is bottom left and (1, 1) is top right
		/// </summary>
		/// <param name="relativePos"></param>
		/// <returns></returns>
		WorldPosition2D GetWorldPos(const WorldPosition2D& centerPos, const NormalizedVec2& relativePos) const;

		std::string ToString() const;
		std::string ToString(const WorldPosition2D& transformPos) const;
	};

	template<size_t ALIGN = alignof(float)>
	struct AABB3DBase
	{
		Vec<float, 3, ALIGN> m_MinPos;
		Vec<float, 3, ALIGN> m_MaxPos;

		AABB3DBase() : m_MinPos(), m_MaxPos() {}
		/// <summary>
		/// Will create a 3D AABB with min and max pos
		/// If fixZeroAxis is TRUE, will correct all axes which have a total length of 0 for in size vector
		/// by adding small epsilon to both min and max pos
		/// </summary>
		/// <param name="minPos"></param>
		/// <param name="maxPos"></param>
		/// <param name="fixZeroAxis"></param>
		template<size_t MIN_POS_ALIGN, size_t MAX_POS_ALIGN>
		AABB3DBase(const Vec<float, 3, MIN_POS_ALIGN>& minPos, const Vec<float, 3, MAX_POS_ALIGN>& maxPos, const bool fixZeroAxis = FIX_AABB_AXES_DEFAULT)
			: m_MinPos(minPos), m_MaxPos(maxPos)
		{
			// NOTE: ideally max is STRICTLY greater than min but due to floating imprecission operations
			// we allow equal and we just fix 0 or close to 0 size axes 
			ENGINE_ASSERT(maxPos >= minPos, "Attempted to create AABB3d with invalid max: {} and/or min pos:{}", maxPos.ToString(), minPos.ToString());
			if (fixZeroAxis) FixZeroAxes();
		}

		/// <summary>
		/// Will create a 3D AABB with set size (where (0, 0, 0) will be the center of the AABB)
		/// If fixZeroAxis is TRUE, will correct all axes which have a total length of 0 for in size vector
		/// by adding small epsilon to both min and max pos
		/// </summary>
		/// <param name="size"></param>
		/// <param name="fixZeroAxis"></param>
		AABB3DBase(const Vec3& size, const bool fixZeroAxis = FIX_AABB_AXES_DEFAULT) : m_MinPos(size / 2 * -1), m_MaxPos(size / 2)
		{
			ENGINE_ASSERT(!size.AnyAxisLessThan(Vec3::Zero()), "Attempted to create AABB3d with invalid negative-value size:{}", size.ToString());
			if (fixZeroAxis) FixZeroAxes();
		}

		static constexpr size_t GetAlignment() { return ALIGN; }
		Vec3 GetCenter() const { return (m_MaxPos + m_MinPos) / 2; }

		void FixZeroAxes()
		{
			const Vec3 size = GetSize();
			const bool isXZero = ::Math::ApproximateEqualsF(size.m_X, 0);
			const bool isYZero = ::Math::ApproximateEqualsF(size.m_Y, 0);
			const bool isZZero = ::Math::ApproximateEqualsF(size.m_Z, 0);
			ENGINE_ASSERT(!(isXZero && isYZero && isZZero), "Attempted to fix AABB3d with all 0 axes which is not allowed");

			if (isXZero)
			{
				m_MinPos.m_X -= AABB_EPSILON;
				m_MaxPos.m_X += AABB_EPSILON;
			}
			if (isYZero)
			{
				m_MinPos.m_Y -= AABB_EPSILON;
				m_MaxPos.m_Y += AABB_EPSILON;
			}
			if (isZZero)
			{
				m_MinPos.m_Z -= AABB_EPSILON;
				m_MaxPos.m_Z += AABB_EPSILON;
			}
		}

		Vec3 GetGlobalMin(const WorldPosition3D& centerWorldPos) const
		{
			return centerWorldPos + m_MinPos;
		}

		Vec3 GetGlobalMax(const WorldPosition3D& centerWorldPos) const
		{
			return centerWorldPos + m_MaxPos;
		}

		Vec3 GetSize() const
		{
			return Vec3(m_MaxPos - m_MinPos);
		}

		Vec3 GetHalfExtent() const
		{
			return GetSize() / 2;
		}

		std::string ToString(const WorldPosition3D& transformPos) const
		{
			return std::format("[GMin:{} GMax:{} Size: {}]", GetGlobalMin(transformPos).ToString(),
				GetGlobalMax(transformPos).ToString(), GetSize().ToString());
		}

		std::string ToString() const
		{
			return std::format("[Min:{} Max:{} Size: {}]", m_MinPos.ToString(),
				m_MaxPos.ToString(), GetSize().ToString());
		}
	};
	using AABB3D = AABB3DBase<alignof(float)>;

	template<size_t START_ALIGN, size_t NEW_ALIGN>
	AABB3DBase<NEW_ALIGN> ConvertAlignment(const AABB3DBase<START_ALIGN>& bounds)
	{
		return AABB3DBase<NEW_ALIGN>(bounds.m_MinPos, bounds.m_MaxPos);
	}

	/// <summary>
	/// Will find the smallest possible bounds which encompasses both bounds
	/// </summary>
	/// <param name="bounds1"></param>
	/// <param name="bounds2"></param>
	/// <returns></returns>
	template<size_t BOUNDS_1_ALIGN = 0, size_t BOUNDS_2_ALIGN = BOUNDS_1_ALIGN, size_t OUT_ALIGN = BOUNDS_1_ALIGN>
	AABB3DBase<OUT_ALIGN> UnifyBounds(const AABB3DBase<BOUNDS_1_ALIGN>& bounds1, const AABB3DBase<BOUNDS_2_ALIGN>& bounds2)
	{
		return AABB3DBase<OUT_ALIGN>(Math::Min(bounds1.m_MinPos, bounds2.m_MinPos), Math::Max(bounds1.m_MaxPos, bounds2.m_MaxPos));
	}
}



