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
		m_bodies.push_back(&body);
	}

	void PhysicsWorld::Update(const float& deltaTime)
	{
#ifdef ENABLE_PROFILER
		ProfilerTimer timer("PhysicsWorld::Update");
#endif 

		bool foundCollision = false;
		for (int i = 0; i< m_bodies.size(); i++)
		{
			if (m_bodies[i] == nullptr) continue;

			PhysicsBodyData& bodyA = *(m_bodies[i]);
			ECS::Entity bodyAEntity = bodyA.GetEntitySafe();
			foundCollision = false;

			//Note: this could get optimized to not need double checking for each possible collision
			//but we need it in this time to ensure that velocity and other updates occur for BOTH bodies
			//and not just to one
			for (int j = 0; j < m_bodies.size(); j++)
			{
				if (j == i) continue;
				if (m_bodies[j] == nullptr) continue;

				PhysicsBodyData& bodyB = *(m_bodies[j]);
				ECS::Entity bodyBEntity = bodyB.GetEntitySafe();

				if (Physics::DoAABBIntersect(bodyAEntity.m_Transform.m_Pos, bodyA.GetAABB(), bodyBEntity.m_Transform.m_Pos, bodyB.GetAABB()))
				{
					foundCollision = true;
					break;
				}
			}

			Utils::LogWarning(std::format("COLLISION FOR ENTITY: {} FOUND: {}", bodyA.m_Entity->m_Name, std::to_string(foundCollision)));
			if (foundCollision)
			{
				bodyA.SetVelocity(bodyA.GetVelocity().GetNormal());
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


