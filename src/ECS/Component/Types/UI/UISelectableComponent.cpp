#include "pch.hpp"
#include "ECS/Component/Types/UI/UISelectableComponent.hpp"
#include "Core/UI/UIInteractionManager.hpp"
#include "Utils/Debug.hpp"
#include "ECS/Component/Types/UI/UIRendererComponent.hpp"
#include "Core/Rendering/Renderer3d.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "Core/Serialization/Serializer.hpp"

namespace Engine::UI
{
	static constexpr ColHDR4 HOVER_COLOR = ColHDR4(COLOR_WHITE, 90 / 255.0f);
	static constexpr ColHDR4 DISABLED_COLOR = ColHDR4(COLOR_BLACK, 155 / 255.0f);

	UISelectableComponent::UISelectableComponent(const float clickCooldown, 
		const TriggerInteractionEventFlags eventFlags, const InteractionRenderFlags renderFlags) :
		m_triggerEventFlags(eventFlags), m_renderFlags(renderFlags), m_renderer(nullptr),
		//m_selectorManager(selectorManager), 
		//m_lastFrameRect(), 
		m_dragTime(0), m_currentEventFlags(InteractionEventFlags::None),
		m_OnSelect(), m_OnDeselect(), m_OnClick(clickCooldown), m_OnHoverStart(), m_OnHoverEnd(), m_OnDragDelta()
	{

	}
	UISelectableComponent::~UISelectableComponent()
	{
		//LogError(std::format("Selectable:{} destroyed", ToStringBase()));
	}

	void UISelectableComponent::AddEventFlags(const TriggerInteractionEventFlags flags)
	{
		::Utils::AddFlags(m_triggerEventFlags, flags);
	}
	void UISelectableComponent::RemoveEventFlags(const TriggerInteractionEventFlags flags)
	{
		::Utils::RemoveFlags(m_triggerEventFlags, flags);
	}
	void UISelectableComponent::AddRenderFlags(const InteractionRenderFlags flags)
	{
		::Utils::AddFlags(m_renderFlags, flags);
	}
	void UISelectableComponent::RemoveRenderFlags(const InteractionRenderFlags flags)
	{
		::Utils::RemoveFlags(m_renderFlags, flags);
	}

	void UISelectableComponent::SetAllUserInteraction(bool doPermit)
	{
		if (doPermit) AddEventFlags(TriggerInteractionEventFlags::All);
		else RemoveEventFlags(TriggerInteractionEventFlags::All);
	}

	bool UISelectableComponent::IsHoveredOver() const { return ::Utils::HasFlagAll(m_currentEventFlags, InteractionEventFlags::Hovered); }
	bool UISelectableComponent::IsDragged() const { return ::Utils::HasFlagAll(m_currentEventFlags, InteractionEventFlags::Dragged); }
	bool UISelectableComponent::IsSelected() const { return ::Utils::HasFlagAll(m_currentEventFlags, InteractionEventFlags::Selected); }
	bool UISelectableComponent::IsDraggedForTime(const float time) const
	{
		return ::Utils::HasFlagAll(m_currentEventFlags, InteractionEventFlags::Dragged) && time <= m_dragTime;
	}

	void UISelectableComponent::UpdateDrag(const Vec2 mouseDelta, const float time)
	{
		if (m_dragTime <= 0) ::Utils::AddFlags(m_currentEventFlags, InteractionEventFlags::Dragged);
		m_dragTime = time;

		//LogError(std::format("drag updated mouse mag:{} time:{}", std::to_string( mouseDelta.GetMagnitude()), std::to_string(time)));
		if (mouseDelta.GetMagnitude() != 0)
		{
			m_OnDragDelta.Invoke(this, m_dragTime, mouseDelta);
		}
	}
	void UISelectableComponent::ClearDragTime()
	{
		m_dragTime = 0;
		::Utils::RemoveFlags(m_currentEventFlags, InteractionEventFlags::Dragged);
	}

