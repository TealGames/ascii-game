#pragma once
#include "PopupGUI.hpp"
#include "UIInputField.hpp"
#include "UISliderComponent.hpp"
#include "UILayout.hpp"
#include <array>
#include "Color.hpp"
#include "UIContainer.hpp"

class GUISelectorManager;
namespace Input { class InputManager; }

struct ColorChannelGUI
{
	UIContainer m_Container;
	UISliderComponent m_Slider;
	UITextComponent m_Text;

	ColorChannelGUI(const Input::InputManager& input);

	void SetValue(const std::uint8_t value);
	std::uint8_t GetValue() const;
};

class ColorPopupGUI : public PopupGUI
{
private:
	UILayout m_sliderLayout;
	UIInputField m_hexField;

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

