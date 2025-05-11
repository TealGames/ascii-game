#pragma once
#include "SelectableGUI.hpp"
#include "IRenderable.hpp"
#include "GUIStyle.hpp"
#include "Color.hpp"

class PopupGUIManager;
namespace Input { class InputManager; }

class ColorPickerGUI :  public SelectableGUI
{
private:
	PopupGUIManager* m_popupManager;
	GUIStyle m_settings;
	Utils::Color m_color;
public:

private:
public:
	ColorPickerGUI(PopupGUIManager& popupManager, const GUIStyle& settings);

	void SetColor(const Utils::Color color);
	void SetSettings(const GUIStyle& settings);

	Utils::Color GetColor() const;

	RenderInfo Render(const RenderInfo& renderInfo) override;
};

