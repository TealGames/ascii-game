#pragma once
#include "Point2DInt.hpp" 
#include "Point2D.hpp" 
#include "CartesianPosition.hpp"
#include "WorldPosition.hpp"
#include "Vec2.hpp"
#include <string>

namespace Physics
{
	struct AABBIntersectionData
	{
		bool m_DoIntersect = false;
		Vec2 m_Depth = {};

		AABBIntersectionData();
		AABBIntersectionData(const bool& intersect, const Vec2& depth);
	};

	//Axis aligned bounding box
	struct AABB
	{
		WorldPosition m_MinPos;
		WorldPosition m_MaxPos;

		AABB();
		AABB(const WorldPosition& minPos, const WorldPosition& maxPos);

		WorldPosition GetGlobalMin(const WorldPosition& centerWorldPos) const;
		WorldPosition GetGlobalMax(const WorldPosition& centerWorldPos) const;

		/// <summary>
		/// Gets the size in [WIDTH, HEIGHT]
		/// </summary>
		/// <returns></returns>
		Utils::Point2D GetSize() const;

		/// <summary>
		/// Gets the half size in [WIDTH/2, HEIGHT/2]
		/// </summary>
		/// <returns></returns>
		Utils::Point2D GetHalfExtent() const;

		std::string ToString(const WorldPosition& transformPos) const;
	};

	bool DoAABBIntersect(const Utils::Point2D& entity1Pos, const AABB& entity1Bounding,
		const Utils::Point2D& entity2Pos, const AABB& entity2Bounding);

	AABBIntersectionData GetAABBIntersectionData(const Utils::Point2D& entity1Pos, const AABB& entity1Bounding,
		const Utils::Point2D& entity2Pos, const AABB& entity2Bounding);
}


