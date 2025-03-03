#pragma once
#include "SelectableGUI.hpp"
#include "InputManager.hpp"
#include <vector>
#include <optional>

class GUISelectorManager
{
private:
	const Input::InputManager& m_inputManager;
	std::vector<SelectableGUI*> m_selectables;
	SelectableGUI* m_currentSelected;

	std::optional<ScreenPosition> m_lastFrameClickedPosition;

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

	std::optional<ScreenPosition> GetLastFrameClickedPosition() const;
};

