#pragma once
#include "SelectableGUI.hpp"
#include "IRenderable.hpp"
#include "GUIStyle.hpp"
#include "Color.hpp"
#include <functional>

class PopupGUIManager;
namespace Input { class InputManager; }

using ColorPickerAction = std::function<void(Utils::Color)>;
class ColorPickerGUI :  public SelectableGUI
{
private:
	PopupGUIManager* m_popupManager;
	//GUIStyle m_settings;
	Utils::Color m_color;

	ColorPickerAction m_valueSetCallback;
public:

private:
public:
	ColorPickerGUI(PopupGUIManager& popupManager, const GUIStyle& settings);

	void SetColor(const Utils::Color color);
	//void SetSettings(const GUIStyle& settings);
	void SetValueSetAction(const ColorPickerAction& action);

	Utils::Color GetColor() const;

	RenderInfo Render(const RenderInfo& renderInfo) override;
};

