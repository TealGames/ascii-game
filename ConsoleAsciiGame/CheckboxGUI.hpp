#pragma once
#include "raylib.h"
#include <functional>
#include "SelectableGUI.hpp"
#include "IRenderable.hpp"
#include "GUISettings.hpp"
#include "GUIRect.hpp"

using CheckboxAction = std::function<void(bool isChecked)>;
class CheckboxGUI : public SelectableGUI, IRenderable
{
private:
	bool m_value;
	GUIRect m_lastFrameCheckboxRect;
	GUISettings m_settings;

	CheckboxAction m_valueSetAction;
public:

private:
public:
	CheckboxGUI();
	CheckboxGUI(GUISelectorManager& manager, const bool& startValue, const GUISettings& settings, 
		const CheckboxAction& valueSetAction=nullptr);

	void SetValue(const bool& value);
	void ToggleValue();
	const bool& IsChecked() const;

	void SetValueSetAction(const CheckboxAction& action);

	void Update();
	ScreenPosition Render(const RenderInfo& renderInfo) override;
};

