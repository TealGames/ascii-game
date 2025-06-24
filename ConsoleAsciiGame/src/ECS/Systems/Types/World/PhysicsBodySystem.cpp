#include "pch.hpp"
#include "PhysicsBodySystem.hpp"
#include "PhysicsBodyData.hpp"
#include "PhysicsWorld.hpp"
#include "PositionConversions.hpp"
#include "HelperFunctions.hpp"
#include "Scene.hpp"
#include "GlobalComponentInfo.hpp"

#ifdef ENABLE_PROFILER
#include "ProfilerTimer.hpp"
#endif 

namespace ECS
{
	static constexpr bool DRAW_BODY_VELOCITY_VECTORS = true;

	PhysicsBodySystem::PhysicsBodySystem(Physics::PhysicsManager& physicsManager) 
		: m_lineBuffer(), m_physicsManager(physicsManager) 
	{
		GlobalComponentInfo::AddComponentInfo(typeid(PhysicsBodySystem), 
			ComponentInfo(DeppendencyType::None, {}, CreateRequiredComponentFunction<CollisionBoxData>()));
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

