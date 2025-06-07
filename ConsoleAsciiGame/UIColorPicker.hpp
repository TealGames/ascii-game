#pragma once
#include "UISelectableData.hpp"
#include "IRenderable.hpp"
#include "GUIStyle.hpp"
#include "Color.hpp"
#include <functional>

class PopupGUIManager;
namespace Input { class InputManager; }

class UIPanel;
using ColorPickerAction = std::function<void(Utils::Color)>;
class UIColorPickerData :  public UISelectableData
{
private:
	PopupGUIManager* m_popupManager;
	UIRendererData* m_renderer;
	UIPanel* m_fieldPanel;
	//GUIStyle m_settings;
	Utils::Color m_color;

	ColorPickerAction m_valueSetCallback;
public:

private:
public:
	UIColorPickerData();
	UIColorPickerData(const GUIStyle& settings);

	void SetColor(const Utils::Color color);
	//void SetSettings(const GUIStyle& settings);
	void SetValueSetAction(const ColorPickerAction& action);

	Utils::Color GetColor() const;
	const UIPanel* GetFieldPanel() const;

	void InitFields() override;
	std::string ToString() const override;

	void Deserialize(const Json& json) override;
	Json Serialize() override;
	//RenderInfo ElementRender(const RenderInfo& renderInfo) override;
};

