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
class UIRendererData;

using ColorPickerAction = std::function<void(Color)>;
class UIColorPickerData : public Component
{
private:
	PopupUIManager* m_popupManager;
	UIRendererData* m_renderer;
	UIPanel* m_fieldPanel;
	UISelectableData* m_selectable;
	//GUIStyle m_settings;
	Color m_color;

	ColorPickerAction m_valueSetCallback;
public:
	friend class ECS::UIColorPickerSystem;

private:
public:
	UIColorPickerData();
	UIColorPickerData(const UIStyle& settings);

	void Init();

	void SetColor(const Color color);
	//void SetSettings(const GUIStyle& settings);
	void SetValueSetAction(const ColorPickerAction& action);

	Color GetColor() const;
	const UIPanel* GetFieldPanel() const;

	void InitFields() override;
	std::string ToString() const override;

	void Deserialize(const Json& json) override;
	Json Serialize() override;
	//RenderInfo ElementRender(const RenderInfo& renderInfo) override;
};

