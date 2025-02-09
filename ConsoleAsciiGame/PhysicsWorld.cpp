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

	const BodyCollectionType& PhysicsWorld::GetBodies() const
	{
		return m_bodies;
	}

	BodyCollectionType& PhysicsWorld::GetBodiesMutable()
	{
		return m_bodies;
	}

	void PhysicsWorld::AddBody(PhysicsBodyData& body)
	{
		LogWarning(this, std::format("Adding body: {}", body.m_Entity->m_Name));
		m_bodies.push_back(&body);
	}

	void PhysicsWorld::UpdateStart(const float& deltaTime)
	{
#ifdef ENABLE_PROFILER
		ProfilerTimer timer("PhysicsWorld::Update");
#endif 

		AABBIntersectionData collision = {};
		Utils::Point2D minBodyDisplacement = {};
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

				//Intersection is handled as body1 being the non-moving body and body2 as the body moving into the bounding area of body1
				collision = Physics::GetAABBIntersectionData(bodyBEntity.m_Transform.m_Pos, bodyB.GetAABB(), bodyAEntity.m_Transform.m_Pos, bodyA.GetAABB());
				if (collision.m_DoIntersect)
				{
					if (!Utils::ApproximateEqualsF(bodyA.GetVelocity().GetMagnitude(), 0))
					{
						bool xIsMin = std::abs(collision.m_Depth.m_X) < std::abs(collision.m_Depth.m_Y);
						float moveDelta = -(xIsMin ? collision.m_Depth.m_X : collision.m_Depth.m_Y);
						//We add the extra distance so that they are still considered as colliding but would not cause any awkward jittering
						//moveDelta += Utils::GetSign(moveDelta) * MAX_DISTANCE_FOR_COLLISION;

						if (xIsMin) bodyAEntity.m_Transform.SetPosDeltaX(moveDelta);
						else bodyAEntity.m_Transform.SetPosDeltaY(moveDelta);

						bodyA.SetAcceleration(Vec2::ZERO);
						bodyA.SetVelocity(Vec2::ZERO);

						minBodyDisplacement = GetBodyMinDisplacement(bodyB, bodyA);
						minBodyDisplacementVec = { std::abs(minBodyDisplacement.m_X), std::abs(minBodyDisplacement.m_Y) };
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

			if (!bodyA.IsCollidingWithBodyInDir(Direction::Down) && !Utils::ApproximateEqualsF(bodyA.GetGravity(), 0))
			{
				bodyA.SetAcceleration({ bodyA.GetAcceleration().m_X, bodyA.GetGravity() });
			}

			//TODO: all of the code below here should probably be extracted out into separate functions later

			bodyA.SetVelocityDelta(bodyA.GetAcceleration() * deltaTime);

			//TOOD: surely there is a better way then frame velocities to handle outside velocities?
			float xVelocity = bodyA.GetVelocity().m_X * deltaTime;;
			float yVelocity = bodyA.GetVelocity().m_Y * deltaTime;;

			//LogWarning(std::format("ENTITY SETTING POS: {}", std::to_string(xVelocity), std::to_string(yVelocity)));
			bodyAEntity.m_Transform.SetPosDelta(Utils::Point2D(xVelocity, yVelocity));
		}
	}
	void PhysicsWorld::UpdateEnd()
	{
	}

	bool DoBodiesIntersect(const PhysicsBodyData& body1, const PhysicsBodyData& body2)
	{
		return DoAABBIntersect(body1.m_Entity->m_Transform.m_Pos, body1.GetAABB(), 
							   body2.m_Entity->m_Transform.m_Pos, body2.GetAABB());
	}

	Utils::Point2D GetBodyMinDisplacement(const PhysicsBodyData& body1, const PhysicsBodyData& body2)
	{
		return GetAABBMinDisplacement(body1.m_Entity->m_Transform.m_Pos, body1.GetAABB(),
			body2.m_Entity->m_Transform.m_Pos, body2.GetAABB());
	}

	RaycastInfo PhysicsWorld::Raycast(const WorldPosition& origin, const Vec2& ray)
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
		float tMin = 0.0f;
		float tMax = std::numeric_limits<float>::infinity();
		
		for (const auto& body : m_bodies)
		{
			const AABB& currentBounds = body->GetAABB();
			boundsWorldMin = currentBounds.GetGlobalMin(body->GetEntitySafe().m_Transform.m_Pos);
			boundsWorldMax= currentBounds.GetGlobalMin(body->GetEntitySafe().m_Transform.m_Pos);

			//TODO: perhaps optimizations could be made by checking to see if distance is too big to make it to this collider
			//so we can just continue

			//----------- X AXIS -----------
			//If the ray is vertical and outside box, there are no intersections here
			if (Utils::ApproximateEqualsF(ray.m_X, 0) &&
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

			if (tMin > tMax) continue;
			
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
			result.m_HitPos = origin + Utils::Point2D(result.m_Displacement.m_X, result.m_Displacement.m_Y);
			return result;
		}
		return result;
	}
}


