#pragma once
#include <vector>
#include "Point2DInt.hpp"
#include "PhysicsBodyData.hpp"
#include "Vec2.hpp"
#include "WorldPosition.hpp"

namespace Physics
{
	struct RaycastInfo
	{
		PhysicsBodyData* m_BodyHit = nullptr;
		WorldPosition m_HitPos = {};
		Vec2 m_Displacement = Vec2::ZERO;
	};

	using BodyCollectionType = std::vector<PhysicsBodyData*>;
	class PhysicsWorld
	{
	private:
		//TODO: this needs to get optimized since this can get really slow
		//Probably optimize using a vector that holds bodies and a binary tree that holds aabb data that is used
		//to check intersection with progressivley smaller nodes until we reach leaf nodes which hold the actual objects
		
		/// <summary>
		/// All the bodies that will be used in the physics simulation
		/// </summary>
		BodyCollectionType m_bodies;
	public:
		/// <summary>
		/// The max distance from another body that is still considered as a "collision"/ touching
		/// </summary>
		static constexpr float MAX_DISTANCE_FOR_COLLISION = 0.01;

	private:
	public:
		PhysicsWorld();

		const BodyCollectionType& GetBodies() const;
		BodyCollectionType& GetBodiesMutable();

		void AddBody(PhysicsBodyData& body);

		void UpdateStart(const float& deltaTime);
		void UpdateEnd();

		/// <summary>
		/// Will raycast from the origin using the ray with magnitude and direction using the slab method
		/// It is essentially like creating a line and finding where it intersections the 4 sides
		/// Slab refers to the area between parallel lines, so we essnetially find min (closest) and max (farthest) 
		/// intersection points for lines for x (both vertical lines) and y (both horizontal lines). Then we
		/// combine those positions into the intersection point which is the min for all segments
		/// </summary>
		/// <param name="origin"></param>
		/// <param name="ray"></param>
		/// <returns></returns>
		RaycastInfo Raycast(const WorldPosition& origin, const Vec2& ray);
	};

	/// <summary>
	/// Will check if the two bodies AABB intersect with each other
	/// </summary>
	/// <param name="body1"></param>
	/// <param name="body2"></param>
	/// <returns></returns>
	bool DoBodiesIntersect(const PhysicsBodyData& body1, const PhysicsBodyData& body2);

	Utils::Point2D GetBodyMinDisplacement(const PhysicsBodyData& body1, const PhysicsBodyData& body2);
}


