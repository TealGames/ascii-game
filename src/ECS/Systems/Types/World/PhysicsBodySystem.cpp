#include "pch.hpp"
#include "ECS/Systems/Types/World/PhysicsBodySystem.hpp"
#include "ECS/Component/Types/World/PhysicsBodyData.hpp"
#include "Core/Physics/PhysicsWorld.hpp"
#include "Core/PositionConversions.hpp"
#include "Utils/HelperFunctions.hpp"
#include "Core/Scene/Scene.hpp"
#include "ECS/Component/GlobalComponentInfo.hpp"

#ifdef ENABLE_PROFILER
#include "Core/Analyzation/ProfilerTimer.hpp"
#endif 

namespace ECS
{
	static constexpr bool DRAW_BODY_VELOCITY_VECTORS = true;

	PhysicsBodySystem::PhysicsBodySystem(Physics::PhysicsManager& physicsManager) 
		: m_lineBuffer(), m_physicsManager(physicsManager) 
	{
		GlobalComponentInfo::AddComponentInfo(typeid(PhysicsBodyData), 
			ComponentInfo(CreateComponentTypes<CollisionBoxData>(), CreateRequiredComponentFunction(CollisionBoxData()),
				[](EntityData& entity)-> void
				{
					PhysicsBodyData& body = *(entity.TryGetComponentMutable<PhysicsBodyData>());
					//Note: since we allow assigning collider from physics body, we only set it if it was not set from constructor
					if (body.m_collider == nullptr)
					{
						body.m_collider = entity.TryGetComponent<CollisionBoxData>();
						if (entity.m_Name == "player") LogError(std::format("Set player collider:{}", body.m_collider->ToString()));
					}
				}));
	}

	void PhysicsBodySystem::SystemUpdate(Scene& scene, CameraData& mainCamera, const float& deltaTime)
	{
#ifdef ENABLE_PROFILER
		ProfilerTimer timer("PhysicsBodySystem::SystemUpdate");
#endif 
		if (deltaTime <= 0) return;

		//TODO: when physics world provides optimization then use it
		//if (!RENDER_COLLIDER_OUTLINES) return;
		//m_colliderOutlineBuffer.ClearAll();
		m_lineBuffer.clear();

		auto& bodies = m_physicsManager.GetPhysicsWorldMutable().GetBodiesMutable();
		EntityData* entity = nullptr;
		for (auto& body : bodies)
		{
			if (body == nullptr) continue;

			entity = &(body->GetEntityMutable());

			float velocityMagnitude = body->GetVelocity().GetMagnitude();
			if (DRAW_BODY_VELOCITY_VECTORS && !Utils::ApproximateEqualsF(velocityMagnitude, 0))
			{
				m_lineBuffer.emplace_back(entity->GetTransformMutable().GetGlobalPos(),
					GetVectorEndPoint(entity->GetTransformMutable().GetGlobalPos(), body->GetVelocity()));
			}
		}
	}

	const LineBuffer& PhysicsBodySystem::GetLineBuffer() const
	{
		return m_lineBuffer;
	}

	LineBuffer& PhysicsBodySystem::GetLineBufferMutable()
	{
		return m_lineBuffer;
	}
}

