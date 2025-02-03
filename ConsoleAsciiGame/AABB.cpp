#include "pch.hpp"
#include "AABB.hpp"
#include "HelperFunctions.hpp"

namespace Physics
{
	AABBIntersectionData::AABBIntersectionData() : 
		AABBIntersectionData(false, Vec2::ZERO) {}

	AABBIntersectionData::AABBIntersectionData(const bool& intersect, const Vec2& depth) :
		m_DoIntersect(intersect), m_Depth(depth) {}

	AABB::AABB() : AABB({}, {}) {}
	AABB::AABB(const WorldPosition& minPos, const WorldPosition& maxPos)
		: m_MinPos(minPos), m_MaxPos(maxPos) {}

	WorldPosition AABB::GetGlobalMin(const WorldPosition& transformPos) const
	{
		return transformPos + m_MinPos;
	}

	WorldPosition AABB::GetGlobalMax(const WorldPosition& transformPos) const
	{
		return transformPos + m_MaxPos;
	}

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

	WorldPosition AABB::GetWorldPos(const WorldPosition& currentPos, const Utils::Point2D& relativePos) const
	{
		Utils::Point2D relativePosSafe = { std::clamp(relativePos.m_X, float(0), float(1)), std::clamp(relativePos.m_Y, float(0), float(1)) };
		Utils::Point2D boundSize = GetSize();

		return currentPos + WorldPosition(relativePosSafe.m_X * boundSize.m_X, relativePosSafe.m_Y * boundSize.m_Y);
	}

	std::string AABB::ToString(const WorldPosition& transformPos) const
	{
		return std::format("[Min:{} Max:{} Size: {}]", GetGlobalMin(transformPos).ToString(), 
			GetGlobalMax(transformPos).ToString(), GetSize().ToString());
	}

	bool DoAABBIntersect(const Utils::Point2D& entity1Pos, const AABB& entity1Bounding,
		const Utils::Point2D& entity2Pos, const AABB& entity2Bounding)
	{
		return GetAABBIntersectionData(entity1Pos, entity1Bounding, entity2Pos, entity2Bounding).m_DoIntersect;
	}

	AABBIntersectionData GetAABBIntersectionData(const Utils::Point2D& entity1Pos, const AABB& entity1Bounding,
		const Utils::Point2D& entity2Pos, const AABB& entity2Bounding)
	{
		//TODO is enetiy pos center or what?
		Utils::Point2D min1Global = Utils::Point2D(entity1Pos.m_X + entity1Bounding.m_MinPos.m_X, entity1Pos.m_Y + entity1Bounding.m_MinPos.m_Y);
		Utils::Point2D max1Global = Utils::Point2D(entity1Pos.m_X + entity1Bounding.m_MaxPos.m_X, entity1Pos.m_Y + entity1Bounding.m_MaxPos.m_Y);
		Utils::Point2D min2Global = Utils::Point2D(entity2Pos.m_X + entity2Bounding.m_MinPos.m_X, entity2Pos.m_Y + entity2Bounding.m_MinPos.m_Y);
		Utils::Point2D max2Global = Utils::Point2D(entity2Pos.m_X + entity2Bounding.m_MaxPos.m_X, entity2Pos.m_Y + entity2Bounding.m_MaxPos.m_Y);

		AABBIntersectionData result = {};
		result.m_DoIntersect = (max1Global.m_X > min2Global.m_X && min1Global.m_X < max2Global.m_X &&
								max1Global.m_Y > min2Global.m_Y && min1Global.m_Y < max2Global.m_Y);

		//Covers case where body2 is fully inside body1
		if (result.m_DoIntersect && min1Global.m_X <= min2Global.m_X && max2Global.m_X <= max1Global.m_X &&
			min1Global.m_Y <= min2Global.m_Y && max2Global.m_Y <= max1Global.m_Y)
		{
			result.m_Depth.m_X = Utils::MinAbs(min2Global.m_X - min1Global.m_X, max2Global.m_X - max1Global.m_X);
			result.m_Depth.m_Y = Utils::MinAbs(min2Global.m_Y - min1Global.m_Y, max2Global.m_Y - max1Global.m_Y);
			return result;
		}

		//TODO: intersection vector depth might not work if smaller collider is fully inside bigger collider
		if (result.m_DoIntersect)
		{
			//Coming in from left side
			if (max2Global.m_X < max1Global.m_X) result.m_Depth.m_X = max2Global.m_X - min1Global.m_X;
			//Coming from right side
			else if (max2Global.m_X > max1Global.m_X) result.m_Depth.m_X = min2Global.m_X - max1Global.m_X;

			//Coming from bottom side
			if (min2Global.m_Y < min1Global.m_Y) result.m_Depth.m_Y = max2Global.m_Y - min1Global.m_Y;
			//Coming from top side
			else if (min2Global.m_Y > min1Global.m_Y) result.m_Depth.m_Y = min2Global.m_Y - max1Global.m_Y;
		}
		return result;
	}

