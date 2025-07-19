#include "pch.hpp"
#include "ECS/Systems/Types/UI/UISliderSystem.hpp"
#include "ECS/Component/GlobalComponentInfo.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "ECS/Component/Types/UI/UIRendererComponent.hpp"
#include "ECS/Component/Types/UI/UIPanel.hpp"
#include "ECS/Component/Types/UI/UISelectableData.hpp"

namespace ECS
{
	UISliderSystem::UISliderSystem() {}
	void UISliderSystem::Init()
	{
		GlobalComponentInfo::AddComponentInfo(typeid(UISliderComponent),
			ComponentInfo(CreateComponentTypes<UIRendererData, UIPanel, UISelectableData>(),
				CreateRequiredComponentFunction(UIRendererData(), UIPanel(), UISelectableData()),
				[](EntityData& entity)-> void
				{
					UISliderComponent& slider = *(entity.TryGetComponentMutable<UISliderComponent>());
					slider.m_renderer = entity.TryGetComponentMutable<UIRendererData>();
					slider.m_backgroundPanel = entity.TryGetComponentMutable<UIPanel>();
					slider.m_selectable = entity.TryGetComponentMutable<UISelectableData>();

					slider.Init();
				}));
	}
}
