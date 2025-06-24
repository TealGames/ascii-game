#include "pch.hpp"
#include "PopupUI.hpp"
#include "raylib.h"
#include "EditorStyles.hpp"
#include "EntityData.hpp"
#include "UITransformData.hpp"
#include "UIPanel.hpp"
#include "UIRendererComponent.hpp"

static constexpr float POPUP_PADDING = 0.05;

PopupUI::PopupUI() : m_Container(nullptr) {}

void PopupUI::CreatePopup(EntityData& parent)
{
	EntityData* entity = nullptr;
	std::tie(entity, m_Container) = parent.CreateChildUI("popup_container");
	UIRendererData& renderer = entity->AddComponent(UIRendererData());
	entity->AddComponent<UIPanel>(UIPanel(EditorStyles::EDITOR_BACKGROUND_COLOR));

	renderer.SetEventBlocker(true);
	m_Container->GetPaddingMutable().SetAll(POPUP_PADDING);

	AddPopupElements();
}

//RenderInfo PopupGUI::Render(const RenderInfo& parentInfo)
//{
//	DrawRectangle(parentInfo.m_TopLeftPos.m_X, parentInfo.m_TopLeftPos.m_Y, parentInfo.m_RenderSize.m_X,
//		parentInfo.m_RenderSize.m_Y, EditorStyles::EDITOR_BACKGROUND_COLOR);
//	InsideRender(parentInfo);
//	return parentInfo;
//}