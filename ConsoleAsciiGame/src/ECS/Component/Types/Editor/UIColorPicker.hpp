#pragma once
#include "ECS/Component/Component.hpp"
#include "Core/UI/UIStyle.hpp"
#include "Utils/Data/Color.hpp"
#include <functional>

class PopupUIManager;
namespace Input { class InputManager; }
namespace ECS { class UIColorPickerSystem; }

class UIPanel;
class UISelectableData;
using ColorPickerAction = std::function<void(Utils::Color)>;
class UIColorPickerData :  public Component
{
private:
	PopupUIManager* m_popupManager;
	UIRendererData* m_renderer;
	UIPanel* m_fieldPanel;
	UISelectableData* m_selectable;
	//GUIStyle m_settings;
	Utils::Color m_color;

	ColorPickerAction m_valueSetCallback;
public:
	friend class ECS::UIColorPickerSystem;

private:
public:
	UIColorPickerData();
	UIColorPickerData(const UIStyle& settings);

	void Init();

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

