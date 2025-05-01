#include "pch.hpp"
#include "SliderGUI.hpp"

SliderGUI::SliderGUI(GUISelectorManager& selector, const Input::InputManager& inputManager, const Vec2 minMaxValues, const GUISettings& settings)
	: SelectableGUI(&selector), m_inputManager(&inputManager), m_settings(settings), 
	m_minMaxValues(minMaxValues), m_value(GetMinValue()), m_OnValueSet() {}

void SliderGUI::SetSettings(const GUISettings& settings)
{
	m_settings = settings;
}

void SliderGUI::SetMinValue(const float min) { m_minMaxValues.m_X = min; }
void SliderGUI::SetMaxValue(const float max) { m_minMaxValues.m_Y = max; }

void SliderGUI::SetValue(const float value)
{
	m_value = std::clamp(value, m_minMaxValues.m_X, m_minMaxValues.m_Y);
	m_OnValueSet.Invoke(m_value);
}

float SliderGUI::GetMinValue() const { return m_minMaxValues.m_X; }
float SliderGUI::GetMaxValue() const { return m_minMaxValues.m_Y; }

int SliderGUI::GetMinValueInt() const { return m_minMaxValues.XAsInt(); }
int SliderGUI::GetMaxValueInt() const { return m_minMaxValues.YAsInt(); }

void SliderGUI::Update()
{
	if (!IsInit()) Init();
}
ScreenPosition SliderGUI::Render(const RenderInfo& renderInfo)
{
	DrawRectangle(renderInfo.m_TopLeftPos.m_X, renderInfo.m_TopLeftPos.m_Y,
		renderInfo.m_RenderSize.m_X, renderInfo.m_RenderSize.m_Y, m_settings.m_BackgroundColor);
	return renderInfo.m_RenderSize;
}