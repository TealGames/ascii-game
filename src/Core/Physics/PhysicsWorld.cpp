#include "pch.hpp"
#include "Core/Physics/PhysicsWorld.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "Core/Analyzation/ProfilerTimer.hpp"
#include "Utils/HelperFunctions.hpp"
#include "Utils/Debug.hpp"
#include "ECS/Component/Types/World/CollisionBoxComponent.hpp"
#include "Core/Collision/CollisionRegistry.hpp"

namespace Engine::Physics
{
	bool HasFlagEntityA(const EntityType& entityType)
	{
		const std::uint8_t typeConverted = static_cast<std::uint8_t>(entityType);
		return typeConverted == static_cast<std::uint8_t>(EntityType::A) ||
			   typeConverted == static_cast<std::uint8_t>(EntityType::AB);
	}
	bool HasFlagEntityB(const EntityType& entityType)
	{
		const std::uint8_t typeConverted = static_cast<std::uint8_t>(entityType);
		return typeConverted == static_cast<std::uint8_t>(EntityType::B) ||
			typeConverted == static_cast<std::uint8_t>(EntityType::AB);
	}

	PhysicsWorld::PhysicsWorld(CollisionRegistry& collisionRegistry)
		: m_collisionRegistry(collisionRegistry), m_bodies() {}


	const PhysicsBodyCollection& PhysicsWorld::GetBodies() const
	{
		return m_bodies;
	}

	PhysicsBodyCollection& PhysicsWorld::GetBodiesMutable()
	{
		return m_bodies;
	}

	void PhysicsWorld::AddBody(PhysicsBodyComponent& body)
	{
		//(this, std::format("Adding body: {}", body.m_Entity->GetName()));
		body.SetPhysicsWorldRef(*this);
		m_bodies.push_back(&body);
	}

	void PhysicsWorld::ClearAllBodies()
	{
		for (auto& body : m_bodies)
		{
			if (body == nullptr) continue;
			body->RemovePhysicsWorldRef();
		}
		m_bodies = {};
	}

	void PhysicsWorld::UpdateStart(const float& deltaTime)
	{
#ifdef ENABLE_PROFILER
		ProfilerTimer timer("PhysicsWorld::Update");
#endif 

		m_nonGroundedCollidingEntities.clear();
		m_nonGroundedCollidingEntities.reserve(m_collisionRegistry.GetTotalCollisionsCount());

		m_collisionRegistry.ExecuteOnAllCollisions([this](CollisionPair& collisionData) -> void
			{
				if (!Assert(collisionData.m_CollisionBoxA != nullptr, "Tried to resolve a collision:{} "
					"but collision box A is null", collisionData.ToString()))
					return;

				if (!Assert(collisionData.m_CollisionBoxB != nullptr, "Tried to resolve a collision:{} "
					"but collision box B is null", collisionData.ToString()))
					return;

				//TODO: if one has physics body while other does not do we still simulate physics?
				PhysicsBodyComponent* bodyA = collisionData.m_CollisionBoxA->GetEntityMutable().TryGetComponentMutable<PhysicsBodyComponent>();
				if (bodyA == nullptr) return;

				PhysicsBodyComponent* bodyB = collisionData.m_CollisionBoxB->GetEntityMutable().TryGetComponentMutable<PhysicsBodyComponent>();
				if (bodyB == nullptr) return;
				
				//Since touching collisions are not actually interecting inside another body we should not
				//apply any methods of collision resolution (but we still need to know they are colliding 
				//for things like grounded checks/gravity)
				
				//TODO: using global pos everywere is expensive, perhaps we can optimize by checking parents first?
				const Vec2 incomingBToADir = bodyA->GetEntityMutable().GetTransformMutable().GetWorldPos().GetXY().GetY() - 
					bodyB->GetEntityMutable().GetTransformMutable().GetWorldPos().GetXY();
				const float dotProductBofA = Math::DotProduct(incomingBToADir, bodyB->GetVelocity());
				/*LogError(std::format("touching:{} DOT BETWEEN B-> A:{} bodyB vel:{} is:{}", std::to_string(collisionData.m_IntersectionData.IsTouchingIntersection()), incomingBToADir.ToString(),
					bodyB->GetVelocity().ToString(), std::to_string(dotProductBofA)));*/
				if (collisionData.m_IntersectionData.IsTouchingIntersection() && dotProductBofA <=0)
				{
					//Assert(false, std::format("YES"));
					return;
				}


				ResolveCollision(collisionData, bodyA, bodyB);
			});
		for (int i = 0; i < m_bodies.size(); i++)
		{
			if (m_bodies[i] == nullptr) continue;

			PhysicsBodyComponent& body = *(m_bodies[i]);
			const CollisionBoxComponent& box = m_bodies[i]->GetCollisionBox();
			EntityData& bodyAEntity = body.GetEntityMutable();

			KinematicUpdate(deltaTime, bodyAEntity, body, box);
			m_OnObjectProcessed.Invoke(&body);
		}
	}

