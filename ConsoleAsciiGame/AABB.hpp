#pragma once
#include "Point2DInt.hpp" 
#include "Point2D.hpp" 
#include "CartesianPosition.hpp"

namespace Physics
{
	//Axis aligned bounding box
	struct AABB
	{
		CartesianGridPosition m_MinPos;
		CartesianGridPosition m_MaxPos;

		AABB();
		AABB(const CartesianGridPosition& minPos, const CartesianGridPosition& maxPos);

		/// <summary>
		/// Gets the size in [WIDTH, HEIGHT]
		/// </summary>
		/// <returns></returns>
		Utils::Point2DInt GetSize() const;
	};

	bool DoAABBIntersect(const Utils::Point2D& entity1Pos, const AABB& entity1Bounding,
		const Utils::Point2D& entity2Pos, const AABB& entity2Bounding);
}


