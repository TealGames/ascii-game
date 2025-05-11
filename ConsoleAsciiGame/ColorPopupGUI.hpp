#pragma once
#include "PopupGUI.hpp"
#include "InputFieldGUI.hpp"
#include "SliderGUI.hpp"

class GUISelectorManager;
namespace Input { class InputManager; }

class ColorPopupGUI : public PopupGUI
{
private:
	InputFieldGUI m_rField;
	TextGUI m_rText;
	SliderGUI m_slider;

public:

private:
public:
	ColorPopupGUI(const Input::InputManager& input);

	void Update(const float deltaTime) override;
	RenderInfo Render(const RenderInfo& renderInfo) override;
};

