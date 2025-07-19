#pragma once
#include "Core/UI/PopupUI.hpp"
#include <array>
#include "Utils/Data/Color.hpp"

namespace Input { class InputManager; }
class UIInteractionManager;
class UITransformData;
class UISliderComponent;
class UITextComponent;
class UILayout;
class UIInputField;

struct ColorChannelUI
{
	UITransformData* m_Container;
	UISliderComponent* m_Slider;
	UITextComponent* m_Text;

	ColorChannelUI();
	void CreateChannel(UITransformData& parent, const Input::InputManager& input);

	void SetValue(const std::uint8_t value);
	std::uint8_t GetValue() const;
};

class ColorPopupUI : public PopupUI
{
private:
	const Input::InputManager* m_inputManager;
	UILayout* m_sliderLayout;
	UIInputField* m_hexField;

	std::array<ColorChannelUI, 3> m_rgbChannels;
public:

private:
public:
	ColorPopupUI(const Input::InputManager& input);
	void AddPopupElements() override;

	void SetColor(const Utils::Color color);
	Utils::Color GetColor() const;
};

