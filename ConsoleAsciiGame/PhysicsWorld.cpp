#include "pch.hpp"
#include "PhysicsWorld.hpp"
#include "Entity.hpp"
#include "ProfilerTimer.hpp"

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
		Utils::LogWarning(this, std::format("Adding body: {}", body.m_Entity->m_Name));
		m_bodies.push_back(&body);
	}

	void PhysicsWorld::Update(const float& deltaTime)
	{
#ifdef ENABLE_PROFILER
		ProfilerTimer timer("PhysicsWorld::Update");
#endif 

		AABBIntersectionData collision = {};
		for (int i = 0; i< m_bodies.size(); i++)
		{
			if (m_bodies[i] == nullptr) continue;

			PhysicsBodyData& bodyA = *(m_bodies[i]);
			ECS::Entity& bodyAEntity = bodyA.GetEntitySafe();

			//We do not need to simulate bodies with no movement since collisions are resolved on bodies that move
			if (Utils::ApproximateEqualsF(bodyA.GetVelocity().GetMagnitude(), 0)) continue;
			collision = {};

			//Note: this could get optimized to not need double checking for each possible collision
			//but we need it in this time to ensure that velocity and other updates occur for BOTH bodies
			//and not just to one
			for (int j = 0; j < m_bodies.size(); j++)
			{
				if (j == i) continue;
				if (m_bodies[j] == nullptr) continue;

				PhysicsBodyData& bodyB = *(m_bodies[j]);
				ECS::Entity& bodyBEntity = bodyB.GetEntitySafe();

				//Intersection is handled as body1 being the non-moving body and body2 as the body moving into the bounding area of body1
				collision = Physics::GetAABBIntersectionData(bodyBEntity.m_Transform.m_Pos, bodyB.GetAABB(), bodyAEntity.m_Transform.m_Pos, bodyA.GetAABB());
				//Utils::LogWarning(std::format("BodyB: {} transform:{}", bodyB.GetAABB().m_MaxPos.ToString(), bodyBEntity.m_Name));
				if (collision.m_DoIntersect) break;
			}

			/*Utils::LogWarning(std::format("COLLISION FOR ENTITY: {} FOUND: {} DEPTH: {}", bodyA.m_Entity->m_Name, 
				std::to_string(collision.m_DoIntersect), collision.m_Depth.ToString()));*/

			if (collision.m_DoIntersect)
			{
				bool xIsMin = std::abs(collision.m_Depth.m_X) < std::abs(collision.m_Depth.m_Y);
				float moveDelta = -(xIsMin ? collision.m_Depth.m_X : collision.m_Depth.m_Y);

				if (xIsMin) bodyAEntity.m_Transform.m_Pos.m_X += moveDelta;
				else bodyAEntity.m_Transform.m_Pos.m_Y += moveDelta;

				//bodyA.SetVelocity(collision.m_Depth.GetOppositeDirection());
			}

			float xVelocity = bodyA.GetVelocity().m_X * deltaTime;;
			float yVelocity = bodyA.GetVelocity().m_Y * deltaTime;;
			//Utils::LogWarning(std::format("ENTITY SETTING POS: {}", std::to_string(xVelocity), std::to_string(yVelocity)));
			bodyAEntity.m_Transform.m_Pos.m_X += xVelocity;
			bodyAEntity.m_Transform.m_Pos.m_Y += yVelocity;

			bodyA.SetVelocityXDelta(bodyA.GetAcceleration().m_X * deltaTime);
			bodyA.SetVelocityYDelta(bodyA.GetAcceleration().m_Y * deltaTime);
		}
	}

	bool DoBodiesIntersect(const PhysicsBodyData& body1, const PhysicsBodyData& body2)
	{
		return DoAABBIntersect(body1.m_Entity->m_Transform.m_Pos, body1.GetAABB(), 
							   body2.m_Entity->m_Transform.m_Pos, body2.GetAABB());
	}
}


