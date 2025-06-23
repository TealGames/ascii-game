#include "pch.hpp"
#include "UIPanelSystem.hpp"
#include "GlobalComponentInfo.hpp"
#include "UIRendererComponent.hpp"
#include "EntityData.hpp"

namespace ECS
{
	UIPanelSystem::UIPanelSystem() 
	{
		GlobalComponentInfo::AddComponentInfo(typeid(UIPanel), ComponentInfo(RequiredComponentCheck<UIRendererData>(),
			[](EntityData& entity)-> void {entity.TryGetComponentMutable<UIPanel>()->m_renderer = entity.TryGetComponentMutable<UIRendererData>(); }));
	}
}
