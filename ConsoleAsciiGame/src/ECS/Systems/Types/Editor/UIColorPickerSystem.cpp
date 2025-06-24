#include "pch.hpp"
#include "ECS/Systems/Types/Editor/UIColorPickerSystem.hpp"
#include "ECS/Component/GlobalComponentInfo.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "ECS/Component/Types/UI/UIRendererComponent.hpp"

namespace ECS
{
	UIColorPickerSystem::UIColorPickerSystem(PopupUIManager& popupManager) : m_popupManager(&popupManager)
	{
		GlobalComponentInfo::AddComponentInfo(typeid(UIColorPickerData), ComponentInfo(CreateRequiredComponentFunction<UIRendererData>(),
			[this](EntityData& entity)-> void 
			{
				UIColorPickerData& colorPicker = *(entity.TryGetComponentMutable<UIColorPickerData>());
				colorPicker.m_renderer = entity.TryGetComponentMutable<UIRendererData>();
				colorPicker.m_popupManager = m_popupManager;
			}));
	}
}
