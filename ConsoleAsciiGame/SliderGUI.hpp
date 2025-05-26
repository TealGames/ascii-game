#pragma once
#include "SelectableGUI.hpp"
#include "IRenderable.hpp"
#include "GUIStyle.hpp"
#include "Event.hpp"

class GUISelectorManager;
namespace Input { class InputManager; }
class SliderGUI : public SelectableGUI
{
private:
	const Input::InputManager* m_inputManager;
	GUIStyle m_settings;
	Vec2 m_minMaxValues;
	float m_value;
public:
	Event<void, float> m_OnValueSet;

private:
	/// <summary>
	/// Will return the current value of the slider as a value between 0 and 1
	/// </summary>
	/// <returns></returns>
	float GetValueNormalized() const;
public:
	SliderGUI(const Input::InputManager& inputManager, 
		const Vec2 minMaxValues, const GUIStyle& settings);

	void SetSettings(const GUIStyle& settings);
	void SetMinValue(const float min);
	void SetMaxValue(const float max);
	void SetValue(const float value);
	void SetValueDelta(const float delta);

	float GetMinValue() const;
	float GetMaxValue() const;
	int GetMinValueInt() const;
	int GetMaxValueInt() const;
	float GetValue() const;

	RenderInfo ElementRender(const RenderInfo& renderInfo) override;
};

