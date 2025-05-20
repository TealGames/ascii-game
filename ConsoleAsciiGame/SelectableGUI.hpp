#pragma once
#include "ISelectable.hpp"
#include "GUIRect.hpp"
#include "Event.hpp"
#include "RenderInfo.hpp"
#include "GUIElement.hpp"

class SelectableGUI;

using SelectableInteractEvent = Event<void, SelectableGUI*>;
/// <summary>
/// Where float is the total drag time and the vec2 is the move delta
/// </summary>
using SelectableDragEvent = Event<void, SelectableGUI*, float, Vec2>;

class SelectableGUI : public ISelectable, public GUIElement
{
private:
	GUIRect m_lastFrameRect;
	//GUISelectorManager* m_selectorManager;

	bool m_isSelected;
	float m_dragTime;

public:
	SelectableInteractEvent m_OnSelect;
	SelectableInteractEvent m_OnDeselect;
	SelectableInteractEvent m_OnClick;
	/// <summary>
	/// Invokes ONLY when this object is dragged with a delta greater than 0
	/// Note: delta is in carteisan coords but in
	/// </summary>
	SelectableDragEvent m_OnDragDelta;

protected:
	//GUISelectorManager& GetSelectorManager();
	void SetLastFramneRect(const GUIRect& newRect);
	GUIRect& GetLastFrameRectMutable();

	void DrawDisabledOverlay(const RenderInfo& renderInfo) const;

public:
	SelectableGUI();
	~SelectableGUI();

	void Select();
	void Deselect();
	void Click();

	bool IsSelected() const;
	bool IsDraggedForTime(const float time) const;

	void UpdateDrag(const Vec2 mouseDelta, const float time);
	void ClearDragTime();

	const GUIRect GetLastFrameRect() const;

	virtual void Update(const float deltaTime) override;
	virtual RenderInfo Render(const RenderInfo& renderInfo) = 0;
};

