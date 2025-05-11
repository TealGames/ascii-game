#pragma once
#include "raylib.h"
#include <functional>
#include "SelectableGUI.hpp"
#include "IRenderable.hpp"
#include "GUIStyle.hpp"
#include "GUIRect.hpp"

using ToggleAction = std::function<void(bool isChecked)>;
class ToggleGUI : public SelectableGUI
{
private:
	/// <summary>
	/// Represets the state of the toggle with TRUE-> on state, FALSE-> off state
	/// that changes between the two after a click
	/// </summary>
	bool m_isToggled;
	GUIStyle m_settings;

	ToggleAction m_valueSetAction;
public:

private:
public:
	ToggleGUI(const bool& startValue, const GUIStyle& settings, 
		const ToggleAction& valueSetAction=nullptr);
	~ToggleGUI();

	void SetSettings(const GUIStyle& settings);

	void SetValue(const bool value);
	void ToggleValue();
	bool IsToggled() const;

	void SetValueSetAction(const ToggleAction& action);

	RenderInfo Render(const RenderInfo& renderInfo) override;
};

