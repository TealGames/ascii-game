#include "pch.hpp"
#include "UISliderSystem.hpp"
#include "GlobalComponentInfo.hpp"
#include "EntityData.hpp"
#include "UIRendererComponent.hpp"
#include "UIPanel.hpp"

namespace ECS
{
	UISliderSystem::UISliderSystem()
	{
		GlobalComponentInfo::AddComponentInfo(typeid(UISliderComponent), ComponentInfo(RequiredComponentCheck<UIRendererData>(),
			[](EntityData& entity)-> void 
			{
				UISliderComponent& slider = *(entity.TryGetComponentMutable<UISliderComponent>());
				slider.m_renderer = entity.TryGetComponentMutable<UIRendererData>();
				slider.m_backgroundPanel = entity.TryGetComponentMutable<UIPanel>();
			}));
	}
}
