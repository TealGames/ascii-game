#pragma once
#include "PopupGUI.hpp"
#include "InputFieldGUI.hpp"
#include "SliderGUI.hpp"
#include "LayoutGUI.hpp"
#include <array>
#include "Color.hpp"
#include "ContainerGUI.hpp"

class GUISelectorManager;
namespace Input { class InputManager; }

struct ColorChannelGUI
{
	ContainerGUI m_Container;
	SliderGUI m_Slider;
	TextGUI m_Text;

	ColorChannelGUI(const Input::InputManager& input);

	void SetValue(const std::uint8_t value);
	std::uint8_t GetValue() const;
};

class ColorPopupGUI : public PopupGUI
{
private:
	LayoutGUI m_sliderLayout;
	InputFieldGUI m_rField;

	std::array<ColorChannelGUI, 3> m_rgbChannels;
public:

private:
public:
	ColorPopupGUI(const Input::InputManager& input);

	void Update(const float deltaTime) override;
	void InsideRender(const RenderInfo& parentInfo) override;

	void SetColor(const Utils::Color color);
	Utils::Color GetColor() const;
};

