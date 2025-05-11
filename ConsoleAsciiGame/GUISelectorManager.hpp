#pragma once
#include "SelectableGUI.hpp"
#include "InputManager.hpp"
#include <map>
#include <optional>
#include "GUIHierarchy.hpp"

class GUISelectorManager
{
private:
	const Input::InputManager& m_inputManager;
	GUIHierarchy& m_hierarchy;
	std::map<GUILayer, std::vector<SelectableGUI*>, std::greater<GUILayer>> m_selectableLayers;
	//TODO: for optimization reasons, this is slow
	//std::map<GUIEventPriority, SelectableGUI*, std::greater<GUIEventPriority>> m_selectables;

	SelectableGUI* m_currentSelected;
	SelectableGUI* m_currentDragged;
	/// <summary>
	/// True if the current selectable was selected THIS frame
	/// </summary>
	bool m_selectedThisFrame;
	Vec2 m_lastFrameMousePos;
public:

private:
	void CraeteSelectableArray();

	void SelectNewSelectable(SelectableGUI* selectable);
	void DeselectCurrentSelectable();
	void ClickSelectable(SelectableGUI* selectable);

	std::string ToStringSelectableTypes() const;
	

public:
	GUISelectorManager(const Input::InputManager& input, GUIHierarchy& hierarchy);

	void Init();
	void Update();

	void AddSelectable(const GUILayer layer, SelectableGUI* selectable);
	void AddSelectables(const GUILayer layer, const std::vector<SelectableGUI*>& selectables);

	bool HasSelecatbleSelected() const;
	const SelectableGUI* TryGetSelectableSelected() const;

	bool SelectedSelectableThisFrame() const;
};

