#pragma once
#include "ISelectable.hpp"
#include "Event.hpp"
#include "RenderInfo.hpp"
#include "GUIElement.hpp"
#include "HelperMacros.hpp"
#include "HelperFunctions.hpp"

class SelectableGUI;

using SelectableInteractEvent = Event<void, SelectableGUI*>;
/// <summary>
/// Where float is the total drag time and the vec2 is the move delta
/// </summary>
using SelectableDragEvent = Event<void, SelectableGUI*, float, Vec2>;

enum class InteractionEventFlags : std::uint8_t
{
	None						= 0,
	InvokeSelectionEvents		= 1 << 0,
	InvokeClickEvent			= 1 << 1,
	InvokeHoverEvents			= 1 << 2,
	InvokeDragDeltaEvent		= 1 << 3,
};
FLAG_ENUM_OPERATORS(InteractionEventFlags)

enum class InteractionRenderFlags : std::uint8_t
{
	None						= 0,
	DrawDisabledOverlay			= 1 << 1,
	DrawHoverOverlay			= 1 << 2,
};
FLAG_ENUM_OPERATORS(InteractionRenderFlags)

class SelectableGUI : public ISelectable, public GUIElement
{
private:
	//GUIRect m_lastFrameRect;
	//GUISelectorManager* m_selectorManager;

	bool m_isSelected;
	bool m_isHovered;
	float m_dragTime;

public:
	static constexpr InteractionEventFlags DEFAULT_EVENT_FLAGS = Utils::SetAllFlags<InteractionEventFlags>();
	static constexpr InteractionRenderFlags DEFAULT_RENDER_FLAGS= Utils::SetAllFlags<InteractionRenderFlags>();

	InteractionEventFlags m_eventFlags;
	InteractionRenderFlags m_renderFlags;

	//TODO: maybe instead of having these events, we should instead have functions that can be overriden within selectable
	//that get triggered when those events occur
	SelectableInteractEvent m_OnSelect;
	SelectableInteractEvent m_OnDeselect;
	SelectableInteractEvent m_OnClick;
	SelectableInteractEvent m_OnHoverStart;
	SelectableInteractEvent m_OnHoverEnd;
	/// <summary>
	/// Invokes ONLY when this object is dragged with a delta greater than 0
	/// Note: delta is in carteisan coords but in
	/// </summary>
	SelectableDragEvent m_OnDragDelta;

protected:
	//GUISelectorManager& GetSelectorManager();
	/*void SetLastFramneRect(const GUIRect& newRect);
	GUIRect& GetLastFrameRectMutable();*/

	void DrawDisabledOverlay(const RenderInfo& renderInfo) const;
	void DrawHoverOverlay(const RenderInfo& renderInfo) const;

public:
	SelectableGUI(const InteractionEventFlags eventFlags= DEFAULT_EVENT_FLAGS, 
				  const InteractionRenderFlags renderFlags= DEFAULT_RENDER_FLAGS);
	~SelectableGUI();

	void Select();
	void Deselect();
	void Click();
	void HoverStart();
	void HoverEnd();

	bool IsHoveredOver() const;
	bool IsSelected() const;
	bool IsDraggedForTime(const float time) const;

	void UpdateDrag(const Vec2 mouseDelta, const float time);
	void ClearDragTime();

	void AddEventFlags(const InteractionEventFlags flags);
	void RemoveEventFlags(const InteractionEventFlags flags);
	void AddRenderFlags(const InteractionRenderFlags flags);
	void RemoveRenderFlags(const InteractionRenderFlags flags);

	//const GUIRect GetLastFrameRect() const;

	virtual void Update(const float deltaTime) override;
	RenderInfo Render(const RenderInfo& renderInfo) final override;
	virtual RenderInfo ElementRender(const RenderInfo& renderInfo) = 0;
};

