#include "pch.hpp"
#include "PhysicsBodySystem.hpp"
#include "ProfilerTimer.hpp"
#include "PhysicsBodyData.hpp"
#include "PhysicsWorld.hpp"
#include "InputData.hpp"

namespace ECS
{
	PhysicsBodySystem::PhysicsBodySystem() {}

	void PhysicsBodySystem::SystemUpdate(Scene& scene, const float& deltaTime)
	{
#ifdef ENABLE_PROFILER
		ProfilerTimer timer("PhysicsBodySystem::SystemUpdate");
#endif 

		//TODO: when physics world provides optimization then use it
		Physics::PhysicsWorld& world = scene.GetPhysicsWorldMutable();
		auto& bodies = world.GetBodiesMutable();

		/*scene.OperateOnComponents<PhysicsBodyData>(
			[this, &scene, &deltaTime](PhysicsBodyData& body, ECS::Entity& entity)-> void
			{
				float xVelocity = body.GetVelocity().m_X * deltaTime;;
				float yVelocity = body.GetVelocity().m_Y * deltaTime;;
				Utils::LogWarning(std::format("ENTITY SETTING POS: {}", std::to_string(xVelocity), std::to_string(yVelocity)));
				entity.m_Transform.m_Pos.m_X += xVelocity;
				entity.m_Transform.m_Pos.m_Y += yVelocity;

				body.SetVelocityXDelta(body.GetAcceleration().m_X * deltaTime);
				body.SetVelocityYDelta(body.GetAcceleration().m_Y * deltaTime);
			});

		return;*/

		bool foundCollision = false;
		for (int i = 0; bodies.size(); i++)
		{
			if (bodies[i] == nullptr) continue;

			PhysicsBodyData& bodyA = *(bodies[i]);
			ECS::Entity bodyAEntity = bodyA.GetEntitySafe();
			foundCollision = false;

			for (int j = i + 1; j < bodies.size(); j++)
			{
				if (bodies[j] == nullptr) continue;

				PhysicsBodyData& bodyB = *(bodies[j]);
				ECS::Entity bodyBEntity = bodyB.GetEntitySafe();

				if (Physics::DoAABBIntersect(bodyAEntity.m_Transform.m_Pos, bodyA.GetAABB(), bodyBEntity.m_Transform.m_Pos, bodyB.GetAABB()))
				{
					foundCollision = true;
					break;
				}
			}

			if (foundCollision) continue;

			float xVelocity = bodyA.GetVelocity().m_X * deltaTime;;
			float yVelocity = bodyA.GetVelocity().m_Y * deltaTime;;
			Utils::LogWarning(std::format("ENTITY SETTING POS: {}", std::to_string(xVelocity), std::to_string(yVelocity)));
			bodyAEntity.m_Transform.m_Pos.m_X += xVelocity;
			bodyAEntity.m_Transform.m_Pos.m_Y += yVelocity;

			bodyA.SetVelocityXDelta(bodyA.GetAcceleration().m_X * deltaTime);
			bodyA.SetVelocityYDelta(bodyA.GetAcceleration().m_Y * deltaTime);
		}
	}
}

