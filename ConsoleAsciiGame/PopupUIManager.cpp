#include "pch.hpp"
#include "PopupUIManager.hpp"
#include "raylib.h"
#include "PositionConversions.hpp"
#include "EntityData.hpp"

PopupGUIInfo::PopupGUIInfo(PopupUI& gui)
	: m_UI(&gui) {}

bool PopupGUIInfo::IsEnabled() const
{
	return m_UI->m_Container->GetEntityMutable().IsEntityActive();
}
void PopupGUIInfo::Enable()
{
	m_UI->m_Container->GetEntityMutable().TryActivateEntity();
}
void PopupGUIInfo::Disable()
{
	m_UI->m_Container->GetEntityMutable().DeactivateEntity();
}

PopupUIManager::PopupUIManager(UIHierarchy& hierarchy)
	: m_popupContainer(nullptr), m_hierarchy(&hierarchy), m_popups(), m_OnPopupOpened(), m_OnPopupClosed()
{
	//m_rootSize = hierarchy.GetRootSize();
	m_popupContainer = std::get<0>(hierarchy.CreateAtRoot(TOP_LAYER, "PopupContainer"));
	LogWarning(std::format("After adding popup gui manager:{}", hierarchy.ToStringTree()));
}

PopupUIManager::~PopupUIManager()
{
	//Assert(false, "DESTROUY");
	if (m_popups.empty()) return;
	for (auto& popup : m_popups)
		delete popup.m_UI;

	m_popups = {};
}

PopupUI* PopupUIManager::OpenPopupAtSimple(PopupGUIInfo& popupInfo, const ScreenPosition& pos)
{
	//Assert(false, std::format("Converted pos:{} to norm:{}", pos.ToString(), screenPos.ToString()));
	const NormalizedPosition screenPos = Conversions::ScreenToNormalizedPosition(pos, m_hierarchy->GetRootSize());
	popupInfo.m_UI->m_Container->SetTopLeftPos(screenPos);
	popupInfo.Enable();

	m_OnPopupOpened.Invoke(Utils::FormatTypeName(typeid(*popupInfo.m_UI).name()), popupInfo.m_UI);
	return popupInfo.m_UI;
}
PopupUI* PopupUIManager::OpenPopupAtSimple(PopupGUIInfo& popupInfo, const UIRect& rect, const PopupPositionFlags flags)
{
	ScreenPosition topLeftPos = rect.m_TopLeftPos;
	if (Utils::HasFlagAny(flags, PopupPositionFlags::BelowRect))
		topLeftPos = topLeftPos+ ScreenPosition(0, rect.GetSize().m_Y);

	//Then we get the x so it is centered to the rect's center
	if (Utils::HasFlagAny(flags, PopupPositionFlags::CenteredXToRect))
	{
		std::optional<UIRect> maybeRect = m_hierarchy->TryCalculateRenderRect(*(popupInfo.m_UI->m_Container));
		if (maybeRect == std::nullopt)
		{
			Assert(false, std::format("Attemtped to open popup at (simple) but could not find its rect"));
			return nullptr;
		}

		const float popupWidth = maybeRect.value().GetSize().m_X;
		topLeftPos.m_X -= (popupWidth - rect.GetSize().m_X) / 2;
	}
	return OpenPopupAtSimple(popupInfo, topLeftPos);
}

bool PopupUIManager::TryClosePopup(PopupGUIInfo& popupInfo)
{
	if (!popupInfo.IsEnabled()) return false;
	popupInfo.Disable();
	m_OnPopupClosed.Invoke(Utils::FormatTypeName(typeid(*popupInfo.m_UI).name()), popupInfo.m_UI);
	return true;
}

//TODO: since we init after adding popup order creation matters so priority from popups can be removed
void PopupUIManager::AddPopup(PopupUI* popup)
{
	PopupGUIInfo& info= m_popups.emplace_back(PopupGUIInfo(*popup));
	info.m_UI->CreatePopup(m_popupContainer->CreateChild(std::format("{}Container", Utils::FormatTypeName(typeid(*popup).name()))));
	//emplaced.first->second.m_GUI->Init();
}
 
void PopupUIManager::CloseAllPopups()
{
	for (auto& popupInfo : m_popups)
	{
		if (popupInfo.IsEnabled())
			TryClosePopup(popupInfo);
	}
}