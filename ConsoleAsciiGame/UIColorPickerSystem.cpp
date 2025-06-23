#include "pch.hpp"
#include "UIColorPickerSystem.hpp"
#include "GlobalComponentInfo.hpp"
#include "EntityData.hpp"
#include "UIRendererComponent.hpp"

namespace ECS
{
	UIColorPickerSystem::UIColorPickerSystem(PopupUIManager& popupManager) : m_popupManager(&popupManager)
	{
		GlobalComponentInfo::AddComponentInfo(typeid(UIColorPickerData), ComponentInfo(RequiredComponentCheck<UIRendererData>(),
			[this](EntityData& entity)-> void 
			{
				UIColorPickerData& colorPicker = *(entity.TryGetComponentMutable<UIColorPickerData>());
				colorPicker.m_renderer = entity.TryGetComponentMutable<UIRendererData>();
				colorPicker.m_popupManager = m_popupManager;
			}));
	}
}
