#pragma once
#include "ECS/Component/Component.hpp"
#include "Core/UI/UIStyle.hpp"
#include "Utils/Data/Event.hpp"

namespace Engine::Input { class InputManager; }
namespace Engine::UI
{
	constexpr Vec2 DEFAULT_SLIDER_MIN_MAX = Vec2(0.0f, 1.0f);

	class UIPanelComponent;
	class UIRendererComponent;
	class UITransformComponent;
	class UISelectableComponent;
	class UIInteractionManager;
	class UISliderSystem;
	class UISliderComponent : public ECS::Component
	{
	private:
		const Input::InputManager* m_inputManager;
		UIStyle m_settings;
		Vec2 m_minMaxValues;
		float m_value;

		UIRendererComponent* m_renderer;
		UISelectableComponent* m_selectable;
		UIPanelComponent* m_backgroundPanel;
	public:
		friend class UISliderSystem;
		UITransformComponent* m_SliderHandle;
		Event<void, float> m_OnValueSet;

	private:
		void Init(const Input::InputManager& inputManager);

		/// <summary>
		/// Will return the current value of the slider as a value between 0 and 1
		/// </summary>
		/// <returns></returns>
		float GetValueNormalized() const;
	public:
		UISliderComponent(const Vec2 minMaxValues = DEFAULT_SLIDER_MIN_MAX, const UIStyle& settings = UIStyle());

		void SetSettings(const UIStyle& settings);
		void SetMinValue(const float min);
		void SetMaxValue(const float max);
		void SetValue(const float value);
		void SetValueDelta(const float delta);

		float GetMinValue() const;
		float GetMaxValue() const;
		int GetMinValueInt() const;
		int GetMaxValueInt() const;
		float GetValue() const;

		void InitFields() override;
		void Serialize(Serialization::Serializer& serializer) const override;
		void Deserialize(Serialization::Deserializer& deserializer) override;
		std::string ToString() const override;

		//RenderInfo ElementRender(const RenderInfo& renderInfo) override;
	};
}


