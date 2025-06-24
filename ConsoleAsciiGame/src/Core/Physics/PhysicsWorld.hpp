#pragma once
#include <vector>
#include <unordered_set>
#include <cstdint>
#include "Utils/Data/Vec2.hpp"
#include "Utils/Data/WorldPosition.hpp"
#include "Core/Collision/CollisionRegistry.hpp"
#include "ECS/Component/Types/World/PhysicsBodyData.hpp"

class CollisionBoxData;
namespace ECS
{
	class Entity;
}

namespace Physics
{
	enum class EntityType : std::uint8_t
	{
		A = 0,
		B = 1,
		AB = 2,
	};
	bool HasFlagEntityA(const EntityType& entityType);
	bool HasFlagEntityB(const EntityType& entityType);

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

		/// <summary>
		/// Stores a list of entity names that have had their collisions resolved this frame
		/// mainly so certain functions should/should not be applied when an entity is marked with a collision resolved
		/// </summary>
		//TODO: perhaps this could be avoided by having a tag property on an entity that can be a const char*
		std::unordered_set<std::string> m_nonGroundedCollidingEntities;

	public:
		/// <summary>
		/// The speed threshold to finish a bounce to prevent jittery behavior when the jump height becomes small
		/// </summary>
		static constexpr float BOUNCE_END_SPEED_THRESHOLD = 0.5;

	private:
		/// <summary>
		/// Will get the collision normal for the two collision boxes in terms of box A
		/// </summary>
		/// <param name="boxA"></param>
		/// <param name="boxB"></param>
		/// <returns></returns>
		Vec2 GetCollisionNormalBodyB(const AABBIntersectionData& data);
		//float CalculateImpulse(const PhysicsBodyData& targetObject, const PhysicsBodyData& collidedObject, const Vec2& collisionNormal);
		float CalculateImpulse(const PhysicsBodyData& targetObject, const PhysicsBodyData& collidedObject, const Vec2& collisionNormal);

		/// <summary>
		/// Will resolve collisions in a variety of manners:
		/// -> first will push object out while still counting it as colliding but not interesecting in area
		/// -> will attempt to use impulse (only if both have mass and are not constrained)
		/// -> will then set new body velocities singularly (if one is constraiend) or both (none constrained)
		/// Note: returns true if the colliding bodies are SEPARATED and NO LONGER COLLIDING (since simple push does not separate them from collision
		/// and only moves it out to min possible area to be counted as collision it would return false)
		/// </summary>
		/// <param name="collision"></param>
		/// <returns></returns>
		void ResolveCollision(CollisionPair& collision, PhysicsBodyData* bodyA, PhysicsBodyData* bodyB);

		void KinematicUpdate(const float& deltaTime, EntityData& entity,
			PhysicsBodyData& body, const CollisionBoxData& collider);

		/// <summary>
		/// The basic type of collision resolution that will push the moved body out or the one which is not
		/// constrained (will force move constrined one in edge case if both constrained) to a direction
		/// This is by default applied to all collisions
		/// </summary>
		/// <param name="entityA"></param>
		/// <param name="entityB"></param>
		/// <param name="bodyA"></param>
		/// <param name="bodyB"></param>
		/// <param name="collision"></param>
		void PushMovedBodyOut(EntityData& entityA, EntityData& entityB,
			PhysicsBodyData& bodyA, PhysicsBodyData& bodyB, const CollisionPair& collision);

		/// <summary>
		/// A type of collision resolution that will apply an impulse to both entities 
		/// using their mass and incoming velocities as well as
		/// their physics profile such as restitution settings
		/// Note: only works when both have NO constraints, both have mass and velocities
		/// </summary>
		/// <param name="entityA"></param>
		/// <param name="entityB"></param>
		/// <param name="bodyA"></param>
		/// <param name="bodyB"></param>
		/// <param name="data"></param>
		void ApplyImpulse(EntityData& entityA, EntityData& entityB,
			PhysicsBodyData& bodyA, PhysicsBodyData& bodyB, const AABBIntersectionData& data);

		/// <summary>
		/// A type of collision resolution that will set the velocities of either entity A, B or both 
		/// from restitution physics settings which dicatates how much velocity is left in normal collision 
		/// direction and can thus be used to calcualte new velocities regardless of mass
		/// Note: used when only one or none have mass values or when one body is constrained while the other is not
		/// </summary>
		/// <param name="entityA"></param>
		/// <param name="entityB"></param>
		/// <param name="bodyA"></param>
		/// <param name="bodyB"></param>
		/// <param name="data"></param>
		/// <param name="updateEntityType"></param>
		void SetVelocitiesFromRestitution(EntityData& entityA, EntityData& entityB,
			PhysicsBodyData& bodyA, PhysicsBodyData& bodyB, const AABBIntersectionData& data, 
			const EntityType updateEntityType);

	public:
		PhysicsWorld(CollisionRegistry& collisionRegistry);

		const PhysicsBodyCollection& GetBodies() const;
		PhysicsBodyCollection& GetBodiesMutable();

		void AddBody(PhysicsBodyData& body);
		void ClearAllBodies();

		void UpdateStart(const float& deltaTime);

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


