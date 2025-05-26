#include "pch.hpp"
#include "SelectableGUI.hpp"
#include "GUISelectorManager.hpp"
#include "Debug.hpp"

SelectableGUI::SelectableGUI(const InteractionEventFlags eventFlags, const InteractionRenderFlags renderFlags) :
	m_eventFlags(eventFlags), m_renderFlags(renderFlags),
	//m_selectorManager(selectorManager), 
	//m_lastFrameRect(), 
	m_isSelected(false), m_isHovered(false),
	m_OnSelect(), m_OnDeselect(), m_dragTime(0)
{
	
}
SelectableGUI::~SelectableGUI()
{
	//LogError(std::format("Selectable:{} destroyed", ToStringBase()));
}

void SelectableGUI::AddEventFlags(const InteractionEventFlags flags)
{
	Utils::AddFlags(m_eventFlags, flags);
}
void SelectableGUI::RemoveEventFlags(const InteractionEventFlags flags)
{
	Utils::RemoveFlags(m_eventFlags, flags);
}
void SelectableGUI::AddRenderFlags(const InteractionRenderFlags flags)
{
	Utils::AddFlags(m_renderFlags, flags);
}
void SelectableGUI::RemoveRenderFlags(const InteractionRenderFlags flags)
{
	Utils::RemoveFlags(m_renderFlags, flags);
}

//GUIRect& SelectableGUI::GetLastFrameRectMutable() { return m_lastFrameRect; }
//const GUIRect SelectableGUI::GetLastFrameRect() const { return m_lastFrameRect; }
//
//void SelectableGUI::SetLastFramneRect(const GUIRect& newRect) { m_lastFrameRect = newRect; }

bool SelectableGUI::IsHoveredOver() const { return m_isHovered; }
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
	if (!Utils::HasFlagAll(m_eventFlags, InteractionEventFlags::InvokeSelectionEvents))
		return;

	m_isSelected = true;
	m_OnSelect.Invoke(this);
}

void SelectableGUI::Deselect()
{
	if (!Utils::HasFlagAll(m_eventFlags, InteractionEventFlags::InvokeSelectionEvents))
		return;

	m_isSelected = false;
	m_OnDeselect.Invoke(this);
}

void SelectableGUI::Click()
{
	if (!Utils::HasFlagAll(m_eventFlags, InteractionEventFlags::InvokeClickEvent))
		return;

	//Assert(false, "SELECTAVBLE CLICKL");
	//LogError(std::format("Invoked click selectable ADDR:{}", Utils::ToStringPointerAddress(this)));
	m_OnClick.Invoke(this);
}

void SelectableGUI::HoverStart()
{
	if (!Utils::HasFlagAll(m_eventFlags, InteractionEventFlags::InvokeHoverEvents))
		return;

	m_isHovered = true;
	m_OnHoverStart.Invoke(this);
}
void SelectableGUI::HoverEnd()
{
	if (!Utils::HasFlagAll(m_eventFlags, InteractionEventFlags::InvokeHoverEvents))
		return;

	m_isHovered = false;
	m_OnHoverEnd.Invoke(this);
}

RenderInfo SelectableGUI::Render(const RenderInfo& renderInfo)
{
	const RenderInfo elementRendered = ElementRender(renderInfo);
	if (m_renderFlags == InteractionRenderFlags::None) return renderInfo;

	if (Utils::HasFlagAll(m_renderFlags, InteractionRenderFlags::DrawHoverOverlay) && IsHoveredOver()) 
		DrawHoverOverlay(elementRendered);
	else if (Utils::HasFlagAll(m_renderFlags, InteractionRenderFlags::DrawDisabledOverlay) && !IsSelected()) 
		DrawDisabledOverlay(elementRendered);

	return renderInfo;
}

void SelectableGUI::DrawDisabledOverlay(const RenderInfo& renderInfo) const
{
	Color disabledOverlay = BLACK;
	disabledOverlay.a = 155;
	DrawRectangle(renderInfo.m_TopLeftPos.m_X, renderInfo.m_TopLeftPos.m_Y,
		renderInfo.m_RenderSize.m_X, renderInfo.m_RenderSize.m_Y, disabledOverlay);
}
void SelectableGUI::DrawHoverOverlay(const RenderInfo& renderInfo) const
{
	Color hoverOverlay = WHITE;
	hoverOverlay.a = 90;
	DrawRectangle(renderInfo.m_TopLeftPos.m_X, renderInfo.m_TopLeftPos.m_Y,
		renderInfo.m_RenderSize.m_X, renderInfo.m_RenderSize.m_Y, hoverOverlay);
}