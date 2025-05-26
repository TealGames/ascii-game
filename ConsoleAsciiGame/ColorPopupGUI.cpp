#include "pch.hpp"
#include "ColorPopupGUI.hpp"
#include "Vec2.hpp"
#include "raylib.h"
#include "HelperFunctions.hpp"
#include <limits>
#include "RaylibUtils.hpp"
#include "EditorStyles.hpp"

static constexpr float MAX_CHANNEL_VAL = std::numeric_limits<std::uint8_t>::max();

static constexpr float PICKER_SPACE_AREA_FACTOR = 0.7;
static constexpr float CHANNEL_SLIDER_WIDTH = 0.8;
static constexpr float CHANNEL_TEXT_WIDTH= 0.2;
static constexpr float COLOR_DISPLAY_RADIUS = 0.15;

static constexpr float SLIDER_LAYOUT_SIZE = 0.5;

constexpr static float INPUT_FIELD_TEXT_FONT_FACTOR = 0.6;

ColorChannelGUI::ColorChannelGUI(const Input::InputManager& input) 
	: m_Container(), m_Slider(input, Vec2{ 0, 1 }, EditorStyles::GetSliderStyle()), m_Text("", EditorStyles::GetTextStyleFactorSize(TextAlignment::Center))
{
	m_Container.SetSize({1, 0.2});
	m_Text.SetBounds(NormalizedPosition::TOP_LEFT, { CHANNEL_TEXT_WIDTH, 0 });
	m_Slider.SetBounds({ CHANNEL_TEXT_WIDTH, 1 }, { CHANNEL_TEXT_WIDTH+ CHANNEL_SLIDER_WIDTH, 0});

	m_Container.PushChild(&m_Slider);
	m_Container.PushChild(&m_Text);

	m_Slider.m_OnValueSet.AddListener([this](float value)-> void
		{
			m_Text.SetText(std::to_string(static_cast<std::uint8_t>(value * MAX_CHANNEL_VAL)));
		});
}
void ColorChannelGUI::SetValue(const std::uint8_t value)
{
	m_Slider.SetValue(value / MAX_CHANNEL_VAL);
}
std::uint8_t ColorChannelGUI::GetValue() const
{
	return static_cast<std::uint8_t>(m_Slider.GetValue() * MAX_CHANNEL_VAL);
}

ColorPopupGUI::ColorPopupGUI(const Input::InputManager& input) 
	: m_sliderLayout(LayoutType::Vertical, SizingType::ShrinkOnly, {0, 0.1}),
	m_hexField(input, InputFieldType::Any, InputFieldFlag::None, EditorStyles::GetInputFieldStyle(TextAlignment::Center, INPUT_FIELD_TEXT_FONT_FACTOR)),
	m_rgbChannels(Utils::ConstructArray<ColorChannelGUI, 3>(input))
	/*m_rgbSliders{ Utils::ConstructArray<SliderGUI, 3>(input, Vec2{ 0, 1 }, GUIStyle()) },
	m_rgbText{ Utils::ConstructArray<TextGUI, 3>("", TextGUIStyle(EntityEditorGUI::EDITOR_SECONDARY_COLOR,
		FontProperties(0, EntityEditorGUI::EDITOR_CHAR_SPACING.m_X, GetGlobalFont()), TextAlignment::CenterLeft, GUIPadding{}, 0.8)) }*/
{
	SetSize({ 0.4, 0.3 });
	m_sliderLayout.SetBounds({ 0, SLIDER_LAYOUT_SIZE }, { 1, 0 });

	for (size_t i=0; i< m_rgbChannels.size(); i++)
	{
		m_sliderLayout.AddLayoutElement(&m_rgbChannels[i].m_Container);
	}

	m_hexField.SetBounds({ COLOR_DISPLAY_RADIUS*3, 1}, {1, SLIDER_LAYOUT_SIZE*1.5 });
	m_hexField.SetSubmitAction([this](const std::string& input) -> void
		{
			//Assert(false, std::format("SUBMIT TRIGGERED"));
			const std::string hexInput = Utils::TryExtractHexadecimal(input);
			if (hexInput.size()!= 6 && hexInput.size()!=8) return;

			Color color = RaylibUtils::GetColorFromHex(std::stoi(hexInput, nullptr, 16));
			SetColor(RaylibUtils::FromRaylibColor(color));
		});
	//m_rSlider.TryCenter(true, false);
	
	
	PushChild(&m_sliderLayout);
	PushChild(&m_hexField);
	

	//Assert(false, std::format("Popup color:{}", ToStringRecursive("")));
}

void ColorPopupGUI::InsideRender(const RenderInfo& parentInfo)
{
	const float radius = COLOR_DISPLAY_RADIUS * parentInfo.m_RenderSize.m_X;

	const ScreenPosition topleftPadding = ScreenPosition(GetPadding().m_Left.GetValue() * parentInfo.m_RenderSize.m_X, GetPadding().m_Top.GetValue() * parentInfo.m_RenderSize.m_Y);
	const ScreenPosition centerPos = parentInfo.m_TopLeftPos + ScreenPosition(radius, radius) + topleftPadding;
	DrawCircle(centerPos.m_X, centerPos.m_Y, radius, RaylibUtils::ToRaylibColor(GetColor()));
}
//RenderInfo ColorPopupGUI::Render(const RenderInfo& renderInfo)
//{
//	const Vec2 pickerSpace = Vec2(renderInfo.m_RenderSize.m_X, PICKER_SPACE_AREA_FACTOR * renderInfo.m_RenderSize.m_Y);
//	const Vec2 displaySpace = Vec2(renderInfo.m_RenderSize.m_X, renderInfo.m_RenderSize.m_Y) - pickerSpace;
//
//	m_slider.Render(renderInfo);
//
//	return renderInfo;
//}
void ColorPopupGUI::Update(const float deltaTime)
{
	//m_slider.Update();
}

void ColorPopupGUI::SetColor(const Utils::Color color)
{
	m_rgbChannels[0].SetValue(color.m_R);
	m_rgbChannels[1].SetValue(color.m_G);
	m_rgbChannels[2].SetValue(color.m_B);
}
Utils::Color ColorPopupGUI::GetColor() const
{
	return Utils::Color(m_rgbChannels[0].GetValue(), m_rgbChannels[1].GetValue(),
		m_rgbChannels[2].GetValue(), MAX_CHANNEL_VAL);
}