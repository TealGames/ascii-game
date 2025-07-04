#include "pch.hpp"
#include "ECS/Systems/Types/Editor/UIColorPickerSystem.hpp"
#include "ECS/Component/GlobalComponentInfo.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "ECS/Component/Types/UI/UIRendererComponent.hpp"
#include "ECS/Component/Types/UI/UISelectableData.hpp"

namespace ECS
{
	UIColorPickerSystem::UIColorPickerSystem(PopupUIManager& popupManager) : m_popupManager(&popupManager)
	{
	}

	void UIColorPickerSystem::Init()
	{
		GlobalComponentInfo::AddComponentInfo(typeid(UIColorPickerData),
			ComponentInfo(CreateComponentTypes<UIRendererData, UISelectableData>(), CreateRequiredComponentFunction(UIRendererData(), UISelectableData()),
				[this](EntityData& entity)-> void
				{
					UIColorPickerData& colorPicker = *(entity.TryGetComponentMutable<UIColorPickerData>());
					colorPicker.m_renderer = entity.TryGetComponentMutable<UIRendererData>();
					colorPicker.m_selectable = entity.TryGetComponentMutable<UISelectableData>();
					colorPicker.m_popupManager = m_popupManager;

					colorPicker.Init();
				}));
	}
}
