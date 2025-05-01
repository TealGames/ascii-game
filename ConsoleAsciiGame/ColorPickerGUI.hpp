#pragma once
#include "SelectableGUI.hpp"
#include "IRenderable.hpp"
#include "GUISettings.hpp"
#include "Color.hpp"

class GUISelectorManager;
class PopupGUIManager;
class ColorPickerGUI :  public SelectableGUI, public IRenderable
{
private:
	PopupGUIManager* m_popupManager;
	GUISettings m_settings;
	Utils::Color m_color;
public:

private:
public:
	ColorPickerGUI(GUISelectorManager& guiSelector, PopupGUIManager& popupManager, const GUISettings& settings);

	void SetColor(const Utils::Color color);
	void SetSettings(const GUISettings& settings);

	Utils::Color GetColor() const;

	void Update();
	ScreenPosition Render(const RenderInfo& renderInfo) override;
};

