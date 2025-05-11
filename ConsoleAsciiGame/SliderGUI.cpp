#include "pch.hpp"
#include "SliderGUI.hpp"
#include "Debug.hpp"

static const Vec2 HANDLE_SIZE = {10, 20};
static constexpr int SLIDER_HEIGHT = 10;
static constexpr float MIN_DRAG_TIME_TO_MOVE = 0.1;

SliderGUI::SliderGUI(const Input::InputManager& inputManager, const Vec2 minMaxValues, const GUIStyle& settings)
	: SelectableGUI(), m_inputManager(&inputManager), m_settings(settings), 
	m_minMaxValues(minMaxValues), m_value(GetMinValue()), m_OnValueSet() 
{
	m_OnDragDelta.AddListener([this](SelectableGUI* gui, const float dragTime, const Vec2 mouseDelta)-> void 
		{
			Assert(false, std::format("REACEHD HERE"));

			if (dragTime < MIN_DRAG_TIME_TO_MOVE) return;

			float moveFraction = mouseDelta.m_X / GetLastFrameRectMutable().GetSize().m_X;
			SetValueDelta(moveFraction * (m_minMaxValues.m_Y- m_minMaxValues.m_X));
		});
}

void SliderGUI::SetSettings(const GUIStyle& settings)
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
void SliderGUI::SetValueDelta(const float delta)
{
	SetValue(m_value + delta);
}

float SliderGUI::GetMinValue() const { return m_minMaxValues.m_X; }
float SliderGUI::GetMaxValue() const { return m_minMaxValues.m_Y; }

int SliderGUI::GetMinValueInt() const { return m_minMaxValues.XAsInt(); }
int SliderGUI::GetMaxValueInt() const { return m_minMaxValues.YAsInt(); }

float SliderGUI::GetValue() const { return m_value; };
float SliderGUI::GetValueNormalized() const { return (m_value / m_minMaxValues.m_Y) + m_minMaxValues.m_X; }

RenderInfo SliderGUI::Render(const RenderInfo& renderInfo)
{
	const float sliderWidth = renderInfo.m_RenderSize.m_X;
	const float sliderHeight = std::min(renderInfo.m_RenderSize.m_Y, SLIDER_HEIGHT);
	DrawRectangle(renderInfo.m_TopLeftPos.m_X, renderInfo.m_TopLeftPos.m_Y,
		sliderWidth, sliderHeight, BLUE);

	const float handlePosX = GetValueNormalized() * sliderWidth + renderInfo.m_TopLeftPos.m_X - (HANDLE_SIZE.m_X/2);
	const float handlePosY = renderInfo.m_TopLeftPos.m_Y - (HANDLE_SIZE.m_Y - sliderHeight) / 2;
	DrawRectangle(handlePosX, handlePosY, HANDLE_SIZE.m_X, HANDLE_SIZE.m_Y, ORANGE);
	SetLastFramneRect(GUIRect(renderInfo.m_TopLeftPos, Vec2Int(sliderWidth, sliderHeight)));

	return { renderInfo.m_TopLeftPos, ScreenPosition(sliderWidth, sliderHeight) };
}