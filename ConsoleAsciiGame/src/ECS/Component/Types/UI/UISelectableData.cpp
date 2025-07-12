#include "pch.hpp"
#include "ECS/Component/Types/UI/UISelectableData.hpp"
#include "Core/UI/UIInteractionManager.hpp"
#include "Core/Analyzation/Debug.hpp"
#include "ECS/Component/Types/UI/UIRendererComponent.hpp"
#include "Core/Rendering/GameRenderer.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"

UISelectableData::UISelectableData(const float clickCooldown, const TriggerInteractionEventFlags eventFlags, const InteractionRenderFlags renderFlags) :
	m_triggerEventFlags(eventFlags), m_renderFlags(renderFlags), m_renderer(nullptr),
	//m_selectorManager(selectorManager), 
	//m_lastFrameRect(), 
	m_dragTime(0), m_currentEventFlags(InteractionEventFlags::None),
	m_OnSelect(), m_OnDeselect(), m_OnClick(clickCooldown), m_OnHoverStart(), m_OnHoverEnd(), m_OnDragDelta()
{
	
}
UISelectableData::~UISelectableData()
{
	//LogError(std::format("Selectable:{} destroyed", ToStringBase()));
}

void UISelectableData::AddEventFlags(const TriggerInteractionEventFlags flags)
{
	Utils::AddFlags(m_triggerEventFlags, flags);
}
void UISelectableData::RemoveEventFlags(const TriggerInteractionEventFlags flags)
{
	Utils::RemoveFlags(m_triggerEventFlags, flags);
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

bool UISelectableData::IsHoveredOver() const { return Utils::HasFlagAll(m_currentEventFlags, InteractionEventFlags::Hovered); }
bool UISelectableData::IsDragged() const { return Utils::HasFlagAll(m_currentEventFlags, InteractionEventFlags::Dragged); }
bool UISelectableData::IsSelected() const { return Utils::HasFlagAll(m_currentEventFlags, InteractionEventFlags::Selected); }
bool UISelectableData::IsDraggedForTime(const float time) const 
{ 
	return Utils::HasFlagAll(m_currentEventFlags, InteractionEventFlags::Dragged) && time <= m_dragTime; 
}

void UISelectableData::UpdateDrag(const Vec2 mouseDelta, const float time) 
{ 
	if (m_dragTime<=0) Utils::AddFlags(m_currentEventFlags, InteractionEventFlags::Dragged);
	m_dragTime = time;  

	//LogError(std::format("drag updated mouse mag:{} time:{}", std::to_string( mouseDelta.GetMagnitude()), std::to_string(time)));
	if (mouseDelta.GetMagnitude() != 0)
	{
		m_OnDragDelta.Invoke(this, m_dragTime, mouseDelta);
	}
}
void UISelectableData::ClearDragTime() 
{ 
	m_dragTime = 0; 
	Utils::RemoveFlags(m_currentEventFlags, InteractionEventFlags::Dragged);
}

bool UISelectableData::RectContainsPos(const ScreenPosition& pos) const
{
	return m_renderer->GetLastRenderRect().ContainsPos(pos);
}

void UISelectableData::Select()
{
	if (!Utils::HasFlagAll(m_triggerEventFlags, TriggerInteractionEventFlags::InvokeSelectionEvents))
		return;

	Utils::AddFlags(m_currentEventFlags, InteractionEventFlags::Selected);
	m_OnSelect.Invoke(this);
}

void UISelectableData::Deselect()
{
	if (!Utils::HasFlagAll(m_triggerEventFlags, TriggerInteractionEventFlags::InvokeSelectionEvents))
		return;

	Utils::RemoveFlags(m_currentEventFlags, InteractionEventFlags::Selected);
	m_OnDeselect.Invoke(this);
}

void UISelectableData::Click()
{
	if (!Utils::HasFlagAll(m_triggerEventFlags, TriggerInteractionEventFlags::InvokeClickEvent))
		return;

	//Assert(false, "SELECTAVBLE CLICKL");
	//LogError(std::format("Invoked click selectable ADDR:{}", Utils::ToStringPointerAddress(this)));
	m_OnClick.Invoke(this);
}

void UISelectableData::HoverStart()
{
	if (!Utils::HasFlagAll(m_triggerEventFlags, TriggerInteractionEventFlags::InvokeHoverEvents))
		return;

	Utils::AddFlags(m_currentEventFlags, InteractionEventFlags::Hovered);
	m_OnHoverStart.Invoke(this);
}
void UISelectableData::HoverEnd()
{
	if (!Utils::HasFlagAll(m_triggerEventFlags, TriggerInteractionEventFlags::InvokeHoverEvents))
		return;

	Utils::RemoveFlags(m_currentEventFlags, InteractionEventFlags::Hovered);
	m_OnHoverEnd.Invoke(this);
}

UIRect UISelectableData::RenderOverlay(const UIRect& elementRendered)
{
	if (m_renderFlags == InteractionRenderFlags::None) return {};

	if (Utils::HasFlagAll(m_renderFlags, InteractionRenderFlags::DrawHoverOverlay) && IsHoveredOver()) 
		DrawHoverOverlay(elementRendered);
	else if (Utils::HasFlagAll(m_renderFlags, InteractionRenderFlags::DrawDisabledOverlay) && !IsSelected()) 
		DrawDisabledOverlay(elementRendered);

	return elementRendered;
}

void UISelectableData::DrawDisabledOverlay(const UIRect& renderInfo)
{
	Color disabledOverlay = BLACK;
	disabledOverlay.a = 155;

	UIRendererData* renderer = GetEntityMutable().TryGetComponentMutable<UIRendererData>();
	renderer->GetRendererMutable().AddRectangleCall(renderInfo.m_TopLeftPos, renderInfo.GetSize(), disabledOverlay);
}
void UISelectableData::DrawHoverOverlay(const UIRect& renderInfo)
{
	Color hoverOverlay = WHITE;
	hoverOverlay.a = 90;

	UIRendererData* renderer = GetEntityMutable().TryGetComponentMutable<UIRendererData>();
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