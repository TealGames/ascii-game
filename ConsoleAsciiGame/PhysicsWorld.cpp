#include "pch.hpp"
#include "PhysicsWorld.hpp"
#include "Entity.hpp"
#include "ProfilerTimer.hpp"
#include "Point2D.hpp"
#include "HelperFunctions.hpp"
#include "Debug.hpp"
#include "CollisionBoxData.hpp"
#include "CollisionRegistry.hpp"

namespace Physics
{
	PhysicsWorld::PhysicsWorld(CollisionRegistry& collisionRegistry) 
		: m_collisionRegistry(collisionRegistry), m_bodies{} {}

	const PhysicsBodyCollection& PhysicsWorld::GetBodies() const
	{
		return m_bodies;
	}

	PhysicsBodyCollection& PhysicsWorld::GetBodiesMutable()
	{
		return m_bodies;
	}

	void PhysicsWorld::AddBody(PhysicsBodyData& body)
	{
		LogWarning(this, std::format("Adding body: {}", body.m_Entity->GetName()));
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

	PhysicsBodyCollection PhysicsWorld::FindBodiesContainingPos(const WorldPosition& worldPos) const
	{
		PhysicsBodyCollection bodiesFound = {};
		for (const auto& body : m_bodies)
		{
			//if (body->DoesAABBContainPos(worldPos))
			if (body->GetCollisionBox().DoIntersect(worldPos))
			{
				bodiesFound.push_back(body);
			}
		}

		return bodiesFound;
	}

	void PhysicsWorld::UpdateStart(const float& deltaTime)
	{
#ifdef ENABLE_PROFILER
		ProfilerTimer timer("PhysicsWorld::Update");
#endif 

		//AABBIntersectionData collision = {};
		//Vec2 minBodyDisplacement = {};
		//Vec2 minBodyDisplacementVec = {};
		for (int i = 0; i< m_bodies.size(); i++)
		{
			if (m_bodies[i] == nullptr) continue;

			PhysicsBodyData& body = *(m_bodies[i]);
			const CollisionBoxData& box = m_bodies[i]->GetCollisionBox();
			ECS::Entity& bodyAEntity = body.GetEntitySafeMutable();

			auto bodyACollisions = m_collisionRegistry.TryGetCollisionsMutable(box);
			//NOTE: WE ARE NOT MARKING ANY COLLISIONS HERE we only resolve them and do something
			//with info but collider system gets to determine if they are still colliding next frame
			for (const auto& collisionData : bodyACollisions)
			{
				Assert(false, std::format("Found collision"));
				HandleCollision(*collisionData);
			}

			KinematicUpdate(deltaTime, bodyAEntity, body, box);

			//We do not need to simulate bodies with no movement since collisions are resolved on bodies that move
			//if (Utils::ApproximateEqualsF(bodyA.GetVelocity().GetMagnitude(), 0)) continue;
			//collision = {};

			/*
			//Note: this could get optimized to not need double checking for each possible collision
			//but we need it in this time to ensure that velocity and other updates occur for BOTH bodies
			//and not just to one
			for (int j = 0; j < m_bodies.size(); j++)
			{
				if (j == i) continue;
				if (m_bodies[j] == nullptr) continue;

				PhysicsBodyData& bodyB = *(m_bodies[j]);
				ECS::Entity& bodyBEntity = bodyB.GetEntitySafeMutable();

				auto entityACollisionBIt = bodyA.GetCollidingBodyIterator(bodyB);
				bool isValidIt = bodyA.IsValidCollidingBodyIterator(entityACollisionBIt);

				//Intersection is handled as BODYA is the body that is colliding with BODYB (Pretending as though bodyb is not moving)
				//NOTE: so we are essentially saying is BODY A encroaching on any other bodies space and if so do something
				//collision = Physics::GetAABBIntersectionData(bodyBEntity.m_Transform.m_Pos, bodyB.GetAABB(), bodyAEntity.m_Transform.m_Pos, bodyA.GetAABB());
				collision = bodyB.GetCollisionBox().GetCollisionIntersectionData(bodyA.GetCollisionBox());
				if (collision.m_DoIntersect)
				{
					//IF BODYA has moved this past frame (it means it must have moved into bodyB)
					if (bodyA.GetEntitySafe().m_Transform.m_Pos!= bodyA.GetEntitySafe().m_Transform.m_LastPos)
					{
						if (bodyA.GetVelocity() == Vec2::ZERO || !bodyA.ConservesMomentum() || !bodyB.ConservesMomentum())
						{
							bool xIsMin = std::abs(collision.m_Depth.m_X) < std::abs(collision.m_Depth.m_Y);
							float moveDelta = -(xIsMin ? collision.m_Depth.m_X : collision.m_Depth.m_Y);
							//We add the extra distance so that they are still considered as colliding but would not cause any awkward jittering
							//moveDelta += Utils::GetSign(moveDelta) * MAX_DISTANCE_FOR_COLLISION;

							if (xIsMin) bodyAEntity.m_Transform.SetPosDeltaX(moveDelta);
							else bodyAEntity.m_Transform.SetPosDeltaY(moveDelta);

							bodyA.SetAcceleration(Vec2::ZERO);
							bodyA.SetVelocity(Vec2::ZERO);
						}
						else
						{
							//TODO: the collision normal does not fully consider cases where things may hit at angles
							const Vec2 collsionNormal = bodyA.GetVelocity().GetOppositeDirection().GetNormalized();
							const Vec2& oldMomenumn = bodyA.GetMomentum();
							const Vec2 newMomentum = oldMomenumn + (collsionNormal * CalculateImpulse(bodyA, bodyB, collsionNormal));

							

							Vec2 newVel = newMomentum / bodyA.GetMass();
							if (std::abs(newVel.m_X) <= BOUNCE_END_SPEED_THRESHOLD) newVel.m_X = 0;
							if (std::abs(newVel.m_Y) <= BOUNCE_END_SPEED_THRESHOLD) newVel.m_Y = 0;


							Vec2 moveDelta = collision.m_Depth.GetOppositeDirection();
							bodyAEntity.m_Transform.SetPosDelta({ moveDelta.m_X, moveDelta.m_Y });

							Log(this, std::format("COSNERVING for {} MASS A: {} MASS B: {} DEPTH: {} in norma: {} old p: {} j delta;{} new: {} vel: {}",
								bodyA.GetEntitySafe().GetName(), std::to_string(bodyA.GetMass()), std::to_string(bodyB.GetMass()),
								moveDelta.ToString(),
								collsionNormal.ToString(), oldMomenumn.ToString(), std::to_string(CalculateImpulse(bodyA, bodyB, collsionNormal)),
								newMomentum.ToString(), (newMomentum / bodyA.GetMass()).ToString()));

							bodyA.SetAcceleration(Vec2::ZERO);
							bodyA.SetVelocity(newVel);
							//throw std::invalid_argument("FART");
						}

						//TODO: problem is if there is body above and below, making it possible for it to move back and worth between collisions of the two
						//minBodyDisplacement = GetBodyMinDisplacement(bodyB, bodyA);
						//minBodyDisplacementVec = { std::abs(minBodyDisplacement.m_X), std::abs(minBodyDisplacement.m_Y) };
						//LogWarning(std::format("After collision prevented new distance: {}", minBodyDisplacementVec.ToString()));
						//throw new std::invalid_argument("yeah");
					}
					if (!isValidIt) bodyA.AddCollidingBody(bodyB);
					break;
				}
				

				minBodyDisplacement = GetBodyMinDisplacement(bodyB, bodyA);
				minBodyDisplacementVec = { std::abs(minBodyDisplacement.m_X), std::abs(minBodyDisplacement.m_Y) };
				//If we pass intersect check (meaning we didnt intersect) and it was valid we can remove iterator if we pass
				//the threshold in order to allow for some degree of touching to still be considered as colliding
				if (isValidIt && minBodyDisplacementVec.GetMagnitude() > MAX_DISTANCE_FOR_COLLISION)
				{
					bodyA.RemoveCollidingBody(entityACollisionBIt);
				}
				
			}
			*/

			/*LogWarning(std::format("COLLISION FOR ENTITY: {} FOUND: {} DEPTH: {}", bodyA.m_Entity->m_Name, 
				std::to_string(collision.m_DoIntersect), collision.m_Depth.ToString()));*/
		}
	}

	void PhysicsWorld::HandleCollision(CollisionPair& collision)
	{
		ECS::Entity& entityA = collision.m_CollisionBoxA->GetEntitySafeMutable();
		ECS::Entity& entityB = collision.m_CollisionBoxB->GetEntitySafeMutable();

		PhysicsBodyData* bodyA = entityA.TryGetComponentMutable<PhysicsBodyData>();
		PhysicsBodyData* bodyB = entityB.TryGetComponentMutable<PhysicsBodyData>();

		//IF BODYA has moved this past frame (it means it must have moved into bodyB)

		if ((bodyA->GetVelocity()==Vec2::ZERO && bodyB->GetVelocity()==Vec2::ZERO) 
			|| !bodyA->ConservesMomentum() || !bodyB->ConservesMomentum())
		{
			//Note: the pentration depth will be the same for both bodies (they might just have different signs)
			//TODO: we only really consider b colliding into a in terms of depth (that is how it is calcualted so we must consider both)
			bool xIsMin = std::abs(collision.m_IntersectionData.m_Depth.m_X) < std::abs(collision.m_IntersectionData.m_Depth.m_Y);
			float moveDelta = -(xIsMin ? collision.m_IntersectionData.m_Depth.m_X : collision.m_IntersectionData.m_Depth.m_Y);

			//We add the extra distance so that they are still considered as colliding but would not cause any awkward jittering
			//moveDelta += Utils::GetSign(moveDelta) * MAX_DISTANCE_FOR_COLLISION;

			const bool aMovedLastFrame = entityA.m_Transform.m_Pos != entityA.m_Transform.m_LastPos;

			//If entityA was the one that moved into this area, we move A out of B, otherwize we move B out of A
			ECS::Entity& movedEntity = aMovedLastFrame ? entityA : entityB;
			if (xIsMin) movedEntity.m_Transform.SetPosDeltaX(moveDelta);
			else movedEntity.m_Transform.SetPosDeltaY(moveDelta);

			//Whichever one got moved out should not have any further movement to prevent potential jittering
			PhysicsBodyData& movedBody = aMovedLastFrame ? *bodyA : *bodyB;
			movedBody.SetAcceleration(Vec2::ZERO);
			movedBody.SetVelocity(Vec2::ZERO);
		}
		else
		{
			//const Vec2 collsionNormal = bodyA.GetVelocity().GetOppositeDirection().GetNormalized();
			const Vec2 collsionNormal = GetCollisionNormal(*collision.m_CollisionBoxA, *collision.m_CollisionBoxB);
			const Vec2& oldMomenumnA = bodyA->GetMomentum();
			const Vec2 impulseA = collsionNormal * CalculateImpulse(*bodyA, *bodyB, collsionNormal);

			//Since impulse gained by one is the one lost by other, we calculate impulse A then apply opposite
			//to be to find both of their new velocities because of momentum conservation
			Vec2 newVelA = (oldMomenumnA+ impulseA) / bodyA->GetMass();
			Vec2 newVelB = (bodyB->GetMomentum() - impulseA) / bodyB->GetMass();

			//We clamp velocities if they surpass bounce speed threshold so they do not bounce forever 
			//with very small bounce heights/jittery look
			if (std::abs(newVelA.m_X) <= BOUNCE_END_SPEED_THRESHOLD) newVelA.m_X = 0;
			if (std::abs(newVelA.m_Y) <= BOUNCE_END_SPEED_THRESHOLD) newVelA.m_Y = 0;

			if (std::abs(newVelB.m_X) <= BOUNCE_END_SPEED_THRESHOLD) newVelB.m_X = 0;
			if (std::abs(newVelB.m_Y) <= BOUNCE_END_SPEED_THRESHOLD) newVelB.m_Y = 0;

			//We move entityA outside of colliding body (note: in reality it does not matter which one we move out)
			Vec2 moveDelta = collision.m_IntersectionData.m_Depth.GetOppositeDirection();
			entityA.m_Transform.SetPosDelta({ moveDelta.m_X, moveDelta.m_Y });

			Log(this, std::format("COSNERVING for {} MASS A: {} MASS B: {} DEPTH: {} in norma: {} old p: {} j delta;{} vela:{} velB:{}",
				entityA.GetName(), std::to_string(bodyA->GetMass()), std::to_string(bodyB->GetMass()),
				moveDelta.ToString(),
				collsionNormal.ToString(), oldMomenumnA.ToString(), impulseA.ToString(),
				newVelA.ToString(), newVelB.ToString()));

			bodyA->SetAcceleration(Vec2::ZERO);
			bodyA->SetVelocity(newVelA);

			bodyB->SetAcceleration(Vec2::ZERO);
			bodyB->SetVelocity(newVelB);
			//throw std::invalid_argument("FART");
		}
	}

	void PhysicsWorld::KinematicUpdate(const float& deltaTime, ECS::Entity& entity, PhysicsBodyData& body, const CollisionBoxData& box)
	{
		if (!m_collisionRegistry.IsCollidingInDirs(box, { MoveDirection::South, MoveDirection::Southeast, MoveDirection::Southwest })
			&& !body.IsExperiencingGravity())
		{
			body.SetAcceleration({ body.GetAcceleration().m_X, body.GetGravity() });
		}

		body.SetVelocityDelta(body.GetAcceleration() * deltaTime);

		//TOOD: surely there is a better way then frame velocities to handle outside velocities?
		float xVelocity = body.GetVelocity().m_X * deltaTime;;
		float yVelocity = body.GetVelocity().m_Y * deltaTime;;

		//LogWarning(std::format("ENTITY SETTING POS: {}", std::to_string(xVelocity), std::to_string(yVelocity)));
		entity.m_Transform.SetPosDelta(Vec2(xVelocity, yVelocity));

		/*
			LogError(std::format("Setting acceleration! Collisions: {} vel: {} vvel magnitude: {} ENTITY: {} has COLLIDING BODIES: {}",
				std::to_string(bodyA.GetTotalBodyCollisions()), bodyA.GetVelocity().ToString(),
				std::to_string(bodyA.GetVelocity().GetMagnitude()), bodyA.GetEntitySafe().m_Name, bodyA.ToStringCollidingBodies()));
			*/
	}

	void PhysicsWorld::UpdateEnd()
	{
	}

	Vec2 PhysicsWorld::GetCollisionNormal(const CollisionBoxData& boxA, const CollisionBoxData& boxB)
	{
		return (boxB.GetAABBCenterWorldPos() - boxA.GetAABBCenterWorldPos()).GetNormalized();
	}

	float PhysicsWorld::CalculateImpulse(const PhysicsBodyData& targetObject, const PhysicsBodyData& collidedObject, const Vec2& collisionNormal)
	{
		const float averageRestitution = (targetObject.GetPhysicsProfile().GetRestitution() +
									collidedObject.GetPhysicsProfile().GetRestitution()) / 2;

		const float& massA = targetObject.GetMass();
		const float& massB = collidedObject.GetMass();

		const Vec2& velA = targetObject.GetVelocity();
		const Vec2& velB = collidedObject.GetVelocity();
		//LogError(std::format("Dot product for{} dot {} is: {}", velA.ToString(), velB.ToString(),std::to_string(DotProduct(velA, velB)) ));
		return ((1 + averageRestitution) * massA * massB) / (massA + massB) * 
			   (DotProduct(velB, collisionNormal) - DotProduct(velA, collisionNormal));
	}

	bool DoBodiesIntersect(const PhysicsBodyData& body1, const PhysicsBodyData& body2)
	{
		/*return DoAABBIntersect(body1.m_Entity->m_Transform.m_Pos, body1.GetAABB(), 
							   body2.m_Entity->m_Transform.m_Pos, body2.GetAABB());*/
		return body1.GetCollisionBox().DoIntersect(body2.GetCollisionBox());
	}

	Vec2 GetBodyMinDisplacement(const PhysicsBodyData& body1, const PhysicsBodyData& body2)
	{
		/*return GetAABBMinDisplacement(body1.m_Entity->m_Transform.m_Pos, body1.GetAABB(),
			body2.m_Entity->m_Transform.m_Pos, body2.GetAABB());*/
		return body1.GetCollisionBox().GetAABBMinDisplacement(body2.GetCollisionBox());
	}

	RaycastInfo PhysicsWorld::Raycast(const WorldPosition& origin, const Vec2& ray) const
	{
		RaycastInfo result = {};
		int xSign = Utils::GetSign(ray.m_X);
		int ySign = Utils::GetSign(ray.m_Y);
		WorldPosition rayEndPoint = GetVectorEndPoint(origin, ray);
		Vec2 rayDir = ray.GetNormalized();

		WorldPosition boundsWorldMin = {};
		WorldPosition boundsWorldMax = {};
		
		float t1 = 0;
		float t2 = 0;

		//Since we consider t values from origin and onward (where tMin represents origin point and tMax represents ray end)
		float tMin = 0;
		float tMax = std::numeric_limits<float>::infinity();
		
		for (auto& body : m_bodies)
		{
			const AABB& currentBounds = body->GetCollisionBox().GetAABB();
			boundsWorldMin = currentBounds.GetGlobalMin(body->GetEntitySafe().m_Transform.m_Pos);
			boundsWorldMax= currentBounds.GetGlobalMax(body->GetEntitySafe().m_Transform.m_Pos);

			//TODO: perhaps optimizations could be made by checking to see if distance is too big to make it to this collider
			//so we can just continue

			//----------- X AXIS -----------
			//If the ray is vertical and outside box, there are no intersections here
			if (Utils::ApproximateEqualsF(ray.m_X, 0) &&
				(origin.m_X < boundsWorldMin.m_X || origin.m_X > boundsWorldMax.m_X))
			{
				/*LogError(std::format("PERFECT LINE origin: {} ray; {} body: {} (body pos: {}) body min: {} max: {}", 
					origin.ToString(), ray.ToString(), body->m_Entity->m_Name, body->GetEntitySafe().m_Transform.m_Pos.ToString(), 
					boundsWorldMin.ToString(), boundsWorldMax.ToString()));*/
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
			if (Utils::ApproximateEqualsF(ray.m_Y, 0) &&
				(origin.m_Y < boundsWorldMin.m_Y || origin.m_Y > boundsWorldMax.m_Y))
			{
				continue;
			}

			t1 = (boundsWorldMin.m_Y - origin.m_Y) / rayDir.m_Y;
			t2 = (boundsWorldMax.m_Y - origin.m_Y) / rayDir.m_Y;
			if (t1 > t2) std::swap(t1, t2);
			tMin = std::max(tMin, t1);
			tMax = std::min(tMax, t2);

			if (tMin > tMax) continue;

			
			//Note: when reaching this point tMin is the intersecction distance from origin
			result.m_BodyHit = body;
			result.m_Displacement = {rayDir.m_X * tMin, rayDir.m_Y * tMin};
			result.m_HitPos = origin + Vec2(result.m_Displacement.m_X, result.m_Displacement.m_Y);

			/*LogError(std::format("REACHED LINE END origin {} ray; {} body: {} (body pos: {}) body min: {} max: {} displacement; {}",
				origin.ToString(), ray.ToString(), body->m_Entity->m_Name, body->GetEntitySafe().m_Transform.m_Pos.ToString(),
				boundsWorldMin.ToString(), boundsWorldMax.ToString(), result.m_Displacement.ToString()));*/
			return result;
		}
		return result;
	}
}


