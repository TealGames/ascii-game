#include "pch.hpp"
#include "Editor/Entity/ColorPopupUI.hpp"
#include "Utils/Data/Vec2Type.hpp"
#include "Utils/HelperFunctions.hpp"
#include <limits>
#include "Editor/EditorStyles.hpp"
#include "ECS/Component/Types/UI/UITransformData.hpp"
#include "ECS/Component/Types/World/EntityComponent.hpp"
#include "ECS/Component/Types/UI/UISliderComponent.hpp"
#include "ECS/Component/Types/UI/UITextComponent.hpp"
#include "ECS/Component/Types/UI/UIInputField.hpp"
#include "ECS/Component/Types/UI/UILayout.hpp"

static constexpr float PICKER_SPACE_AREA_FACTOR = 0.7;
static constexpr float CHANNEL_SLIDER_WIDTH = 0.8;
static constexpr float CHANNEL_TEXT_WIDTH= 0.2;
static constexpr float COLOR_DISPLAY_RADIUS = 0.15;

static constexpr float SLIDER_LAYOUT_SIZE = 0.5;

constexpr static float INPUT_FIELD_TEXT_FONT_FACTOR = 0.6;

ColorChannelUI::ColorChannelUI() : m_Container(nullptr), m_Slider(nullptr), m_Text(nullptr) {}
void ColorChannelUI::CreateChannel(UITransformData& parent, const Input::InputManager& input)
{
	//m_Text())
	EntityData* containerEntity = nullptr;
	std::tie(containerEntity, m_Container) = parent.GetEntityMutable().CreateChildUI("UIColorChannel");
	m_Container->SetSize({ 1, 0.2 });

	m_Slider = &(containerEntity->AddComponent(UISliderComponent(input, Vec2{ 0, 1 }, EditorStyles::GetSliderStyle())));
	m_Text = &(containerEntity->AddComponent(UITextComponent("", EditorStyles::GetTextStyleFactorSize(TextAlignment::Center))));
	
	m_Text->GetEntityMutable().TryGetComponentMutable<UITransformData>()->SetBounds(NormalizedPosition::TOP_LEFT, { CHANNEL_TEXT_WIDTH, 0 });
	m_Slider->GetEntityMutable().TryGetComponentMutable<UITransformData>()->SetBounds({ CHANNEL_TEXT_WIDTH, 1 }, { CHANNEL_TEXT_WIDTH+ CHANNEL_SLIDER_WIDTH, 0});

	m_Slider->m_OnValueSet.AddListener([this](float value)-> void
		{
			m_Text->SetText(std::to_string(value));
		});
}
void ColorChannelUI::SetValue(const float value)
{
	m_Slider->SetValue(std::max(value, 0.0f));
}
float ColorChannelUI::GetValue() const
{
	return m_Slider->GetValue();
}

ColorPopupUI::ColorPopupUI(const Input::InputManager& input)
	: PopupUI(), m_inputManager(&input), m_sliderLayout(),
	m_hexField(nullptr), m_rgbChannels(Utils::ConstructArray<ColorChannelUI, 3>()) {}
	/*m_rgbSliders{ Utils::ConstructArray<SliderGUI, 3>(input, Vec2{ 0, 1 }, GUIStyle()) },
	m_rgbText{ Utils::ConstructArray<TextGUI, 3>("", TextGUIStyle(EntityEditorGUI::EDITOR_SECONDARY_COLOR,
		FontProperties(0, EntityEditorGUI::EDITOR_CHAR_SPACING.m_X, GetGlobalFont()), TextAlignment::CenterLeft, GUIPadding{}, 0.8)) }*/

void ColorPopupUI::AddPopupElements()
{
	m_hexField = &(m_Container->GetEntityMutable().AddComponent(UIInputField(*m_inputManager, InputFieldType::Any, InputFieldFlag::None, 
		EditorStyles::GetInputFieldStyle(TextAlignment::Center, INPUT_FIELD_TEXT_FONT_FACTOR))));

	m_Container->SetSize({ 0.4, 0.3 });
	m_sliderLayout = &(m_Container->GetEntityMutable().AddComponent(UILayout(LayoutType::Vertical, SizingType::ShrinkOnly, { 0, 0.1 })));
	m_sliderLayout->GetEntityMutable().TryGetComponentMutable<UITransformData>()->SetBounds({ 0, SLIDER_LAYOUT_SIZE }, { 1, 0 });

	m_hexField->GetEntityMutable().TryGetComponentMutable<UITransformData>()->SetBounds({ COLOR_DISPLAY_RADIUS*3, 1}, {1, SLIDER_LAYOUT_SIZE*1.5 });
	m_hexField->SetSubmitAction([this](const std::string& input) -> void
		{
			//Assert(false, std::format("SUBMIT TRIGGERED"));
			const std::string hexInput = Utils::TryExtractHexadecimal(input);
			if (hexInput.size()!= 6 && hexInput.size()!=8) return;

			SetColor(ConstructColorFromHex(std::stoi(hexInput, nullptr, 16)));
		});
	//m_rSlider.TryCenter(true, false);
	

	//Assert(false, std::format("Popup color:{}", ToStringRecursive("")));
}

void ColorPopupUI::SetColor(const Color color)
{
	m_rgbChannels[0].SetValue(color.m_R);
	m_rgbChannels[1].SetValue(color.m_G);
	m_rgbChannels[2].SetValue(color.m_B);
}
Color ColorPopupUI::GetColor() const
{
	return Color(m_rgbChannels[0].GetValue(), m_rgbChannels[1].GetValue(),
		m_rgbChannels[2].GetValue(), MAX_FLOAT_COLOR_CHANNEL);
}