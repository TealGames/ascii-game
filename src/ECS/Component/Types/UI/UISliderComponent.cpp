#include "pch.hpp"
#include "ECS/Component/Types/UI/UISliderComponent.hpp"
#include "Core/Analyzation/Debug.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "ECS/Component/Types/UI/UIPanel.hpp"
#include "ECS/Component/Types/UI/UIRendererComponent.hpp"
#include "ECS/Component/Types/UI/UITransformData.hpp"
#include "ECS/Component/Types/UI/UISelectableData.hpp"

static const NormalizedPosition HANDLE_SIZE = {0.05, 1};
static const float SLIDER_HEIGHT = 0.5;
//static constexpr int SLIDER_HEIGHT = 10;
static constexpr float MIN_DRAG_TIME_TO_MOVE = 0.1;

UISliderComponent::UISliderComponent(const Input::InputManager& inputManager, const Vec2 minMaxValues, const UIStyle& settings)
	: m_inputManager(&inputManager), m_settings(settings), 
	m_minMaxValues(minMaxValues), m_value(GetMinValue()), m_OnValueSet(), 
	m_renderer(nullptr), m_backgroundPanel(nullptr), m_SliderHandle(nullptr), m_selectable(nullptr)
{
	
}

void UISliderComponent::Init()
{
	m_selectable->m_OnDragDelta.AddListener([this](UISelectableData* gui, const float dragTime, const Vec2 mouseDelta)-> void
		{
			//Assert(false, std::format("REACEHD HERE"));

			if (dragTime < MIN_DRAG_TIME_TO_MOVE) return;

			const Vec2 lastRenderSize = m_renderer->GetLastRenderRect().GetSize();
			float moveFraction = mouseDelta.m_X / lastRenderSize.m_X;
			SetValueDelta(moveFraction * (m_minMaxValues.m_Y - m_minMaxValues.m_X));

			m_SliderHandle->SetTopLeftPos(NormalizedPosition(GetValueNormalized() - (0.5 * HANDLE_SIZE.GetX()), 1));
		});
}

void UISliderComponent::SetSettings(const UIStyle& settings)
{
	m_settings = settings;
	if (m_backgroundPanel != nullptr) m_backgroundPanel->SetColor(settings.m_BackgroundColor);
}

void UISliderComponent::SetMinValue(const float min) { m_minMaxValues.m_X = min; }
void UISliderComponent::SetMaxValue(const float max) { m_minMaxValues.m_Y = max; }

void UISliderComponent::SetValue(const float value)
{
	m_value = std::clamp(value, m_minMaxValues.m_X, m_minMaxValues.m_Y);
	m_OnValueSet.Invoke(m_value);
}
void UISliderComponent::SetValueDelta(const float delta)
{
	SetValue(m_value + delta);
}

float UISliderComponent::GetMinValue() const { return m_minMaxValues.m_X; }
float UISliderComponent::GetMaxValue() const { return m_minMaxValues.m_Y; }

int UISliderComponent::GetMinValueInt() const { return m_minMaxValues.XAsInt(); }
int UISliderComponent::GetMaxValueInt() const { return m_minMaxValues.YAsInt(); }

float UISliderComponent::GetValue() const { return m_value; };
float UISliderComponent::GetValueNormalized() const { return (m_value / m_minMaxValues.m_Y) + m_minMaxValues.m_X; }

void UISliderComponent::Deserialize(const Json& json)
{
	//TODO: implement
	return;
}
Json UISliderComponent::Serialize() 
{
	//TODO: implement
	return {};
}

std::string UISliderComponent::ToString() const 
{
	return std::format("[UISlider ]");
}

//RenderInfo UISliderComponent::ElementRender(const RenderInfo& renderInfo)
//{
//	const float sliderWidth = renderInfo.m_RenderSize.m_X;
//	const float sliderHeight = renderInfo.m_RenderSize.m_Y;
//	DrawRectangle(renderInfo.m_TopLeftPos.m_X, renderInfo.m_TopLeftPos.m_Y,
//		sliderWidth, sliderHeight, m_settings.m_BackgroundColor);
//
//	const float handlePosX = GetValueNormalized() * sliderWidth + renderInfo.m_TopLeftPos.m_X - (HANDLE_SIZE.m_X/2);
//	const float handlePosY = renderInfo.m_TopLeftPos.m_Y - (HANDLE_SIZE.m_Y - sliderHeight) / 2;
//	DrawRectangle(handlePosX, handlePosY, HANDLE_SIZE.m_X, HANDLE_SIZE.m_Y, m_settings.m_SecondaryColor);
//	//SetLastFramneRect(GUIRect(renderInfo.m_TopLeftPos, Vec2Int(sliderWidth, sliderHeight)));
//
//	return renderInfo;
//}