	void PhysicsWorld::ResolveCollision(CollisionPair& collision, PhysicsBodyComponent* bodyA, PhysicsBodyComponent* bodyB)
	{
		EntityData& entityA = collision.m_CollisionBoxA->GetEntityMutable();
		EntityData& entityB = collision.m_CollisionBoxB->GetEntityMutable();

		if (!Assert(bodyA != nullptr && bodyB != nullptr, "Tried to resolve collision:{} body bodyA and/or bodyB "
			"has no physicsBody", collision.ToString()))
			return;

		bool bothBodiesZeroVelocity = bodyA->GetVelocity() == Vec2::Zero() && bodyB->GetVelocity() == Vec2::Zero();

		//By default, we always want to push one of the bodies out
		PushMovedBodyOut(entityA, entityB, *bodyA, *bodyB, collision);

		//If the movement happened instantly (both have no velocities) then we can exit after push out
		if (bothBodiesZeroVelocity)
		{
			//Assert(false, std::format("Leaving due to no vel"));
			return;
		}

		//If both have mass (and both are NOT constrained) we can apply impulse to resolve collision
		//and we can just not set the constrained object's position
		if (bodyA->HasMass() && bodyB->HasMass())
		{
			//TODO: consider external forces which results in no conservation of momentum
			//Assert(false, std::format("IMPUSLESE"));
			ApplyImpulse(entityA, entityB, *bodyA, *bodyB, collision.m_IntersectionData);
			/*LogError(std::format("APPLYING IMPULSE for depth:{} separation:{}", collision.m_IntersectionData.ToString(), 
				std::to_string((entityA.m_Transform.GetPos() > entityB.m_Transform.GetPos()? 
					collision.m_CollisionBoxA->GetGlobalMin() - collision.m_CollisionBoxB->GetGlobalMax() :
					collision.m_CollisionBoxA->GetGlobalMax() - collision.m_CollisionBoxB->GetGlobalMin()
				).m_Y)), true, false, false, true);*/

			return;
		}

		const bool isConstrainedA = bodyA->HasAnyConstraints();
		const bool isConstrainedB = bodyB->HasAnyConstraints();
		//If one is constrianed but the other is not, the one that is moving gets the new velocity
		//applied from the restitution setting in the physics profile (mass ignored)
		if (isConstrainedA!= isConstrainedB)
		{
			//Assert(false, std::format("single constraint"));
			EntityType updatedEntity = bodyA->HasAnyConstraints() ? EntityType::B : EntityType::A;
			SetVelocitiesFromRestitution(entityA, entityB, *bodyA, *bodyB, collision.m_IntersectionData, updatedEntity);
			return;
		}
		//If none of them are constrained (and either one has mass or neither have mass)
		//we can then use similar velocity setting from restitution but on both entities
		if (!isConstrainedA && !isConstrainedB)
		{
			SetVelocitiesFromRestitution(entityA, entityB, *bodyA, *bodyB, collision.m_IntersectionData, EntityType::AB);
			return;
		}

		LogError(std::format("Tried to handle collision:{} but no previous resolution methods "
			"were possible so simple push is executed", collision.ToString()));
		return;
	}

