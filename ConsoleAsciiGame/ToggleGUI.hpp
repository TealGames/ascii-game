#pragma once
#include "raylib.h"
#include <functional>
#include "SelectableGUI.hpp"
#include "IRenderable.hpp"
#include "GUISettings.hpp"
#include "GUIRect.hpp"

using ToggleAction = std::function<void(bool isChecked)>;
class ToggleGUI :  SelectableGUI, public IRenderable
{
private:
	/// <summary>
	/// Represets the state of the toggle with TRUE-> on state, FALSE-> off state
	/// that changes between the two after a click
	/// </summary>
	bool m_isToggled;
	GUISettings m_settings;

	ToggleAction m_valueSetAction;
public:

private:
public:
	ToggleGUI();
	ToggleGUI(GUISelectorManager& manager, const bool& startValue, const GUISettings& settings, 
		const ToggleAction& valueSetAction=nullptr);

	void SetSettings(const GUISettings& settings);

	void SetValue(const bool& value);
	void ToggleValue();
	const bool& IsToggled() const;

	void SetValueSetAction(const ToggleAction& action);

	void Update();
	ScreenPosition Render(const RenderInfo& renderInfo) override;
};

