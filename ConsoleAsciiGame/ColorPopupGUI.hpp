#pragma once
#include "PopupGUI.hpp"
#include "InputField.hpp"
#include "SliderGUI.hpp"

class GUISelectorManager;
namespace Input { class InputManager; }

class ColorPopupGUI : public PopupGUI
{
private:
	InputField m_rField;
	TextGUI m_rText;
	SliderGUI m_slider;

public:

private:
public:
	ColorPopupGUI(GUISelectorManager& guiSelector, const Input::InputManager& input);

	void Init() override;
	void Update(const float deltaTime) override;
	ScreenPosition Render(const RenderInfo& renderInfo) override;
};

