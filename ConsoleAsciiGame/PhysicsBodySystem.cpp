#include "pch.hpp"
#include "PhysicsBodySystem.hpp"
#include "PhysicsBodyData.hpp"
#include "PhysicsWorld.hpp"
#include "PositionConversions.hpp"
#include "HelperFunctions.hpp"
#include "CameraData.hpp"
#include "Scene.hpp"

#ifdef ENABLE_PROFILER
#include "ProfilerTimer.hpp"
#endif 

namespace ECS
{
	static constexpr bool RENDER_COLLIDER_OUTLINES = true;
	static constexpr bool DRAW_BODY_VELOCITY_VECTORS = true;

	PhysicsBodySystem::PhysicsBodySystem(Physics::PhysicsManager& physicsManager) 
		: m_colliderOutlineBuffer(), m_lineBuffer(), m_physicsManager(physicsManager) {}

	void PhysicsBodySystem::SystemUpdate(Scene& scene, CameraData& mainCamera, const float& deltaTime)
	{
#ifdef ENABLE_PROFILER
		ProfilerTimer timer("PhysicsBodySystem::SystemUpdate");
#endif 

		//TODO: when physics world provides optimization then use it
		if (!RENDER_COLLIDER_OUTLINES) return;
		m_colliderOutlineBuffer.ClearAll();
		m_lineBuffer.clear();

		auto& bodies = m_physicsManager.GetPhysicsWorldMutable().GetBodiesMutable();
		ECS::Entity* entity = nullptr;
		for (auto& body : bodies)
		{
			if (body == nullptr) continue;

			entity = &(body->GetEntitySafeMutable());
			if (RENDER_COLLIDER_OUTLINES)
			{
				/*if (!Assert(this, mainCamera != nullptr, std::format("Tried to render collider outlines for entity: {} "
					"but the scene:{} has no active camera!", entity->GetName(), scene.GetName()))) return;*/

				//WorldPosition topLeftColliderPos = body->GetAABBTopLeftWorldPos();
				WorldPosition topLeftColliderPos = body->GetCollisionBox().GetAABBTopLeftWorldPos();
				//TODO: the camera should convert to screen pos not here
				ScreenPosition topLeftScreenPos = Conversions::WorldToScreenPosition(mainCamera, topLeftColliderPos);
				/*LogWarning(std::format("ADDING OUTLINE for entity: {} pos: {} top left collider: {} SCREEN TOP LEFT: {} half size: {}",
					entity.m_Name, entity.m_Transform.m_Pos.ToString(), topLeftColliderPos.ToString(), topLeftScreenPos.ToString(), body.GetAABB().GetHalfExtent().ToString()));*/

				//m_colliderOutlineBuffer.AddRectangle(RectangleOutlineData(body->GetAABB().GetSize(), topLeftScreenPos));
				m_colliderOutlineBuffer.AddRectangle(RectangleOutlineData(body->GetCollisionBox().GetAABB().GetSize(), topLeftScreenPos));
			}

			float velocityMagnitude = body->GetVelocity().GetMagnitude();
			if (DRAW_BODY_VELOCITY_VECTORS && !Utils::ApproximateEqualsF(velocityMagnitude, 0))
			{
				m_lineBuffer.emplace_back(entity->m_Transform.m_Pos,
					GetVectorEndPoint(entity->m_Transform.m_Pos, body->GetVelocity()));
			}
		}
	}

	const ColliderOutlineBuffer& PhysicsBodySystem::GetColliderBuffer() const
	{
		return m_colliderOutlineBuffer;
	}

	ColliderOutlineBuffer& PhysicsBodySystem::GetColliderBufferMutable()
	{
		return m_colliderOutlineBuffer;
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

