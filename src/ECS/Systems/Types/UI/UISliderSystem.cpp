#include "pch.hpp"
#include "ECS/Systems/Types/UI/UISliderSystem.hpp"
#include "ECS/Component/GlobalComponentInfo.hpp"
#include "ECS/Component/Types/World/EntityComponent.hpp"
#include "ECS/Component/Types/UI/UIRendererComponent.hpp"
#include "ECS/Component/Types/UI/UIPanelComponent.hpp"
#include "ECS/Component/Types/UI/UISelectableData.hpp"

namespace ECS
{
	UISliderSystem::UISliderSystem() {}
	void UISliderSystem::Init()
	{
		GlobalComponentInfo::AddComponentInfo(typeid(UISliderComponent),
			ComponentInfo(CreateComponentTypes<UIRendererData, UIPanelComponent, UISelectableData>(),
				CreateRequiredComponentFunction(UIRendererData(), UIPanelComponent(), UISelectableData()),
				[](EntityData& entity)-> void
				{
					UISliderComponent& slider = *(entity.TryGetComponentMutable<UISliderComponent>());
					slider.m_renderer = entity.TryGetComponentMutable<UIRendererData>();
					slider.m_backgroundPanel = entity.TryGetComponentMutable<UIPanelComponent>();
					slider.m_selectable = entity.TryGetComponentMutable<UISelectableData>();

					slider.Init();
				}));
	}
}
