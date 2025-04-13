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
	/// <summary>
	/// True if the current selectable was selected THIS frame
	/// </summary>
	bool m_selectedThisFrame;

	std::optional<ScreenPosition> m_lastFrameClickedPosition;
	
public:

private:
	void SelectNewSelectable(SelectableGUI* selectable);
	void DeselectCurrentSelectable();
	void ClickSelectable(SelectableGUI* selectable);

public:
	GUISelectorManager(const Input::InputManager& input);

	void Update();

	void AddSelectable(SelectableGUI* selectable);
	bool HasSelecatbleSelected() const;
	const SelectableGUI* TryGetSelectableSelected() const;

	bool SelectedSelectableThisFrame() const;

	std::optional<ScreenPosition> GetLastFrameClickedPosition() const;
};

