#include "pch.hpp"
#include "Utils/Debug.hpp"
#include "ECS/Component/Types/UI/UISliderComponent.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "ECS/Component/Types/UI/UIPanelComponent.hpp"
#include "ECS/Component/Types/UI/UIRendererComponent.hpp"
#include "ECS/Component/Types/UI/UITransformComponent.hpp"
#include "ECS/Component/Types/UI/UISelectableComponent.hpp"
#include "Core/Serialization/Serializer.hpp"

namespace Engine::UI
{
	static const NormalizedVec2 HANDLE_SIZE = { 0.05, 1 };
	static const float SLIDER_HEIGHT = 0.5;
	//static constexpr int SLIDER_HEIGHT = 10;
	static constexpr float MIN_DRAG_TIME_TO_MOVE = 0.1;

	UISliderComponent::UISliderComponent( const Vec2 minMaxValues, const UIStyle& settings)
		: m_inputManager(nullptr), m_settings(settings),
		m_minMaxValues(minMaxValues), m_value(GetMinValue()), m_OnValueSet(),
		m_renderer(nullptr), m_backgroundPanel(nullptr), m_SliderHandle(nullptr), m_selectable(nullptr)
	{

	}

	void UISliderComponent::Init(const Input::InputManager& inputManager)
	{
		m_inputManager = &inputManager;
		m_selectable->m_OnDragDelta.AddListener([this](UISelectableComponent* gui, const float dragTime, const Vec2 mouseDelta)-> void
			{
				//Assert(false, std::format("REACEHD HERE"));

				if (dragTime < MIN_DRAG_TIME_TO_MOVE) return;

				const Vec2 lastRenderSize = m_renderer->GetLastRenderRect().GetSize().AsVec2();
				float moveFraction = mouseDelta.m_X / lastRenderSize.m_X;
				SetValueDelta(moveFraction * (m_minMaxValues.m_Y - m_minMaxValues.m_X));

				m_SliderHandle->SetLocalTopLeftPos(NormalizedVec2(GetValueNormalized() - (0.5 * HANDLE_SIZE.GetX()), 1));
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

	int UISliderComponent::GetMinValueInt() const { return static_cast<int>(m_minMaxValues.m_X); }
	int UISliderComponent::GetMaxValueInt() const { return static_cast<int>(m_minMaxValues.m_Y); }

	float UISliderComponent::GetValue() const { return m_value; };
	float UISliderComponent::GetValueNormalized() const { return (m_value / m_minMaxValues.m_Y) + m_minMaxValues.m_X; }

	void UISliderComponent::InitFields()
	{
		m_Fields = {};
	}
	void UISliderComponent::Serialize(Serialization::Serializer& serializer) const
	{
	}
	void UISliderComponent::Deserialize(Serialization::Deserializer& deserializer)
	{
	}

	std::string UISliderComponent::ToString() const
	{
		return std::format("[UISlider ]");
	}
}
