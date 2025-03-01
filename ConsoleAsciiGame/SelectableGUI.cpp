#include "pch.hpp"
#include "SelectableGUI.hpp"
#include "GUISelectorManager.hpp"
#include "Debug.hpp"

SelectableGUI::SelectableGUI() 
	: m_selectorManager(nullptr), m_lastFrameRect(), 
	m_isSelected(false), m_addedToManager(false), 
	m_OnSelect(), m_OnDeselect()
{

}

SelectableGUI::SelectableGUI(GUISelectorManager* selectorManager) : 
	m_selectorManager(selectorManager), m_lastFrameRect(), 
	m_isSelected(false), m_addedToManager(false),
	m_OnSelect(), m_OnDeselect()
{
	
}

bool SelectableGUI::IsInit() const
{
	return m_addedToManager;
}
void SelectableGUI::Init()
{
	if (IsInit()) return;
	if (m_selectorManager != nullptr)
	{
		GetSelectorManager().AddSelectable(this);
		m_addedToManager = true;
	}
}

GUIRect& SelectableGUI::GetLastRectMutable()
{
	return m_lastFrameRect;
}

const GUIRect SelectableGUI::GetLastFrameRect() const
{
	return m_lastFrameRect;
}

void SelectableGUI::SetLastFramneRect(const GUIRect& newRect)
{
	m_lastFrameRect = newRect;
}

GUISelectorManager& SelectableGUI::GetSelectorManager()
{
	if (!Assert(this, m_selectorManager != nullptr, std::format("Tried to get selector manager" 
		" from selectable gui but the manager is NULL")))
	{
		throw std::invalid_argument("Invalid selector manager state");
	}

	return *m_selectorManager;
}

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

bool SelectableGUI::IsSelected() const
{
	return m_isSelected;
}