	Utils::Point2D GetAABBMinDisplacement(const Utils::Point2D& entity1Pos, const AABB& entity1Bounding,
		const Utils::Point2D& entity2Pos, const AABB& entity2Bounding)
	{
		Utils::Point2D min1Global = Utils::Point2D(entity1Pos.m_X + entity1Bounding.m_MinPos.m_X, entity1Pos.m_Y + entity1Bounding.m_MinPos.m_Y);
		Utils::Point2D max1Global = Utils::Point2D(entity1Pos.m_X + entity1Bounding.m_MaxPos.m_X, entity1Pos.m_Y + entity1Bounding.m_MaxPos.m_Y);
		Utils::Point2D min2Global = Utils::Point2D(entity2Pos.m_X + entity2Bounding.m_MinPos.m_X, entity2Pos.m_Y + entity2Bounding.m_MinPos.m_Y);
		Utils::Point2D max2Global = Utils::Point2D(entity2Pos.m_X + entity2Bounding.m_MaxPos.m_X, entity2Pos.m_Y + entity2Bounding.m_MaxPos.m_Y);

		Utils::Point2D displacement = {};

		//body 2 is fully to the RIGHT of body1
		if (max1Global.m_X < max2Global.m_X && max1Global.m_X < min2Global.m_X) displacement.m_X = min2Global.m_X - max1Global.m_X;
		//body2 is fully to the LEFT of body1
		else if (max2Global.m_X < min1Global.m_X && min2Global.m_X < min1Global.m_X) displacement.m_X = max2Global.m_X - min1Global.m_X;
		//If body2 is touching body1 or is only in vertical dir then displacement is 0
		else displacement.m_X = 0;

		// body 2 is fully ON TOP of body1
		if (max1Global.m_Y < max2Global.m_Y && max1Global.m_Y < min2Global.m_Y) displacement.m_Y = min2Global.m_Y - max1Global.m_Y;
		// body 2 is fully BELOW body1
		else if (max2Global.m_Y < min1Global.m_Y && min2Global.m_Y < min1Global.m_Y) displacement.m_Y = max2Global.m_Y - min1Global.m_Y;
		else displacement.m_Y = 0;

		return displacement;
	}

	std::optional<Direction> GetAABBDirection(const Utils::Point2D& entity1Pos, const AABB& entity1Bounding,
		const Utils::Point2D& entity2Pos, const AABB& entity2Bounding)
	{
		Utils::Point2D min1Global = Utils::Point2D(entity1Pos.m_X + entity1Bounding.m_MinPos.m_X, entity1Pos.m_Y + entity1Bounding.m_MinPos.m_Y);
		Utils::Point2D max1Global = Utils::Point2D(entity1Pos.m_X + entity1Bounding.m_MaxPos.m_X, entity1Pos.m_Y + entity1Bounding.m_MaxPos.m_Y);
		Utils::Point2D min2Global = Utils::Point2D(entity2Pos.m_X + entity2Bounding.m_MinPos.m_X, entity2Pos.m_Y + entity2Bounding.m_MinPos.m_Y);
		Utils::Point2D max2Global = Utils::Point2D(entity2Pos.m_X + entity2Bounding.m_MaxPos.m_X, entity2Pos.m_Y + entity2Bounding.m_MaxPos.m_Y);

		if (max1Global.m_X < max2Global.m_X && max1Global.m_X <= min2Global.m_X) return Direction::Right;
		else if (min2Global.m_X < min1Global.m_X && max2Global.m_X <= min1Global.m_X) return Direction::Left;
		else if (max1Global.m_Y < max2Global.m_Y && max1Global.m_Y <= min2Global.m_Y) return Direction::Up;
		else if (min2Global.m_Y < min1Global.m_Y && max2Global.m_Y <= min1Global.m_Y) return Direction::Down;
		else
		{
			Utils::LogError(std::format("Tried to get AABB direction but no directions match any of the criteria!"));
			return std::nullopt;
		}
	}
}
