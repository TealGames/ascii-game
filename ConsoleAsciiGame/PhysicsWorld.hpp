#pragma once
#include <vector>
#include "Vec2.hpp"
#include "WorldPosition.hpp"
#include "CollisionRegistry.hpp"
#include "PhysicsBodyData.hpp"

class CollisionBoxData;
namespace ECS
{
	class Entity;
}

namespace Physics
{
	struct RaycastInfo
	{
		PhysicsBodyData* m_BodyHit = nullptr;
		WorldPosition m_HitPos = {};
		Vec2 m_Displacement = Vec2::ZERO;
	};

	using PhysicsBodyCollection = std::vector<PhysicsBodyData*>;
	class PhysicsWorld
	{
	private:
		//TODO: this needs to get optimized since this can get really slow
		//Probably optimize using a vector that holds bodies and a binary tree that holds aabb data that is used
		//to check intersection with progressivley smaller nodes until we reach leaf nodes which hold the actual objects
		
		/// <summary>
		/// All the bodies that will be used in the physics simulation
		/// </summary>
		PhysicsBodyCollection m_bodies;
		CollisionRegistry& m_collisionRegistry;

	public:
		/// <summary>
		/// The max distance from another body that is still considered as a "collision"/ touching
		/// </summary>
		static constexpr float BOUNCE_END_SPEED_THRESHOLD = 1;

	private:
		/// <summary>
		/// Will get the collision normal for the two collision boxes in terms of box A
		/// </summary>
		/// <param name="boxA"></param>
		/// <param name="boxB"></param>
		/// <returns></returns>
		Vec2 GetCollisionNormal(const CollisionBoxData& boxA, const CollisionBoxData& boxB);
		float CalculateImpulse(const PhysicsBodyData& targetObject, const PhysicsBodyData& collidedObject, const Vec2& collisionNormal);

		void HandleCollision(CollisionPair& collision);
		void KinematicUpdate(const float& deltaTime, ECS::Entity& entity, 
			PhysicsBodyData& body, const CollisionBoxData& collider);

	public:
		PhysicsWorld(CollisionRegistry& collisionRegistry);

		const PhysicsBodyCollection& GetBodies() const;
		PhysicsBodyCollection& GetBodiesMutable();
		PhysicsBodyCollection FindBodiesContainingPos(const WorldPosition& wordlPos) const;

		void AddBody(PhysicsBodyData& body);
		void ClearAllBodies();

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
		RaycastInfo Raycast(const WorldPosition& origin, const Vec2& ray) const;
	};

	/// <summary>
	/// Will check if the two bodies AABB intersect with each other
	/// </summary>
	/// <param name="body1"></param>
	/// <param name="body2"></param>
	/// <returns></returns>
	bool DoBodiesIntersect(const PhysicsBodyData& body1, const PhysicsBodyData& body2);

	Vec2 GetBodyMinDisplacement(const PhysicsBodyData& body1, const PhysicsBodyData& body2);
}


