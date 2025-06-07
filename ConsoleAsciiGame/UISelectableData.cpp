#include "pch.hpp"
#include "UISelectableData.hpp"
#include "GUISelectorManager.hpp"
#include "Debug.hpp"
#include "UIRendererComponent.hpp"
#include "GameRenderer.hpp"
#include "EntityData.hpp"
#include "UIRendererComponent.hpp"

UISelectableData::UISelectableData(const InteractionEventFlags eventFlags, const InteractionRenderFlags renderFlags) :
	m_eventFlags(eventFlags), m_renderFlags(renderFlags),
	//m_selectorManager(selectorManager), 
	//m_lastFrameRect(), 
	m_isSelected(false), m_isHovered(false),
	m_OnSelect(), m_OnDeselect(), m_dragTime(0)
{
	
}
UISelectableData::~UISelectableData()
{
	//LogError(std::format("Selectable:{} destroyed", ToStringBase()));
}

void UISelectableData::AddEventFlags(const InteractionEventFlags flags)
{
	Utils::AddFlags(m_eventFlags, flags);
}
void UISelectableData::RemoveEventFlags(const InteractionEventFlags flags)
{
	Utils::RemoveFlags(m_eventFlags, flags);
}
void UISelectableData::AddRenderFlags(const InteractionRenderFlags flags)
{
	Utils::AddFlags(m_renderFlags, flags);
}
void UISelectableData::RemoveRenderFlags(const InteractionRenderFlags flags)
{
	Utils::RemoveFlags(m_renderFlags, flags);
}

//GUIRect& SelectableGUI::GetLastFrameRectMutable() { return m_lastFrameRect; }
//const GUIRect SelectableGUI::GetLastFrameRect() const { return m_lastFrameRect; }
//
//void SelectableGUI::SetLastFramneRect(const GUIRect& newRect) { m_lastFrameRect = newRect; }

bool UISelectableData::IsHoveredOver() const { return m_isHovered; }
bool UISelectableData::IsSelected() const { return m_isSelected; }
bool UISelectableData::IsDraggedForTime(const float time) const { return time <= m_dragTime; }
void UISelectableData::UpdateDrag(const Vec2 mouseDelta, const float time) 
{ 
	m_dragTime = time;  
	//LogError(std::format("drag updated mouse mag:{} time:{}", std::to_string( mouseDelta.GetMagnitude()), std::to_string(time)));
	if (mouseDelta.GetMagnitude() != 0)
	{
		m_OnDragDelta.Invoke(this, m_dragTime, mouseDelta);
	}
}
void UISelectableData::ClearDragTime() { m_dragTime = 0; }

bool UISelectableData::RectContainsPos(const ScreenPosition& pos) const
{
	return m_renderer->GetLastRenderRect().ContainsPos(pos);
}

void UISelectableData::Update(const float deltaTime) {}

void UISelectableData::Select()
{
	if (!Utils::HasFlagAll(m_eventFlags, InteractionEventFlags::InvokeSelectionEvents))
		return;

	m_isSelected = true;
	m_OnSelect.Invoke(this);
}

void UISelectableData::Deselect()
{
	if (!Utils::HasFlagAll(m_eventFlags, InteractionEventFlags::InvokeSelectionEvents))
		return;

	m_isSelected = false;
	m_OnDeselect.Invoke(this);
}

void UISelectableData::Click()
{
	if (!Utils::HasFlagAll(m_eventFlags, InteractionEventFlags::InvokeClickEvent))
		return;

	//Assert(false, "SELECTAVBLE CLICKL");
	//LogError(std::format("Invoked click selectable ADDR:{}", Utils::ToStringPointerAddress(this)));
	m_OnClick.Invoke(this);
}

void UISelectableData::HoverStart()
{
	if (!Utils::HasFlagAll(m_eventFlags, InteractionEventFlags::InvokeHoverEvents))
		return;

	m_isHovered = true;
	m_OnHoverStart.Invoke(this);
}
void UISelectableData::HoverEnd()
{
	if (!Utils::HasFlagAll(m_eventFlags, InteractionEventFlags::InvokeHoverEvents))
		return;

	m_isHovered = false;
	m_OnHoverEnd.Invoke(this);
}

GUIRect UISelectableData::RenderOverlay(const GUIRect& elementRendered)
{
	if (m_renderFlags == InteractionRenderFlags::None) return {};

	if (Utils::HasFlagAll(m_renderFlags, InteractionRenderFlags::DrawHoverOverlay) && IsHoveredOver()) 
		DrawHoverOverlay(elementRendered);
	else if (Utils::HasFlagAll(m_renderFlags, InteractionRenderFlags::DrawDisabledOverlay) && !IsSelected()) 
		DrawDisabledOverlay(elementRendered);

	return elementRendered;
}

void UISelectableData::DrawDisabledOverlay(const GUIRect& renderInfo)
{
	Color disabledOverlay = BLACK;
	disabledOverlay.a = 155;

	UIRendererData* renderer = GetEntitySafeMutable().TryGetComponentMutable<UIRendererData>();
	renderer->GetRendererMutable().AddRectangleCall(renderInfo.m_TopLeftPos, renderInfo.GetSize(), disabledOverlay);
}
void UISelectableData::DrawHoverOverlay(const GUIRect& renderInfo)
{
	Color hoverOverlay = WHITE;
	hoverOverlay.a = 90;

	UIRendererData* renderer = GetEntitySafeMutable().TryGetComponentMutable<UIRendererData>();
	renderer->GetRendererMutable().AddRectangleCall(renderInfo.m_TopLeftPos, renderInfo.GetSize(), hoverOverlay);
}

void UISelectableData::InitFields()
{
	m_Fields = {};
}

std::string UISelectableData::ToString() const
{
	return std::format("[UISelectable]");
}

void UISelectableData::Deserialize(const Json& json)
{
	//TODO: implement
	return;
}
Json UISelectableData::Serialize()
{
	//TODO: implement
	return {};
}