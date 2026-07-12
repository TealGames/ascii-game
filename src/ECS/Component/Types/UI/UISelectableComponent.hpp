#pragma once
#include "Utils/Data/Event.hpp"
#include "Utils/Data/CooldownEvent.hpp"
#include "Utils/HelperMacros.hpp"
#include "Utils/HelperFunctions.hpp"
#include "Core/Primitives/Vector.hpp"
#include "Core/UI/UIRect.hpp"
#include "ECS/Component/Component.hpp"

namespace Engine::UI
{
	class UIRendererComponent;
	class UISelectableComponent;
	class UISelectableSystem;

	using SelectableInteractEvent = Event<void, UISelectableComponent*>;
	using SelectableCooldownInteractEvent = CooldownEvent<void, UISelectableComponent*>;
	/// <summary>
	/// Where float is the total drag time and the vec2 is the move delta
	/// </summary>
	using SelectableDragEvent = Event<void, UISelectableComponent*, float, Vec2>;

	enum class TriggerInteractionEventFlags : std::uint8_t
	{
		None = 0,
		InvokeSelectionEvents = 1 << 0,
		InvokeClickEvent = 1 << 1,
		InvokeHoverEvents = 1 << 2,
		InvokeDragDeltaEvent = 1 << 3,
		All = 0xFF
	};
	FLAG_ENUM_OPERATORS(TriggerInteractionEventFlags)

		enum class InteractionRenderFlags : std::uint8_t
	{
		None = 0,
		DrawDisabledOverlay = 1 << 1,
		DrawHoverOverlay = 1 << 2,
		All = 0xFF
	};
	FLAG_ENUM_OPERATORS(InteractionRenderFlags)

		enum class InteractionEventFlags : std::uint8_t
	{
		None = 0,
		Selected = 1,
		Hovered = 1 << 1,
		Dragged = 1 << 2,
		All = 0xFF
	};
	FLAG_ENUM_OPERATORS(InteractionEventFlags)

	class UISelectableComponent : public ECS::Component
	{
	private:
		//GUIRect m_lastFrameRect;
		//UIInteractionManager* m_selectorManager;
		UIRendererComponent* m_renderer;

		float m_dragTime;
	public:
		friend class UISelectableSystem;
		static constexpr TriggerInteractionEventFlags DEFAULT_EVENT_FLAGS = TriggerInteractionEventFlags::All;
		static constexpr InteractionRenderFlags DEFAULT_RENDER_FLAGS = InteractionRenderFlags::All;

		TriggerInteractionEventFlags m_triggerEventFlags;
		InteractionEventFlags m_currentEventFlags;
		InteractionRenderFlags m_renderFlags;

		//TODO: maybe instead of having these events, we should instead have functions that can be overriden within selectable
		//that get triggered when those events occur
		SelectableInteractEvent m_OnSelect;
		SelectableInteractEvent m_OnDeselect;
		SelectableCooldownInteractEvent m_OnClick;
		SelectableInteractEvent m_OnHoverStart;
		SelectableInteractEvent m_OnHoverEnd;
		/// <summary>
		/// Invokes ONLY when this object is dragged with a delta greater than 0
		/// Note: delta is in carteisan coords but in
		/// </summary>
		SelectableDragEvent m_OnDragDelta;

	protected:
		//UIInteractionManager& GetSelectorManager();
		/*void SetLastFramneRect(const GUIRect& newRect);
		GUIRect& GetLastFrameRectMutable();*/

		void DrawDisabledOverlay(const UIRect& rect);
		void DrawHoverOverlay(const UIRect& rect);

	public:
		UISelectableComponent(const float clickCooldown = 0, const TriggerInteractionEventFlags eventFlags = DEFAULT_EVENT_FLAGS,
			const InteractionRenderFlags renderFlags = DEFAULT_RENDER_FLAGS);
		~UISelectableComponent();

		void Select();
		void Deselect();
		void Click();
		void HoverStart();
		void HoverEnd();

		bool IsHoveredOver() const;
		bool IsSelected() const;
		bool IsDragged() const;
		bool IsDraggedForTime(const float time) const;
		bool RectContainsPos(const ScreenPosition& pos) const;

		void UpdateDrag(const Vec2 mouseDelta, const float time);
		void ClearDragTime();

		void AddEventFlags(const TriggerInteractionEventFlags flags);
		void RemoveEventFlags(const TriggerInteractionEventFlags flags);
		void AddRenderFlags(const InteractionRenderFlags flags);
		void RemoveRenderFlags(const InteractionRenderFlags flags);

		/// <summary>
		/// If true, all interaction events are flagged to trigger.
		/// Otherwise, all are flagged to not trigger
		/// </summary>
		/// <param name="doPermit"></param>
		void SetAllUserInteraction(bool doPermit);

		//const GUIRect GetLastFrameRect() const;

		UIRect RenderOverlay(const UIRect& elementRendered);

		void InitFields() override;
		void Serialize(Serialization::Serializer& serializer) const override;
		void Deserialize(Serialization::Deserializer& deserializer) override;
		std::string ToString() const override;
	};
}


