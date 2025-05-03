#pragma once
#include "SelectableGUI.hpp"
#include "InputManager.hpp"
#include <map>
#include <optional>

class GUISelectorManager
{
private:
	const Input::InputManager& m_inputManager;
	//TODO: for optimization reasons, this is slow
	std::map<GUIEventPriority, SelectableGUI*, std::greater<GUIEventPriority>> m_selectables;
	SelectableGUI* m_currentSelected;
	SelectableGUI* m_currentDragged;
	/// <summary>
	/// True if the current selectable was selected THIS frame
	/// </summary>
	bool m_selectedThisFrame;
	Vec2 m_lastFrameMousePos;
public:

private:
	void SelectNewSelectable(SelectableGUI* selectable);
	void DeselectCurrentSelectable();
	void ClickSelectable(SelectableGUI* selectable);

	std::string ToStringSelectableTypes() const;
	

public:
	GUISelectorManager(const Input::InputManager& input);

	void Update();

	void AddSelectable(SelectableGUI* selectable, const GUIEventPriority eventPriority);
	bool HasSelecatbleSelected() const;
	const SelectableGUI* TryGetSelectableSelected() const;

	bool SelectedSelectableThisFrame() const;
};

