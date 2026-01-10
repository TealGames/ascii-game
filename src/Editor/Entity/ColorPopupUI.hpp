#pragma once
#include "Core/UI/PopupUI.hpp"
#include <array>
#include "Utils/Data/Color.hpp"

namespace Input { class InputManager; }
class UIInteractionManager;
class UITransformData;
class UISliderComponent;
class UITextComponent;
class UILayoutComponent;
class UIInputFieldComponent;

struct ColorChannelUI
{
	UITransformData* m_Container;
	UISliderComponent* m_Slider;
	UITextComponent* m_Text;

	ColorChannelUI();
	void CreateChannel(UITransformData& parent, const Input::InputManager& input);

	void SetValue(const float value);
	float GetValue() const;
};

class ColorPopupUI : public PopupUI
{
private:
	const Input::InputManager* m_inputManager;
	UILayoutComponent* m_sliderLayout;
	UIInputFieldComponent* m_hexField;

	std::array<ColorChannelUI, 3> m_rgbChannels;
public:

private:
public:
	ColorPopupUI(const Input::InputManager& input);
	void AddPopupElements() override;

	void SetColor(const HDRColor color);
	HDRColor GetColor() const;
};

