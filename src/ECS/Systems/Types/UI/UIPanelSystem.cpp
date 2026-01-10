#include "pch.hpp"
#include "ECS/Systems/Types/UI/UIPanelSystem.hpp"
#include "ECS/Component/GlobalComponentInfo.hpp"
#include "ECS/Component/Types/UI/UIRendererComponent.hpp"
#include "ECS/Component/Types/World/EntityComponent.hpp"

namespace ECS
{
	UIPanelSystem::UIPanelSystem() {}

	void UIPanelSystem::Init()
	{
		GlobalComponentInfo::AddComponentInfo(typeid(UIPanelComponent),
			ComponentInfo(CreateComponentTypes<UIRendererData>(), CreateRequiredComponentFunction(UIRendererData()),
				[](EntityData& entity)-> void
				{
					entity.TryGetComponentMutable<UIPanelComponent>()->m_renderer = entity.TryGetComponentMutable<UIRendererData>();
				}));
	}
}
