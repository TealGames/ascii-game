#include "pch.hpp"
#include "SelectableGUI.hpp"
#include "GUISelectorManager.hpp"
#include "Debug.hpp"

SelectableGUI::SelectableGUI() : 
	//m_selectorManager(selectorManager), 
	//m_lastFrameRect(), 
	m_isSelected(false),
	m_OnSelect(), m_OnDeselect(), m_dragTime(0)
{
	
}
SelectableGUI::~SelectableGUI()
{
	//LogError(std::format("Selectable:{} destroyed", ToStringBase()));
}

//GUIRect& SelectableGUI::GetLastFrameRectMutable() { return m_lastFrameRect; }
//const GUIRect SelectableGUI::GetLastFrameRect() const { return m_lastFrameRect; }
//
//void SelectableGUI::SetLastFramneRect(const GUIRect& newRect) { m_lastFrameRect = newRect; }

bool SelectableGUI::IsSelected() const { return m_isSelected; }
bool SelectableGUI::IsDraggedForTime(const float time) const { return time <= m_dragTime; }
void SelectableGUI::UpdateDrag(const Vec2 mouseDelta, const float time) 
{ 
	m_dragTime = time;  
	//LogError(std::format("drag updated mouse mag:{} time:{}", std::to_string( mouseDelta.GetMagnitude()), std::to_string(time)));
	if (mouseDelta.GetMagnitude() != 0)
	{
		m_OnDragDelta.Invoke(this, m_dragTime, mouseDelta);
	}
}
void SelectableGUI::ClearDragTime() { m_dragTime = 0; }

void SelectableGUI::Update(const float deltaTime) {}

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
	LogError(std::format("Invoked click selectable ADDR:{}", Utils::ToStringPointerAddress(this)));
	m_OnClick.Invoke(this);
}

void SelectableGUI::DrawDisabledOverlay(const RenderInfo& renderInfo) const
{
	Color disabledOverlay = BLACK;
	disabledOverlay.a = 155;
	DrawRectangle(renderInfo.m_TopLeftPos.m_X, renderInfo.m_TopLeftPos.m_Y,
		renderInfo.m_RenderSize.m_X, renderInfo.m_RenderSize.m_Y, disabledOverlay);
}