	bool UISelectableComponent::RectContainsPos(const ScreenPosition& pos) const
	{
		return m_renderer->GetLastRenderRect().ContainsPos(pos);
	}

	void UISelectableComponent::Select()
	{
		if (!::Utils::HasFlagAll(m_triggerEventFlags, TriggerInteractionEventFlags::InvokeSelectionEvents))
			return;

		::Utils::AddFlags(m_currentEventFlags, InteractionEventFlags::Selected);
		m_OnSelect.Invoke(this);
	}

	void UISelectableComponent::Deselect()
	{
		if (!::Utils::HasFlagAll(m_triggerEventFlags, TriggerInteractionEventFlags::InvokeSelectionEvents))
			return;

		::Utils::RemoveFlags(m_currentEventFlags, InteractionEventFlags::Selected);
		//if (GetEntity().m_Name == "ConsoleInput") LogError("Deselected command field");
		m_OnDeselect.Invoke(this);
	}

	void UISelectableComponent::Click()
	{
		if (!::Utils::HasFlagAll(m_triggerEventFlags, TriggerInteractionEventFlags::InvokeClickEvent))
			return;

		//Assert(false, "SELECTAVBLE CLICKL");
		//LogError(std::format("Invoked click selectable ADDR:{}", ::Utils::ToStringPointerAddress(this)));
		m_OnClick.Invoke(this);
	}

	void UISelectableComponent::HoverStart()
	{
		if (!::Utils::HasFlagAll(m_triggerEventFlags, TriggerInteractionEventFlags::InvokeHoverEvents))
			return;

		::Utils::AddFlags(m_currentEventFlags, InteractionEventFlags::Hovered);
		m_OnHoverStart.Invoke(this);
	}
	void UISelectableComponent::HoverEnd()
	{
		if (!::Utils::HasFlagAll(m_triggerEventFlags, TriggerInteractionEventFlags::InvokeHoverEvents))
			return;

		::Utils::RemoveFlags(m_currentEventFlags, InteractionEventFlags::Hovered);
		m_OnHoverEnd.Invoke(this);
	}

	UIRect UISelectableComponent::RenderOverlay(const UIRect& elementRendered)
	{
		if (m_renderFlags == InteractionRenderFlags::None) return {};

		if (::Utils::HasFlagAll(m_renderFlags, InteractionRenderFlags::DrawHoverOverlay) && IsHoveredOver())
			DrawHoverOverlay(elementRendered);
		else if (::Utils::HasFlagAll(m_renderFlags, InteractionRenderFlags::DrawDisabledOverlay) && !IsSelected())
			DrawDisabledOverlay(elementRendered);

		return elementRendered;
	}

	void UISelectableComponent::DrawDisabledOverlay(const UIRect& renderInfo)
	{
		UIRendererComponent* renderer = GetEntityMutable().TryGetComponentMutable<UIRendererComponent>();
		//TODO: add ui render call
		//renderer->GetRendererMutable().AddRectangleCall(renderInfo.m_TopLeftPos, renderInfo.GetSize(), DISABLED_COLOR);
	}
	void UISelectableComponent::DrawHoverOverlay(const UIRect& renderInfo)
	{
		UIRendererComponent* renderer = GetEntityMutable().TryGetComponentMutable<UIRendererComponent>();
		//TODO: add ui render call
		//renderer->GetRendererMutable().AddRectangleCall(renderInfo.m_TopLeftPos, renderInfo.GetSize(), HOVER_COLOR);
	}

	void UISelectableComponent::InitFields()
	{
		m_Fields = {};
	}
	void UISelectableComponent::Serialize(Serialization::Serializer& serializer) const
	{
	}
	void UISelectableComponent::Deserialize(Serialization::Deserializer& deserializer)
	{
	}
	std::string UISelectableComponent::ToString() const
	{
		return std::format("[UISelectable]");
	}
}
