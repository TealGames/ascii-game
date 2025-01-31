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

	PhysicsBodySystem::PhysicsBodySystem() : m_ColliderOutlineBuffer() {}

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
		m_ColliderOutlineBuffer.ClearAll();

		CameraData* mainCamera = scene.TryGetMainCameraData();
		scene.OperateOnComponents<PhysicsBodyData>(
			[this, &scene, &deltaTime, &mainCamera](PhysicsBodyData& body, ECS::Entity& entity)-> void
			{
				if (RENDER_COLLIDER_OUTLINES)
				{
					if (!Utils::Assert(this, mainCamera != nullptr, std::format("Tried to render collider outlines for entity: {} "
						"but the scene:{} has no active camera!", entity.m_Name, scene.m_SceneName))) return;

					WorldPosition topLeftColliderPos = body.GetAABBTopLeftWorldPos(entity.m_Transform.m_Pos);
					ScreenPosition topLeftScreenPos = Conversions::WorldToScreenPosition(*mainCamera, topLeftColliderPos);
					Utils::LogWarning(std::format("ADDING OUTLINE for entity: {} pos: {} top left collider: {} SCREEN TOP LEFT: {} half size: {}", 
						entity.m_Name, entity.m_Transform.m_Pos.ToString(), topLeftColliderPos.ToString(), topLeftScreenPos.ToString(), body.GetAABB().GetHalfExtent().ToString()));

					m_ColliderOutlineBuffer.AddRectangle(RectangleOutlineData(body.GetAABB().GetSize(), topLeftScreenPos));
				}
			});
	}

	const ColliderOutlineBuffer* PhysicsBodySystem::TryGetColliderBuffer() const
	{
		return &m_ColliderOutlineBuffer;
	}

	ColliderOutlineBuffer* PhysicsBodySystem::TryGetColliderBufferMutable()
	{
		return &m_ColliderOutlineBuffer;
	}
}

