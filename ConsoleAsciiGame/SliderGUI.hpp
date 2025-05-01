#pragma once
#include "SelectableGUI.hpp"
#include "IRenderable.hpp"
#include "GUISettings.hpp"
#include "Event.hpp"

class GUISelectorManager;
namespace Input { class InputManager; }
class SliderGUI : public SelectableGUI, public IRenderable
{
private:
	const Input::InputManager* m_inputManager;
	GUISettings m_settings;
	Vec2 m_minMaxValues;
	float m_value;
	
public:
	Event<void, float> m_OnValueSet;

private:
public:
	SliderGUI(GUISelectorManager& selector, const Input::InputManager& inputManager, 
		const Vec2 minMaxValues, const GUISettings& settings);

	void SetSettings(const GUISettings& settings);
	void SetMinValue(const float min);
	void SetMaxValue(const float max);
	void SetValue(const float value);

	float GetMinValue() const;
	float GetMaxValue() const;
	int GetMinValueInt() const;
	int GetMaxValueInt() const;

	void Update();
	ScreenPosition Render(const RenderInfo& renderInfo) override;
};

