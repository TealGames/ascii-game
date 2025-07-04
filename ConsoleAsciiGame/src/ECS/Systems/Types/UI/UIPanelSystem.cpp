#include "pch.hpp"
#include "ECS/Systems/Types/UI/UIPanelSystem.hpp"
#include "ECS/Component/GlobalComponentInfo.hpp"
#include "ECS/Component/Types/UI/UIRendererComponent.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"

namespace ECS
{
	UIPanelSystem::UIPanelSystem() {}

	void UIPanelSystem::Init()
	{
		GlobalComponentInfo::AddComponentInfo(typeid(UIPanel),
			ComponentInfo(CreateComponentTypes<UIRendererData>(), CreateRequiredComponentFunction(UIRendererData()),
				[](EntityData& entity)-> void
				{
					entity.TryGetComponentMutable<UIPanel>()->m_renderer = entity.TryGetComponentMutable<UIRendererData>();
				}));
	}
}
