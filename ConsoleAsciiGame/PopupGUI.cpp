#include "pch.hpp"
#include "PopupGUI.hpp"
#include "raylib.h"
#include "EditorStyles.hpp"

static constexpr float POPUP_PADDING = 0.05;

PopupGUI::PopupGUI()
{
	GetPaddingMutable().SetAll(POPUP_PADDING);
	SetEventBlocker(true);
}

RenderInfo PopupGUI::Render(const RenderInfo& parentInfo)
{
	DrawRectangle(parentInfo.m_TopLeftPos.m_X, parentInfo.m_TopLeftPos.m_Y, parentInfo.m_RenderSize.m_X,
		parentInfo.m_RenderSize.m_Y, EditorStyles::EDITOR_BACKGROUND_COLOR);
	InsideRender(parentInfo);
	return parentInfo;
}