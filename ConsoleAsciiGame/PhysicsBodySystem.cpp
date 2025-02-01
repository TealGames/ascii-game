#include "pch.hpp"
#include "PhysicsBodySystem.hpp"
#include "ProfilerTimer.hpp"
#include "PhysicsBodyData.hpp"
#include "PhysicsWorld.hpp"
#include "InputData.hpp"
#include "PositionConversions.hpp"
#include "HelperFunctions.hpp"
#include "CameraData.hpp"

namespace ECS
{
	static constexpr bool RENDER_COLLIDER_OUTLINES = true;
	static constexpr bool DRAW_BODY_VELOCITY_VECTORS = true;

	PhysicsBodySystem::PhysicsBodySystem() : m_colliderOutlineBuffer() {}

	void PhysicsBodySystem::SystemUpdate(Scene& scene, const float& deltaTime)
	{
#ifdef ENABLE_PROFILER
		ProfilerTimer timer("PhysicsBodySystem::SystemUpdate");
#endif 

		//TODO: when physics world provides optimization then use it
		Physics::PhysicsWorld& world = scene.GetPhysicsWorldMutable();
		auto& bodies = world.GetBodiesMutable();

		//TODO: this could probably be optimized to not clear static objects (ones that do not move)
		if (!RENDER_COLLIDER_OUTLINES) return;
		m_colliderOutlineBuffer.ClearAll();
		m_lineBuffer.clear();

		CameraData* mainCamera = scene.TryGetMainCameraData();
		scene.OperateOnComponents<PhysicsBodyData>(
			[this, &scene, &deltaTime, &mainCamera](PhysicsBodyData& body, ECS::Entity& entity)-> void
			{
				if (RENDER_COLLIDER_OUTLINES)
				{
					if (!Utils::Assert(this, mainCamera != nullptr, std::format("Tried to render collider outlines for entity: {} "
						"but the scene:{} has no active camera!", entity.m_Name, scene.m_SceneName))) return;

					WorldPosition topLeftColliderPos = body.GetAABBTopLeftWorldPos(entity.m_Transform.m_Pos);
					//TODO: the camera should convert to screen pos not here
					ScreenPosition topLeftScreenPos = Conversions::WorldToScreenPosition(*mainCamera, topLeftColliderPos);
					/*Utils::LogWarning(std::format("ADDING OUTLINE for entity: {} pos: {} top left collider: {} SCREEN TOP LEFT: {} half size: {}", 
						entity.m_Name, entity.m_Transform.m_Pos.ToString(), topLeftColliderPos.ToString(), topLeftScreenPos.ToString(), body.GetAABB().GetHalfExtent().ToString()));*/

					m_colliderOutlineBuffer.AddRectangle(RectangleOutlineData(body.GetAABB().GetSize(), topLeftScreenPos));
				}

				float velocityMagnitude = body.GetVelocity().GetMagnitude();
				if (DRAW_BODY_VELOCITY_VECTORS && !Utils::ApproximateEqualsF(velocityMagnitude, 0))
				{
					m_lineBuffer.emplace_back(entity.m_Transform.m_Pos, 
						GetVectorEndPoint(entity.m_Transform.m_Pos, body.GetVelocity()));
				}
			});
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

