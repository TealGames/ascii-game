#include "pch.hpp"
#include "TriggerSystem.hpp"
#include "TriggerData.hpp"
#include "Scene.hpp"


#ifdef ENABLE_PROFILER
#include "ProfilerTimer.hpp"
#endif 

namespace ECS
{
	TriggerSystem::TriggerSystem() {}

	void TriggerSystem::SystemUpdate(Scene& scene, CameraData& mainCamera, const float& deltaTime)
	{
#ifdef ENABLE_PROFILER
		ProfilerTimer timer("TriggerSystem::SystemUpdate");
#endif 

		scene.OperateOnComponents<TriggerData>(
			[this, &scene, &deltaTime](TriggerData& data, ECS::Entity& entity)-> void
			{
				const auto& onEnterBodies = data.GetCollisionBox().GetCollisionEnterBoxes();
				LogError(std::format("Found enter bodies: {}", std::to_string(data.GetCollisionBox().GetAllCollisionBoxes().size())));
				if (!onEnterBodies.empty())
				{
					for (const auto& body : onEnterBodies) data.m_OnEnter.Invoke(body);
				}

				const auto& onExitBodies = data.GetCollisionBox().GetCollisionExitBoxes();
				if (!onExitBodies.empty())
				{
					for (const auto& body : onExitBodies) data.m_OnExit.Invoke(body);
				}
			});
	}
}
