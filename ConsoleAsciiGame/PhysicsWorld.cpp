#include "pch.hpp"
#include "PhysicsWorld.hpp"
#include "Entity.hpp"
#include "ProfilerTimer.hpp"
#include "Point2D.hpp"
#include "HelperFunctions.hpp"
#include "Debug.hpp"

namespace Physics
{
	PhysicsWorld::PhysicsWorld() : m_bodies{} {}

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
		body.SetPhysicsWorld(*this);
		m_bodies.push_back(&body);
	}

	void PhysicsWorld::ClearAllBodies()
	{
		m_bodies = {};
	}

	PhysicsBodyCollection PhysicsWorld::FindBodiesContainingPos(const WorldPosition& worldPos) const
	{
		PhysicsBodyCollection bodiesFound = {};
		for (const auto& body : m_bodies)
		{
			if (body->DoesAABBContainPos(worldPos))
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

		AABBIntersectionData collision = {};
		Vec2 minBodyDisplacement = {};
		Vec2 minBodyDisplacementVec = {};
		for (int i = 0; i< m_bodies.size(); i++)
		{
			if (m_bodies[i] == nullptr) continue;

			PhysicsBodyData& bodyA = *(m_bodies[i]);
			ECS::Entity& bodyAEntity = bodyA.GetEntitySafeMutable();

			//We do not need to simulate bodies with no movement since collisions are resolved on bodies that move
			//if (Utils::ApproximateEqualsF(bodyA.GetVelocity().GetMagnitude(), 0)) continue;
			collision = {};

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
				collision = Physics::GetAABBIntersectionData(bodyBEntity.m_Transform.m_Pos, bodyB.GetAABB(), bodyAEntity.m_Transform.m_Pos, bodyA.GetAABB());
				if (collision.m_DoIntersect)
				{
					//if (!Utils::ApproximateEqualsF(bodyA.GetVelocity().GetMagnitude(), 0))
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
						/*throw new std::invalid_argument("yeah");*/
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

			/*LogWarning(std::format("COLLISION FOR ENTITY: {} FOUND: {} DEPTH: {}", bodyA.m_Entity->m_Name, 
				std::to_string(collision.m_DoIntersect), collision.m_Depth.ToString()));*/
			/*
			LogError(std::format("Setting acceleration! Collisions: {} vel: {} vvel magnitude: {} ENTITY: {} has COLLIDING BODIES: {}", 
				std::to_string(bodyA.GetTotalBodyCollisions()), bodyA.GetVelocity().ToString(), 
				std::to_string(bodyA.GetVelocity().GetMagnitude()), bodyA.GetEntitySafe().m_Name, bodyA.ToStringCollidingBodies()));
			*/

			if (!bodyA.IsCollidingWithBodyInDirs({MoveDirection::South, MoveDirection::Southeast, MoveDirection::Southwest}) 
				&& !bodyA.IsExperiencingGravity())
			{
				bodyA.SetAcceleration({ bodyA.GetAcceleration().m_X, bodyA.GetGravity() });
			}

			//TODO: all of the code below here should probably be extracted out into separate functions later

			bodyA.SetVelocityDelta(bodyA.GetAcceleration() * deltaTime);

			//TOOD: surely there is a better way then frame velocities to handle outside velocities?
			float xVelocity = bodyA.GetVelocity().m_X * deltaTime;;
			float yVelocity = bodyA.GetVelocity().m_Y * deltaTime;;

			//LogWarning(std::format("ENTITY SETTING POS: {}", std::to_string(xVelocity), std::to_string(yVelocity)));
			bodyAEntity.m_Transform.SetPosDelta(Vec2(xVelocity, yVelocity));
		}
	}
	void PhysicsWorld::UpdateEnd()
	{
	}

	float PhysicsWorld::CalculateImpulse(const PhysicsBodyData& targetObject, const PhysicsBodyData& collidedObject, const Vec2& collisionNormal)
	{
		float averageRestitution = (targetObject.GetPhysicsProfile().GetRestitution() +
									collidedObject.GetPhysicsProfile().GetRestitution()) / 2;

		const float& massA = targetObject.GetMass();
		const float& massB = collidedObject.GetMass();

		const Vec2 velA = targetObject.GetVelocity();
		const Vec2 velB = collidedObject.GetVelocity();
		//LogError(std::format("Dot product for{} dot {} is: {}", velA.ToString(), velB.ToString(),std::to_string(DotProduct(velA, velB)) ));
		return ((1 + averageRestitution) * massA * massB) / (massA + massB) * 
			   (DotProduct(velB, collisionNormal) - DotProduct(velA, collisionNormal));
	}

	bool DoBodiesIntersect(const PhysicsBodyData& body1, const PhysicsBodyData& body2)
	{
		return DoAABBIntersect(body1.m_Entity->m_Transform.m_Pos, body1.GetAABB(), 
							   body2.m_Entity->m_Transform.m_Pos, body2.GetAABB());
	}

	Vec2 GetBodyMinDisplacement(const PhysicsBodyData& body1, const PhysicsBodyData& body2)
	{
		return GetAABBMinDisplacement(body1.m_Entity->m_Transform.m_Pos, body1.GetAABB(),
			body2.m_Entity->m_Transform.m_Pos, body2.GetAABB());
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
			const AABB& currentBounds = body->GetAABB();
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


