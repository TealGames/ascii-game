#pragma once
#include "PopupGUI.hpp"
#include "InputField.hpp"

class GUISelectorManager;
namespace Input { class InputManager; }

class ColorPopupGUI : public PopupGUI
{
private:
	InputField m_rField;
	TextGUI m_rText;

public:

private:
public:
	ColorPopupGUI(GUISelectorManager& guiSelector, const Input::InputManager& input);

	ScreenPosition Render(const RenderInfo& renderInfo) override;
	void Update(const float deltaTime) override;
};

