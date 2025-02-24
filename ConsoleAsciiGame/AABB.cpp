#include "pch.hpp"
#include "AABB.hpp"
#include "HelperFunctions.hpp"
#include "Debug.hpp"

namespace Physics
{
	AABBIntersectionData::AABBIntersectionData() : 
		AABBIntersectionData(false, Vec2::ZERO) {}

	AABBIntersectionData::AABBIntersectionData(const bool& intersect, const Vec2& depth) :
		m_DoIntersect(intersect), m_Depth(depth) {}

	AABB::AABB() : AABB({}, {}) {}
	AABB::AABB(const WorldPosition& minPos, const WorldPosition& maxPos)
		: m_MinPos(minPos), m_MaxPos(maxPos) {}

	WorldPosition AABB::GetGlobalMin(const WorldPosition& centerWorldPos) const
	{
		return centerWorldPos + m_MinPos;
	}

	WorldPosition AABB::GetGlobalMax(const WorldPosition& centerWorldPos) const
	{
		return centerWorldPos + m_MaxPos;
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

	WorldPosition AABB::GetWorldPos(const WorldPosition& centerPos, const NormalizedPosition& relativePos) const
	{
		Utils::Point2D boundSize = GetSize();
		WorldPosition bottomLeftPos = centerPos - WorldPosition(boundSize.m_X / 2, boundSize.m_Y/2);
		if (relativePos.GetPos() == Utils::Point2D{0, 0}) return bottomLeftPos;

		return bottomLeftPos + WorldPosition(relativePos.GetPos().m_X * boundSize.m_X, relativePos.GetPos().m_Y * boundSize.m_Y);
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
		Utils::Point2D min1Global = entity1Bounding.GetGlobalMin(entity1Pos);
		Utils::Point2D max1Global = entity1Bounding.GetGlobalMax(entity1Pos);
		Utils::Point2D min2Global = entity2Bounding.GetGlobalMin(entity2Pos);
		Utils::Point2D max2Global = entity2Bounding.GetGlobalMax(entity2Pos);

		AABBIntersectionData result = {};
		result.m_DoIntersect = (max1Global.m_X > min2Global.m_X && min1Global.m_X < max2Global.m_X &&
								max1Global.m_Y > min2Global.m_Y && min1Global.m_Y < max2Global.m_Y);

		//Covers case where body2 is fully inside body1
		if (result.m_DoIntersect && min1Global.m_X <= min2Global.m_X && max2Global.m_X <= max1Global.m_X &&
			min1Global.m_Y <= min2Global.m_Y && max2Global.m_Y <= max1Global.m_Y)
		{
			result.m_Depth.m_X = Utils::MinAbs(min2Global.m_X - max1Global.m_X, max2Global.m_X - min1Global.m_X);
			result.m_Depth.m_Y = Utils::MinAbs(min2Global.m_Y - max1Global.m_Y, max2Global.m_Y - min1Global.m_Y);

			/*LogError(std::format("Calculating min abs X of {} and {} is: {} MIN DEPTH Y: {}", std::to_string(min2Global.m_X - min1Global.m_X), 
				std::to_string(max2Global.m_X - max1Global.m_X), std::to_string(result.m_Depth.m_X), std::to_string(result.m_Depth.m_Y)));*/
			return result;
		}

		//TODO: intersection vector depth might not work if smaller collider is fully inside bigger collider
		if (result.m_DoIntersect)
		{
			//Coming from right side
			if (max2Global.m_X > max1Global.m_X)
			{
				result.m_Depth.m_X = min2Global.m_X - max1Global.m_X;
			}
			//Coming from bottom side
			else if (min2Global.m_X < min1Global.m_X)
			{
				result.m_Depth.m_X = max2Global.m_X - min1Global.m_X;
			}

			//Coming in from top
			if (max2Global.m_Y > max1Global.m_Y) result.m_Depth.m_Y = min2Global.m_Y - max1Global.m_Y;
			//Coming from bottom side
			else if (min2Global.m_Y < min1Global.m_Y) result.m_Depth.m_Y = max2Global.m_Y - min1Global.m_Y;
		}
		return result;
	}

	Utils::Point2D GetAABBMinDisplacement(const Utils::Point2D& entity1Pos, const AABB& entity1Bounding,
		const Utils::Point2D& entity2Pos, const AABB& entity2Bounding)
	{
		Utils::Point2D min1Global = entity1Bounding.GetGlobalMin(entity1Pos);
		Utils::Point2D max1Global = entity1Bounding.GetGlobalMax(entity1Pos);
		Utils::Point2D min2Global = entity2Bounding.GetGlobalMin(entity2Pos);
		Utils::Point2D max2Global = entity2Bounding.GetGlobalMax(entity2Pos);

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

	Vec2 GetAABBDirection(const Utils::Point2D& entity1Pos, const AABB& entity1Bounding,
		const Utils::Point2D& entity2Pos, const AABB& entity2Bounding, const bool& considerCollisions)
	{
		Utils::Point2D min1Global = entity1Bounding.GetGlobalMin(entity1Pos);
		Utils::Point2D max1Global = entity1Bounding.GetGlobalMax(entity1Pos);
		Utils::Point2D min2Global = entity2Bounding.GetGlobalMin(entity2Pos);
		Utils::Point2D max2Global = entity2Bounding.GetGlobalMax(entity2Pos);


		Vec2 dir = Vec2::ZERO;
		if (considerCollisions)
		{
			//If enttiy 2 is to the right of entity1 center x
			if (entity1Pos.m_X < max2Global.m_X && entity1Pos.m_X <= min2Global.m_X) dir.m_X = 1;
			//Entity 2 is to the left of entity1 center x
			else if (min2Global.m_X < entity1Pos.m_X && max2Global.m_X <= entity1Pos.m_X) dir.m_X = -1;

			//Entity2 is on top of entity1 center y
			if (entity1Pos.m_Y < max2Global.m_Y && entity1Pos.m_Y <= min2Global.m_Y) dir.m_Y = 1;
			//Entity2 is bottom of entity1 center y
			else if (min2Global.m_Y < entity1Pos.m_Y && max2Global.m_Y <= entity1Pos.m_Y) dir.m_Y = -1;
		}
		else
		{
			//If enttiy 2 is to the right FULLY
			if (max1Global.m_X < max2Global.m_X && max1Global.m_X < min2Global.m_X) dir.m_X = 1;
			//Entity 2 is to the left of entity 1 FULLY
			else if (min2Global.m_X < min1Global.m_X && max2Global.m_X < min1Global.m_X) dir.m_X = -1;

			//Entity2 is on top of entity1 FULLY
			if (max1Global.m_Y < max2Global.m_Y && max1Global.m_Y < min2Global.m_Y) dir.m_Y = 1;
			//Entity2 is bottom of entity1 FULLY
			else if (min2Global.m_Y < min1Global.m_Y && max2Global.m_Y < min1Global.m_Y) dir.m_Y = -1;
		}
		/*if (max1Global.m_X < max2Global.m_X && max1Global.m_X <= min2Global.m_X) return Direction::Right;
		else if (min2Global.m_X < min1Global.m_X && max2Global.m_X <= min1Global.m_X) return Direction::Left;
		else if (max1Global.m_Y < max2Global.m_Y && max1Global.m_Y <= min2Global.m_Y) return Direction::Up;
		else if (min2Global.m_Y < min1Global.m_Y && max2Global.m_Y <= min1Global.m_Y) return Direction::Down;
		else
		{
			LogError(std::format("Tried to get AABB direction but no directions match any of the criteria! Entity 1:{} Entity2:{}", 
				entity1Bounding.ToString(entity1Pos), entity2Bounding.ToString(entity2Pos)));
			return std::nullopt;
		}*/

		if (Utils::ApproximateEqualsF(std::abs(dir.m_X), 1) && 
			Utils::ApproximateEqualsF(std::abs(dir.m_Y), 1))
		{
			dir.m_X = Utils::GetSign(dir.m_X) * 0.5;
			dir.m_Y = Utils::GetSign(dir.m_Y) * 0.5;
		}

		//Assert(false, std::format("aabb dir: {} normalized: {}", dir.ToString(), dir.GetNormalized().ToString()));
		return dir;
	}
}
