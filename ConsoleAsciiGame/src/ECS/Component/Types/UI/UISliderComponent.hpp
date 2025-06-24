#pragma once
#include "ECS/Component/Types/UI/UISelectableData.hpp"
#include "Core/UI/UIStyle.hpp"
#include "Utils/Data/Event.hpp"

class UIPanel;
class UIRenderer;
class UITransformData;
class UIInteractionManager;
namespace Input { class InputManager; }
namespace ECS { class UISliderSystem; }

class UISliderComponent : public UISelectableData
{
private:
	const Input::InputManager* m_inputManager;
	UIStyle m_settings;
	Vec2 m_minMaxValues;
	float m_value;

	UIRendererData* m_renderer;
	UIPanel* m_backgroundPanel;
public:
	friend class ECS::UISliderSystem;
	UITransformData* m_SliderHandle;
	Event<void, float> m_OnValueSet;

private:
	/// <summary>
	/// Will return the current value of the slider as a value between 0 and 1
	/// </summary>
	/// <returns></returns>
	float GetValueNormalized() const;
public:
	UISliderComponent(const Input::InputManager& inputManager, 
		const Vec2 minMaxValues, const UIStyle& settings);

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

	//RenderInfo ElementRender(const RenderInfo& renderInfo) override;
};

