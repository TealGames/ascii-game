#include "pch.hpp"
#include "SelectableGUI.hpp"
#include "GUISelectorManager.hpp"
#include "Debug.hpp"

SelectableGUI::SelectableGUI()
	: SelectableGUI(nullptr) {}

SelectableGUI::SelectableGUI(GUISelectorManager* selectorManager) : 
	m_selectorManager(selectorManager), m_lastFrameRect(), 
	m_isSelected(false), m_addedToManager(false),
	m_OnSelect(), m_OnDeselect(), m_dragTime(0)
{
	
}

bool SelectableGUI::IsInit() const { return m_addedToManager; }
void SelectableGUI::Init()
{
	if (IsInit()) return;

	if (!Assert(this, m_selectorManager != nullptr,
		std::format("Tried to init selectable GUI but selector manager is NULL")))
		return;

	GetSelectorManager().AddSelectable(this);
	m_addedToManager = true;
}

GUIRect& SelectableGUI::GetLastFrameRectMutable() { return m_lastFrameRect; }
const GUIRect SelectableGUI::GetLastFrameRect() const { return m_lastFrameRect; }

void SelectableGUI::SetLastFramneRect(const GUIRect& newRect) { m_lastFrameRect = newRect; }

bool SelectableGUI::IsSelected() const { return m_isSelected; }
bool SelectableGUI::IsDraggedForTime(const float time) const { return time <= m_dragTime; }
void SelectableGUI::SetDragTime(const float time) { m_dragTime = time; }
void SelectableGUI::ClearDragTime() { m_dragTime = 0; }

GUISelectorManager& SelectableGUI::GetSelectorManager()
{
	if (!Assert(this, m_selectorManager != nullptr, std::format("Tried to get selector manager" 
		" from selectable gui but the manager is NULL")))
	{
		throw std::invalid_argument("Invalid selector manager state");
	}

	return *m_selectorManager;
}

void SelectableGUI::Select()
{
	m_isSelected = true;
	m_OnSelect.Invoke(this);
}


void SelectableGUI::Deselect()
{
	m_isSelected = false;
	m_OnDeselect.Invoke(this);
}

void SelectableGUI::Click()
{
	//Assert(false, "SELECTAVBLE CLICKL");
	m_OnClick.Invoke(this);
}

void SelectableGUI::DrawDisabledOverlay(const RenderInfo& renderInfo) const
{
	Color disabledOverlay = BLACK;
	disabledOverlay.a = 155;
	DrawRectangle(renderInfo.m_TopLeftPos.m_X, renderInfo.m_TopLeftPos.m_Y,
		renderInfo.m_RenderSize.m_X, renderInfo.m_RenderSize.m_Y, disabledOverlay);
}