	void PhysicsWorld::KinematicUpdate(const float& deltaTime, EntityData& entity, PhysicsBodyComponent& body, const CollisionBoxComponent& box)
	{
		body.SetIsGrounded(m_collisionRegistry.IsCollidingInDirs(box,
			{ MoveDirection::South, MoveDirection::Southeast, MoveDirection::Southwest }, true));
		//if (isGrounded) Assert(false, std::format("BALLS"));

		/*if (entity.GetName()=="player") LogError(std::format("Player has down collisions:{} total collisions:{}", 
			std::to_string(body.IsGrounded()), ::Utils::ToStringIterable<std::vector<MoveDirection>, MoveDirection>(m_collisionRegistry.TryGetCollisionDirs(box))));*/

		bool gravitySet = false;
		if (!body.IsGrounded() && !body.IsExperiencingGravity())
		{
			body.SetAcceleration({ body.GetAcceleration().m_X, body.GetGravity() });
		}
		else if (body.IsGrounded() && body.IsExperiencingGravity())
		{
			gravitySet = true;
			body.SetAcceleration({body.GetAcceleration().m_X, 0});
			//body.SetVelocity(Vec2::ZERO);
			//Assert(false, std::format("gravity removed"));
			//return;
		}

		body.SetVelocityDelta(body.GetAcceleration() * deltaTime);

		float moveX = body.GetVelocity().m_X * deltaTime;
		if (body.HasXConstraint()) moveX = 0;

		float moveY = body.GetVelocity().m_Y * deltaTime;
		if (body.HasYConstraint()) moveY = 0;

		//LogWarning(std::format("ENTITY SETTING POS: {}", std::to_string(xVelocity), std::to_string(yVelocity)));
		entity.GetTransformMutable().GetLocalPosMutable() += Vec3(moveX, moveY, 0);
	}

	void PhysicsWorld::PushMovedBodyOut(EntityData& entityA, EntityData& entityB,
		PhysicsBodyComponent& bodyA, PhysicsBodyComponent& bodyB, const CollisionPair& collision)
	{
		//If A has constraints or both have constraints (since it is first choice A will get choosen)
		//then we select A, otherwise we select B if A has constrinats. If none have constraints,
		//we simply pick the one that moved last frame

		bool isMoveEntityB = false;
		if (bodyB.HasAnyConstraints())
		{
			isMoveEntityB = false;
			if (bodyA.HasAnyConstraints())
			{
				LogError(std::format("Handling collision for pair:{} but both are constrained "
					"objects so entityA is pushed out", collision.ToString()));
			}
		}
		else if (bodyA.HasAnyConstraints()) isMoveEntityB = true;
		else isMoveEntityB = false;

		EntityData* movedEntity = isMoveEntityB ? &entityB : &entityA;

		//Note: the pentration depth will be the same for both bodies (they might just have different signs)
		const bool xIsMin = std::abs(collision.m_IntersectionData.m_Depth.m_X) < std::abs(collision.m_IntersectionData.m_Depth.m_Y) 
			&& collision.m_IntersectionData.m_Depth.m_X != 0;

		//Note: since depth is for B penetration into A, if it was B that moved we need reverse depth to get out
		//otherwise if it is A, the depth would be the reverse of depth data, thus we do not need to change direction of move
		const int deltaSign = movedEntity == &entityA ? 1 : -1;
		const float moveDelta = (xIsMin ? collision.m_IntersectionData.m_Depth.m_X : collision.m_IntersectionData.m_Depth.m_Y) * deltaSign;

		if (xIsMin) movedEntity->GetTransformMutable().GetLocalPosMutable().m_X += moveDelta;
		else movedEntity->GetTransformMutable().GetLocalPosMutable().m_Y += moveDelta;
	}

