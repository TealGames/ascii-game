#pragma once
#include "Point2DInt.hpp" 
#include "Point2D.hpp" 
#include "CartesianPosition.hpp"
#include "WorldPosition.hpp"

namespace Physics
{
	//Axis aligned bounding box
	struct AABB
	{
		WorldPosition m_MinPos;
		WorldPosition m_MaxPos;

		AABB();
		AABB(const WorldPosition& minPos, const WorldPosition& maxPos);

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
	};

	bool DoAABBIntersect(const Utils::Point2D& entity1Pos, const AABB& entity1Bounding,
		const Utils::Point2D& entity2Pos, const AABB& entity2Bounding);
}


