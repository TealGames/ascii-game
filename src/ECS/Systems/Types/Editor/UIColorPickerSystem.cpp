#include "pch.hpp"
#include "ECS/Systems/Types/Editor/UIColorPickerSystem.hpp"
#include "ECS/Component/GlobalComponentInfo.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "ECS/Component/Types/UI/UIRendererComponent.hpp"
#include "ECS/Component/Types/UI/UISelectableComponent.hpp"
#include "ECS/Component/Types/Editor/ColorPickerEditorComponent.hpp"

namespace Engine::Editor::UI
{
	UIColorPickerSystem::UIColorPickerSystem(UI::PopupUIManager& popupManager) : m_popupManager(&popupManager)
	{
	}

	void UIColorPickerSystem::Init()
	{
		ECS::GlobalComponentInfo::AddComponentInfo(typeid(ColorPickerEditorComponent),
			ECS::ComponentInfo(ECS::CreateComponentTypes<UI::UIRendererComponent, UI::UISelectableComponent>(), 
				ECS::CreateRequiredComponentFunction(UI::UIRendererComponent(), UI::UISelectableComponent()),
				[this](ECS::EntityData& entity)-> void
				{
					ColorPickerEditorComponent& colorPicker = *(entity.TryGetComponentMutable<ColorPickerEditorComponent>());
					colorPicker.m_renderer = entity.TryGetComponentMutable<UI::UIRendererComponent>();
					colorPicker.m_selectable = entity.TryGetComponentMutable<UI::UISelectableComponent>();
					colorPicker.m_popupManager = m_popupManager;

					colorPicker.Init();
				}));
	}
}