	void PhysicsWorld::ApplyImpulse(EntityData& entityA, EntityData& entityB,
		PhysicsBodyComponent& bodyA, PhysicsBodyComponent& bodyB, const AABBIntersectionData& intersectionData)
	{
		const float e = (bodyA.GetSurfacePhysicsProfile().GetRestitution() +
			bodyB.GetSurfacePhysicsProfile().GetRestitution()) / 2;

		const Vec2 collsionNormalA = -GetCollisionNormalBodyB(intersectionData);

		const float mA = bodyA.GetMass();
		const float mB = bodyB.GetMass();
		const Vec2 vnOA = collsionNormalA * Math::DotProduct(bodyA.GetVelocity(), collsionNormalA);
		const Vec2 vnOB = collsionNormalA * Math::DotProduct(bodyB.GetVelocity(), collsionNormalA);

		const Vec2 vNormalFinalA = ((vnOA * mA) + (vnOB * mB) - ((vnOA - vnOB) * e * mB)) / (mA + mB);
		Vec2 newVelA = vNormalFinalA + (bodyA.GetVelocity() - vnOA);
		const Vec2 impulseA = (newVelA * bodyA.GetMass()) - bodyA.GetMomentum();

		if (!bodyA.HasAnyConstraints())
		{
			//We apply a threshold to stop velocity when the velocity is small enough to prevent jittering
			if (std::abs(newVelA.GetMagnitude()) <= BOUNCE_END_SPEED_THRESHOLD)
			{
				newVelA = {};
				//Assert(false, std::format("SETTING 0 VEL IN IMPUSELSE NOW IT IS:{}", newVelA.ToString()));
			}

			//bodyA.SetAcceleration(Vec2::ZERO);
			bodyA.SetVelocity(newVelA);
		}
		
		if (!bodyB.HasAnyConstraints())
		{
			Vec2 newVelB = (bodyB.GetMomentum() - impulseA) / bodyB.GetMass();
			//We apply a threshold to stop velocity when the velocity is small enough to prevent jittering
			if (std::abs(newVelB.GetMagnitude()) <= BOUNCE_END_SPEED_THRESHOLD)
			{
				newVelB = {};
				//Assert(false, std::format("SETTING 0 VEL IN IMPUSELSE"));
			}

			//bodyB.SetAcceleration(Vec2::ZERO);
			bodyB.SetVelocity(newVelB);
		}
	}

	//PRECONDITION: we assume that one of the bodies is NOT constrained
	void PhysicsWorld::SetVelocitiesFromRestitution(EntityData& entityA, EntityData& entityB,
		PhysicsBodyComponent& bodyA, PhysicsBodyComponent& bodyB, const AABBIntersectionData& intersectionData, 
		const EntityType updateEntityType)
	{
		const Vec2 collsionNormalA = -GetCollisionNormalBodyB(intersectionData);

		const float averageRestitution = (bodyA.GetSurfacePhysicsProfile().GetRestitution() +
			bodyB.GetSurfacePhysicsProfile().GetRestitution()) / 2;

		Vec2 newBodyNormal = {};
		Vec2 velocityNormal = {};

		// For each entity if we have to update it, we first get normal component of old velocity 
		//Then we update it based on restitution (how much velocity is left) in opposite direction
		//The new velocity is the vector in normal (collision) dir + tangential velocity unchanged (old vel - old vel normal)
		if (HasFlagEntityA(updateEntityType))
		{
			velocityNormal = collsionNormalA * Math::DotProduct(bodyA.GetVelocity(), collsionNormalA);
			newBodyNormal= velocityNormal * -(averageRestitution);
			bodyA.SetVelocity(newBodyNormal + (bodyA.GetVelocity() - velocityNormal));
		}
		if (HasFlagEntityB(updateEntityType))
		{
			//Since collision normal is in terms of A, B's normal is the opposite direction
			velocityNormal = -collsionNormalA * Math::DotProduct(bodyB.GetVelocity(), -collsionNormalA);
			newBodyNormal = velocityNormal * -(averageRestitution);
			bodyB.SetVelocity(newBodyNormal + (bodyB.GetVelocity() - velocityNormal));
		}
	}

	Vec2 PhysicsWorld::GetCollisionNormalBodyB(const AABBIntersectionData& data)
	{
		//return (boxB.GetAABBCenterWorldPos() - boxA.GetAABBCenterWorldPos()).GetNormalized();
		return data.m_Depth.GetNormalized();
	}

