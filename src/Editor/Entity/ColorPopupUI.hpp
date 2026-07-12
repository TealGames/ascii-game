#pragma once
#include "Core/UI/PopupUI.hpp"
#include <array>
#include "Core/Primitives/Color.hpp"

namespace Engine::Input { class InputManager; }
namespace Engine::UI
{
	class UIInteractionManager;
	class UITransformComponent;
	class UISliderComponent;
	class UITextComponent;
	class UILayoutComponent;
	class UIInputFieldComponent;
}
namespace Engine::Editor::UI
{
	namespace UI = Engine::UI;
	struct ColorChannelUI
	{
		UI::UITransformComponent* m_Container;
		UI::UISliderComponent* m_Slider;
		UI::UITextComponent* m_Text;

		ColorChannelUI();
		void CreateChannel(UI::UITransformComponent& parent, const Input::InputManager& input);

		void SetValue(const float value);
		float GetValue() const;
	};

	class ColorPopupUI : public UI::PopupUI
	{
	private:
		const Input::InputManager* m_inputManager;
		UI::UILayoutComponent* m_sliderLayout;
		UI::UIInputFieldComponent* m_hexField;

		std::array<ColorChannelUI, 3> m_rgbChannels;
	public:

	private:
	public:
		ColorPopupUI(const Input::InputManager& input);
		void AddPopupElements() override;

		void SetColor(const ColHDR4 color);
		ColHDR4 GetColor() const;
	};
}


