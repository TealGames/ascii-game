#pragma once
#include "ISelectable.hpp"
#include "GUIRect.hpp"
#include "Event.hpp"
#include "RenderInfo.hpp"

class GUISelectorManager;
class SelectableGUI;

using SelectableEvent = Event<void, SelectableGUI*>;

class SelectableGUI : ISelectable
{
private:
	GUIRect m_lastFrameRect;
	GUISelectorManager* m_selectorManager;

	bool m_isSelected;
	bool m_addedToManager;

public:
	SelectableEvent m_OnSelect;
	SelectableEvent m_OnDeselect;

protected:
	GUISelectorManager& GetSelectorManager();
	void SetLastFramneRect(const GUIRect& newRect);
	GUIRect& GetLastFrameRectMutable();

	void DrawDisabledOverlay(const RenderInfo& renderInfo) const;

public:
	SelectableGUI();
	SelectableGUI(GUISelectorManager* selectorManager);

	bool IsInit() const;
	void Init();

	virtual void Select();
	virtual void Deselect();
	bool IsSelected() const;

	const GUIRect GetLastFrameRect() const;
};