	float PhysicsWorld::CalculateImpulse(const PhysicsBodyComponent& targetObject, const PhysicsBodyComponent& collidedObject, const Vec2& collisionNormal)
	{
		const float averageRestitution = (targetObject.GetSurfacePhysicsProfile().GetRestitution() +
									collidedObject.GetSurfacePhysicsProfile().GetRestitution()) / 2;

		const float& massA = targetObject.GetMass();
		const float& massB = collidedObject.GetMass();

		const Vec2& velA = targetObject.GetVelocity();
		const Vec2& velB = collidedObject.GetVelocity();
		//LogError(std::format("Dot product for{} dot {} is: {}", velA.ToString(), velB.ToString(),std::to_string(DotProduct(velA, velB)) ));
		return ((1 + averageRestitution) * massA * massB) / (massA + massB) * 
			   (Math::DotProduct(velB, collisionNormal) - Math::DotProduct(velA, collisionNormal));
	}

	bool DoBodiesIntersect(const PhysicsBodyComponent& body1, const PhysicsBodyComponent& body2)
	{
		return body1.GetCollisionBox().DoIntersect(body2.GetCollisionBox());
	}

	Vec2 GetBodyMinDisplacement(const PhysicsBodyComponent& body1, const PhysicsBodyComponent& body2)
	{
		return body1.GetCollisionBox().GetAABBMinDisplacement(body2.GetCollisionBox());
	}

	RaycastInfo2D PhysicsWorld::Raycast2D(const WorldPosition2D& origin, const Vec2& ray) const
	{
		RaycastInfo2D result = {};
		int xSign = ::Math::GetSign(ray.m_X);
		int ySign = ::Math::GetSign(ray.m_Y);
		WorldPosition2D rayEndPoint = origin + ray;
		Vec2 rayDir = ray.GetNormalized();

		WorldPosition2D boundsWorldMin = {};
		WorldPosition2D boundsWorldMax = {};
		
		float t1 = 0;
		float t2 = 0;

		//Since we consider t values from origin and onward (where tMin represents origin point and tMax represents ray end)
		float tMin = 0;
		float tMax = std::numeric_limits<float>::infinity();
		
		for (auto& body : m_bodies)
		{
			boundsWorldMin = body->GetCollisionBox().GetGlobalMin();
			boundsWorldMax = body->GetCollisionBox().GetGlobalMax();

			//TODO: perhaps optimizations could be made by checking to see if distance is too big to make it to this collider
			//so we can just continue

			//----------- X AXIS -----------
			//If the ray is vertical and outside box, there are no intersections here
			if (::Math::ApproximateEqualsF(ray.m_X, 0) &&
				(origin.m_X < boundsWorldMin.m_X || origin.m_X > boundsWorldMax.m_X))
			{
				continue;
			}

			t1 = (boundsWorldMin.m_X - origin.m_X) / rayDir.m_X;
			t2= (boundsWorldMax.m_X - origin.m_X) / rayDir.m_X;
			//Swap in case the direction is negative meaning the closer and far vertical bounds are reversed
			if (t1 > t2) std::swap(t1, t2);
			tMin = std::max(tMin, t1);
			tMax = std::min(tMax, t2);

			if (tMin > tMax)
			{
				//LogError(std::format("SKIPPING DUE TO MIN: {} > max; {}", std::to_string(tMin), std::to_string(tMax)));
				continue;
			}
			
			//----------- Y AXIS -----------
			//Repeats the same process but for y axis
			if (::Math::ApproximateEqualsF(ray.m_Y, 0) &&
				(origin.m_Y < boundsWorldMin.m_Y || origin.m_Y > boundsWorldMax.m_Y))
			{
				continue;
			}

			t1 = (boundsWorldMin.m_Y - origin.m_Y) / rayDir.m_Y;
			t2 = (boundsWorldMax.m_Y - origin.m_Y) / rayDir.m_Y;
			if (t1 > t2) std::swap(t1, t2);
			tMin = std::max(tMin, t1);
			tMax = std::min(tMax, t2);

			if (tMin > tMax) 
				continue;

			//Note: when reaching this point tMin is the intersecction distance from origin
			result.m_BodyHit = body;
			result.m_Displacement = {rayDir.m_X * tMin, rayDir.m_Y * tMin};
			result.m_HitPos = origin + Vec2(result.m_Displacement.m_X, result.m_Displacement.m_Y);
			return result;
		}
		return result;
	}
}


