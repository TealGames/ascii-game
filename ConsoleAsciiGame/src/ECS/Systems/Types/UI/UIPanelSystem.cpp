#include "pch.hpp"
#include "ECS/Systems/Types/UI/UIPanelSystem.hpp"
#include "ECS/Component/GlobalComponentInfo.hpp"
#include "ECS/Component/Types/UI/UIRendererComponent.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"

namespace ECS
{
	UIPanelSystem::UIPanelSystem() 
	{
		GlobalComponentInfo::AddComponentInfo(typeid(UIPanel), ComponentInfo(CreateRequiredComponentFunction<UIRendererData>(),
			[](EntityData& entity)-> void {entity.TryGetComponentMutable<UIPanel>()->m_renderer = entity.TryGetComponentMutable<UIRendererData>(); }));
	}
}
