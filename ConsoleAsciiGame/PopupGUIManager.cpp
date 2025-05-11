#include "pch.hpp"
#include "PopupGUIManager.hpp"
#include "raylib.h"
#include "EntityEditorGUI.hpp"

const ScreenPosition PopupGUIInfo::INVALID_RENDER_POS = {-1, -1};

PopupGUIInfo::PopupGUIInfo(PopupGUI& gui, const RenderInfo& info)
	: m_GUI(&gui), m_RenderInfo(info) {}

bool PopupGUIInfo::IsEnabled() const
{
	return m_RenderInfo.m_TopLeftPos != INVALID_RENDER_POS;
}

PopupGUIManager::PopupGUIManager() 
	: m_popups(), m_OnPopupOpened(), m_OnPopupClosed() {}

PopupGUIManager::~PopupGUIManager()
{
	//Assert(false, "DESTROUY");
	if (m_popups.empty()) return;
	for (auto& popup : m_popups)
		delete popup.second.m_GUI;

	m_popups = {};
}

PopupGUI* PopupGUIManager::OpenPopupAtSimple(PopupGUIInfo& popupInfo, const ScreenPosition& pos)
{
	popupInfo.m_RenderInfo.m_TopLeftPos = pos;
	m_OnPopupOpened.Invoke(popupInfo.m_GUI);
	return popupInfo.m_GUI;
}

bool PopupGUIManager::TryClosePopup(PopupGUIInfo& popupInfo)
{
	if (!popupInfo.IsEnabled()) return false;

	popupInfo.m_RenderInfo.m_TopLeftPos = PopupGUIInfo::INVALID_RENDER_POS;
	m_OnPopupClosed.Invoke(popupInfo.m_GUI);
	return true;
}

//TODO: since we init after adding popup order creation matters so priority from popups can be removed
void PopupGUIManager::AddPopup(PopupGUI* popup, const ScreenPosition& targetSize, const RenderPriority priority)
{
	m_popups.emplace(priority, PopupGUIInfo(*popup, RenderInfo(PopupGUIInfo::INVALID_RENDER_POS, targetSize)));
	//emplaced.first->second.m_GUI->Init();
}

void PopupGUIManager::UpdatePopups(const float deltaTime)
{
	//Note: since we store in decreasing priority, higher priority-> faster init/gui selector placement
	//and therefore get checked earlier by select/click/drag events
	for (auto& popup : m_popups)
	{
		if (!popup.second.IsEnabled()) continue;

		popup.second.m_GUI->Update(deltaTime);
	}
}
 
void PopupGUIManager::CloseAllPopups()
{
	for (auto& popupInfo : m_popups)
	{
		if (popupInfo.second.IsEnabled())
			TryClosePopup(popupInfo.second);
	}
}

void PopupGUIManager::RenderPopups()
{
	for (auto& popup : m_popups)
	{
		if (!popup.second.IsEnabled()) continue;

		DrawRectangle(popup.second.m_RenderInfo.m_TopLeftPos.m_X, popup.second.m_RenderInfo.m_TopLeftPos.m_Y, popup.second.m_RenderInfo.m_RenderSize.m_X,
			popup.second.m_RenderInfo.m_RenderSize.m_Y, EntityEditorGUI::EDITOR_BACKGROUND_COLOR);

		//TODO: we should consider any psoition or sizing restrictions that may apply
		popup.second.m_GUI->Render(popup.second.m_RenderInfo);
	}
}