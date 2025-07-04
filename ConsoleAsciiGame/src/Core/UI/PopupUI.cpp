#include "pch.hpp"
#include "Core/UI/PopupUI.hpp"
#include "raylib.h"
#include "Editor/EditorStyles.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "ECS/Component/Types/UI/UITransformData.hpp"
#include "ECS/Component/Types/UI/UIPanel.hpp"
#include "ECS/Component/Types/UI/UIRendererComponent.hpp"

static constexpr float POPUP_PADDING = 0.05;

PopupUI::PopupUI() : m_Container(nullptr) {}

void PopupUI::CreatePopup(EntityData& parent)
{
	EntityData* entity = nullptr;
	std::tie(entity, m_Container) = parent.CreateChildUI("popup_container");
	entity->AddComponent<UIPanel>(UIPanel(EditorStyles::EDITOR_BACKGROUND_COLOR));

	m_Container->SetEventBlocker(true);
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