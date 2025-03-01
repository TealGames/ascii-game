#pragma once
#include "SelectableGUI.hpp"
#include "InputManager.hpp"
#include <vector>

class GUISelectorManager
{
private:
	const Input::InputManager& m_inputManager;
	std::vector<SelectableGUI*> m_selectables;
	SelectableGUI* m_currentSelected;
public:

private:
	void SelectNewSelectable(SelectableGUI* selectable);
	void DeselectCurrentSelectable();

public:
	GUISelectorManager(const Input::InputManager& input);

	void Update();

	void AddSelectable(SelectableGUI* selectable);
	bool HasSelecatbleSelected() const;
	const SelectableGUI* TryGetSelectableSelected() const;
};

