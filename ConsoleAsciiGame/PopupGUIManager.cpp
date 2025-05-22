#include "pch.hpp"
#include "PopupGUIManager.hpp"
#include "raylib.h"
#include "PositionConversions.hpp"

const size_t PopupGUIInfo::INVALID_INDEX = -1;

PopupGUIInfo::PopupGUIInfo(PopupGUI& gui)
	: m_GUI(&gui), m_ChildIndex(INVALID_INDEX) {}

bool PopupGUIInfo::IsEnabled() const
{
	return m_ChildIndex != size_t(-1);
}

PopupGUIManager::PopupGUIManager(GUIHierarchy& hierarchy)
	: m_popupContainer(), m_hierarchy(&hierarchy), m_popups(), m_OnPopupOpened(), m_OnPopupClosed()
{
	//m_rootSize = hierarchy.GetRootSize();
	hierarchy.AddToRoot(TOP_LAYER, &m_popupContainer);
	LogWarning(std::format("After adding popup gui manager:{}", hierarchy.ToStringTree()));
}

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
	//Assert(false, std::format("Converted pos:{} to norm:{}", pos.ToString(), screenPos.ToString()));
	popupInfo.m_ChildIndex = m_popupContainer.PushChild(popupInfo.m_GUI);

	const NormalizedPosition screenPos = Conversions::ScreenToNormalizedPosition(pos, m_hierarchy->GetRootSize());
	popupInfo.m_GUI->SetTopLeftPos(screenPos);

	m_OnPopupOpened.Invoke(Utils::FormatTypeName(typeid(*popupInfo.m_GUI).name()), popupInfo.m_GUI);
	return popupInfo.m_GUI;
}
PopupGUI* PopupGUIManager::OpenPopupAtSimple(PopupGUIInfo& popupInfo, const GUIRect& rect, const PopupPositionFlags flags)
{
	ScreenPosition topLeftPos = rect.GetTopleftPos();
	if (Utils::HasFlagAny(flags, PopupPositionFlags::BelowRect))
		topLeftPos = topLeftPos+ ScreenPosition(0, rect.GetSize().m_Y);

	//Then we get the x so it is centered to the rect's center
	if (Utils::HasFlagAny(flags, PopupPositionFlags::CenteredXToRect))
	{
		const float popupWidth = m_hierarchy->TryCalculateElementRenderInfoFromRoot(*(popupInfo.m_GUI)).m_RenderSize.m_X;
		topLeftPos.m_X -= (popupWidth - rect.GetSize().m_X) / 2;
	}
	return OpenPopupAtSimple(popupInfo, topLeftPos);
}

bool PopupGUIManager::TryClosePopup(PopupGUIInfo& popupInfo)
{
	if (!popupInfo.IsEnabled()) return false;
	if (m_popupContainer.TryPopChildAt(popupInfo.m_ChildIndex) == nullptr)
	{
		Assert(false, std::format("Tried to close popup from gui manager but popup could not be removed "
			"from container. ChildIndex:{} container:{}", std::to_string(popupInfo.m_ChildIndex), m_popupContainer.ToStringRecursive("")));
		return false;
	}

	popupInfo.m_ChildIndex = PopupGUIInfo::INVALID_INDEX;
	m_OnPopupClosed.Invoke(Utils::FormatTypeName(typeid(*popupInfo.m_GUI).name()), popupInfo.m_GUI);
	return true;
}

//TODO: since we init after adding popup order creation matters so priority from popups can be removed
void PopupGUIManager::AddPopup(PopupGUI* popup, const ScreenPosition& targetSize, const RenderPriority priority)
{
	m_popups.emplace(priority, PopupGUIInfo(*popup));
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

//void PopupGUIManager::RenderPopups()
//{
//	for (auto& popup : m_popups)
//	{
//		if (!popup.second.IsEnabled()) continue;
//
//		DrawRectangle(popup.second.m_RenderInfo.m_TopLeftPos.m_X, popup.second.m_RenderInfo.m_TopLeftPos.m_Y, popup.second.m_RenderInfo.m_RenderSize.m_X,
//			popup.second.m_RenderInfo.m_RenderSize.m_Y, EntityEditorGUI::EDITOR_BACKGROUND_COLOR);
//
//		//TODO: we should consider any psoition or sizing restrictions that may apply
//		popup.second.m_GUI->Render(popup.second.m_RenderInfo);
//	}
//}