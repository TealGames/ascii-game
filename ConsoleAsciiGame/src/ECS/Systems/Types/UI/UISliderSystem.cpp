#include "pch.hpp"
#include "ECS/Systems/Types/UI/UISliderSystem.hpp"
#include "ECS/Component/GlobalComponentInfo.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "ECS/Component/Types/UI/UIRendererComponent.hpp"
#include "ECS/Component/Types/UI/UIPanel.hpp"

namespace ECS
{
	UISliderSystem::UISliderSystem()
	{
		GlobalComponentInfo::AddComponentInfo(typeid(UISliderComponent), ComponentInfo(CreateRequiredComponentFunction<UIRendererData>(),
			[](EntityData& entity)-> void 
			{
				UISliderComponent& slider = *(entity.TryGetComponentMutable<UISliderComponent>());
				slider.m_renderer = entity.TryGetComponentMutable<UIRendererData>();
				slider.m_backgroundPanel = entity.TryGetComponentMutable<UIPanel>();
			}));
	}
}
