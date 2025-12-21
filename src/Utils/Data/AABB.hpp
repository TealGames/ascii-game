#pragma once
#include "Utils/Data/WorldPosition.hpp"
#include <string>
#include "Utils/Data/NormalizedPosition.hpp"
#include "Utils/Data/AlignmentTypes.hpp"

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
	WorldPosition2D GetWorldPos(const WorldPosition2D& centerPos, const NormalizedPosition& relativePos) const;

	std::string ToString() const;
	std::string ToString(const WorldPosition2D& transformPos) const;
};

template<AlignType Align>
struct AABB3DBase
{
	alignas(static_cast<IntegralAlignType>(Align)) WorldPosition3D m_MinPos;
	alignas(static_cast<IntegralAlignType>(Align)) WorldPosition3D m_MaxPos;

	AABB3DBase() : m_MinPos(), m_MaxPos() {}
	/// <summary>
	/// Will create a 3D AABB with min and max pos
	/// If fixZeroAxis is TRUE, will correct all axes which have a total length of 0 for in size vector
	/// by adding small epsilon to both min and max pos
	/// </summary>
	/// <param name="minPos"></param>
	/// <param name="maxPos"></param>
	/// <param name="fixZeroAxis"></param>
	AABB3DBase(const WorldPosition3D& minPos, const WorldPosition3D& maxPos, const bool fixZeroAxis = FIX_AABB_AXES_DEFAULT)
		: m_MinPos(minPos), m_MaxPos(maxPos) 
	{
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
		if (fixZeroAxis) FixZeroAxes();
	}

	static constexpr AlignType GetAlignment() { return Align; }
	WorldPosition3D GetCenter() const { return (m_MaxPos + m_MinPos) / 2; }

	void FixZeroAxes()
	{
		const Vec3 size = GetSize();
		const bool isXZero = Utils::ApproximateEqualsF(size.m_X, 0);
		const bool isYZero = Utils::ApproximateEqualsF(size.m_Y, 0);
		const bool isZZero = Utils::ApproximateEqualsF(size.m_Z, 0);
		ENGINE_ASSERT(!(isXZero && isYZero && isZZero), "Attempted to fix AABB3d with all 0 axes which is not allowed");

		if (isXZero)
		{
			const float epsilonX = std::max(AABB_EPSILON * size.m_X, AABB_EPSILON);
			m_MinPos.m_X -= epsilonX;
			m_MaxPos.m_X += epsilonX;
		}
		if (isYZero)
		{
			const float epsilonY = std::max(AABB_EPSILON * size.m_Y, AABB_EPSILON);
			m_MinPos.m_Y -= epsilonY;
			m_MaxPos.m_Y += epsilonY;
		}
		if (isZZero)
		{
			const float epsilonZ = std::max(AABB_EPSILON * size.m_Z, AABB_EPSILON);
			m_MinPos.m_Z -= epsilonZ;
			m_MaxPos.m_Z += epsilonZ;
		}
	}

	WorldPosition3D GetGlobalMin(const WorldPosition3D& centerWorldPos) const
	{
		return centerWorldPos + m_MinPos;
	}

	WorldPosition3D GetGlobalMax(const WorldPosition3D& centerWorldPos) const
	{
		return centerWorldPos + m_MaxPos;
	}

	Vec3 GetSize() const
	{
		return m_MaxPos - m_MinPos;
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
constexpr AlignType DefaultAABB3DAlign = AlignType::Align4;
using AABB3D = AABB3DBase<DefaultAABB3DAlign>;
using AABB3D_Align16 = AABB3DBase<AlignType::Align16>;

template<AlignType StartAlign, AlignType NewAlign>
AABB3DBase<NewAlign> ConvertAlignment(const AABB3DBase<StartAlign>& bounds)
{
	return AABB3DBase<NewAlign>(bounds.m_MinPos, bounds.m_MaxPos);
}

/// <summary>
/// Will find the smallest possible bounds which encompasses both bounds
/// </summary>
/// <param name="bounds1"></param>
/// <param name="bounds2"></param>
/// <returns></returns>
AABB3D UnifyBounds(const AABB3D& bounds1, const AABB3D& bounds2);


