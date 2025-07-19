#include "pch.hpp"
#include "ECS/Systems/Types/World/TriggerSystem.hpp"
#include "ECS/Component/Types/World/TriggerData.hpp"
#include "Core/Scene/Scene.hpp"
#include "ECS/Component/GlobalComponentInfo.hpp"


#ifdef ENABLE_PROFILER
#include "Core/Analyzation/ProfilerTimer.hpp"
#endif 

namespace ECS
{
	TriggerSystem::TriggerSystem() 
	{
		GlobalComponentInfo::AddComponentInfo(typeid(TriggerData),
			ComponentInfo(CreateComponentTypes<CollisionBoxData>(), CreateRequiredComponentFunction(CollisionBoxData()),
				[](EntityData& entity)-> void
				{
					TriggerData& trigger = *(entity.TryGetComponentMutable<TriggerData>());
					if (trigger.m_collider != nullptr) trigger.m_collider = entity.TryGetComponent<CollisionBoxData>();
					//fieldComponent.m_background = entity.TryGetComponentMutable<UIPanel>();
				}));
	}

	void TriggerSystem::SystemUpdate(Scene& scene, CameraData& mainCamera, const float& deltaTime)
	{
#ifdef ENABLE_PROFILER
		ProfilerTimer timer("TriggerSystem::SystemUpdate");
#endif 

		scene.OperateOnComponents<TriggerData>(
			[this, &scene, &deltaTime](TriggerData& data)-> void
			{
				const auto& onEnterBodies = data.GetCollisionBox().GetCollisionEnterBoxes();
				//LogError(std::format("Found enter bodies: {}", std::to_string(data.GetCollisionBox().GetAllCollisionBoxes().size())));
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
