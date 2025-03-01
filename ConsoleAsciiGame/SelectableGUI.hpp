#pragma once
#include "ISelectable.hpp"
#include "GUIRect.hpp"
#include "Event.hpp"

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

private:
	GUISelectorManager& GetSelectorManager();
protected:
	void SetLastFramneRect(const GUIRect& newRect);
	GUIRect& GetLastRectMutable();

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

