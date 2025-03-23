#pragma once
#include "Vec2Int.hpp" 
#include "WorldPosition.hpp"
#include "Vec2.hpp"
#include "Direction.hpp"
#include <string>
#include <optional>
#include "NormalizedPosition.hpp"

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
		AABB(const Vec2& size);
		AABB(const WorldPosition& minPos, const WorldPosition& maxPos);

		WorldPosition GetGlobalMin(const WorldPosition& centerWorldPos) const;
		WorldPosition GetGlobalMax(const WorldPosition& centerWorldPos) const;

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
		WorldPosition GetWorldPos(const WorldPosition& centerPos, const NormalizedPosition& relativePos) const;

		std::string ToString(const WorldPosition& transformPos) const;
	};

	bool DoAABBIntersect(const Vec2& entity1Pos, const AABB& entity1Bounding,
		const Vec2& entity2Pos, const AABB& entity2Bounding);

	AABBIntersectionData GetAABBIntersectionData(const Vec2& entity1Pos, const AABB& entity1Bounding,
		const Vec2& entity2Pos, const AABB& entity2Bounding);

	Vec2 GetAABBMinDisplacement(const Vec2& entity1Pos, const AABB& entity1Bounding,
		const Vec2& entity2Pos, const AABB& entity2Bounding);

	/// <summary>
	/// Will get the direction that is the second one in relation to first in normalized dir
	/// NOTE: if collisions considered then it will use the entity1 CENTER as reference for dir whereas
	/// no collisions considered will only count if the entity2 is FULLY outside of entity1 in that direction
	/// Example: 'Background'(Entity1) is at x= 0 and 'Player'(entity2) is at x=1 then result is [1, 0] (RIGHT)
	/// returns [0, 0] if body2 is inside body1
	/// </summary>
	/// <param name="entity1Pos"></param>
	/// <param name="entity1Bounding"></param>
	/// <param name="entity2Pos"></param>
	/// <param name="entity2Bounding"></param>
	/// <returns></returns>
	Vec2 GetAABBDirection(const Vec2& entity1Pos, const AABB& entity1Bounding,
		const Vec2& entity2Pos, const AABB& entity2Bounding, const bool& considerCollisions= true);
}


