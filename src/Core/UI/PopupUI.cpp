#include "pch.hpp"
#include "Core/UI/PopupUI.hpp"
#include "Editor/EditorStyles.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "ECS/Component/Types/UI/UITransformComponent.hpp"
#include "ECS/Component/Types/UI/UIPanelComponent.hpp"
#include "ECS/Component/Types/UI/UIRendererComponent.hpp"

namespace Engine::UI
{
	static constexpr float POPUP_PADDING = 0.05;

	PopupUI::PopupUI() : m_Container(nullptr) {}

	void PopupUI::CreatePopup(UITransformComponent& parent)
	{
		m_Container = &parent;
		m_Container->GetEntityMutable().AddComponent<UIPanelComponent>(UIPanelComponent(Editor::Styles::EDITOR_BACKGROUND_COLOR));
		m_Container->SetEventBlocker(true);

		AddPopupElements();
	}
}