#include "pch.hpp"
#include "ECS/Systems/Types/World/TriggerSystem.hpp"
#include "ECS/Component/Types/World/TriggerComponent.hpp"
#include "Core/Scene/Scene.hpp"
#include "ECS/Component/GlobalComponentInfo.hpp"

#ifdef ENABLE_PROFILER
#include "Core/Analyzation/ProfilerTimer.hpp"
#endif 

namespace Engine::World
{
	TriggerSystem::TriggerSystem() 
	{
		ECS::GlobalComponentInfo::AddComponentInfo(typeid(TriggerComponent),
			ECS::ComponentInfo(ECS::CreateComponentTypes<CollisionBoxComponent>(), CreateRequiredComponentFunction(CollisionBoxComponent()),
				[](ECS::EntityData& entity)-> void
				{
					TriggerComponent& trigger = *(entity.TryGetComponentMutable<TriggerComponent>());
					if (trigger.m_collider != nullptr) trigger.m_collider = entity.TryGetComponent<CollisionBoxComponent>();
					//fieldComponent.m_background = entity.TryGetComponentMutable<UIPanel>();
				}));
	}

	void TriggerSystem::SystemUpdate(Scenes::Scene& scene, Camera::CameraComponent& mainCamera, const float& deltaTime)
	{
#ifdef ENABLE_PROFILER
		ProfilerTimer timer("TriggerSystem::SystemUpdate");
#endif 

		scene.OperateOnActiveComponents<TriggerComponent>(
			[this, &scene, &deltaTime](TriggerComponent& data)-> void
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
