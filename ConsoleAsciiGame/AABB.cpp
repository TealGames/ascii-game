#include "pch.hpp"
#include "AABB.hpp"

namespace Physics
{
	AABB::AABB() : AABB({}, {}) {}
	AABB::AABB(const WorldPosition& minPos, const WorldPosition& maxPos)
		: m_MinPos(minPos), m_MaxPos(maxPos) {}

	Utils::Point2D AABB::GetSize() const
	{
		return { m_MaxPos.m_X- m_MinPos.m_X, 
				 m_MaxPos.m_Y- m_MinPos.m_Y };
	}

	Utils::Point2D AABB::GetHalfExtent() const
	{
		Utils::Point2D size = GetSize();
		return { size.m_X/2, size.m_Y/2 };
	}

	bool DoAABBIntersect(const Utils::Point2D& entity1Pos, const AABB& entity1Bounding,
		const Utils::Point2D& entity2Pos, const AABB& entity2Bounding)
	{
		Utils::Point2D min1Global = Utils::Point2D(entity1Pos.m_X + entity1Bounding.m_MinPos.m_X, entity1Pos.m_Y + entity1Bounding.m_MinPos.m_Y);
		Utils::Point2D max1Global = Utils::Point2D(entity1Pos.m_X + entity1Bounding.m_MaxPos.m_X, entity1Pos.m_Y + entity1Bounding.m_MaxPos.m_Y);
		Utils::Point2D min2Global = Utils::Point2D(entity2Pos.m_X + entity2Bounding.m_MinPos.m_X, entity2Pos.m_Y + entity2Bounding.m_MinPos.m_Y);
		Utils::Point2D max2Global = Utils::Point2D(entity2Pos.m_X + entity2Bounding.m_MaxPos.m_X, entity2Pos.m_Y + entity2Bounding.m_MaxPos.m_Y);

		return (max1Global.m_X >= min2Global.m_X && min1Global.m_X <= max2Global.m_X &&
			max1Global.m_Y >= min2Global.m_Y && min1Global.m_Y <= max2Global.m_Y);

		return false;
	}
}
