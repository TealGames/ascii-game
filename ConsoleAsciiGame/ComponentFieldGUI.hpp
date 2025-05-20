#pragma once
#include <functional>
#include <string>
#include "ScreenPosition.hpp"
#include "ComponentField.hpp"
#include "IDelayedRenderable.hpp"
#include "ITreeGUIConstructible.hpp"
#include "InputFieldGUI.hpp"
#include "ContainerGUI.hpp"
#include "InputManager.hpp"
#include "ToggleGUI.hpp"
#include "ColorPickerGUI.hpp"

class GUISelectorManager;
class ComponentGUI;
class PopupGUIManager;

class ComponentFieldGUI : public ITreeGUIConstructible
{
private:
	const Input::InputManager* m_inputManager;
	ContainerGUI m_guiLayout;
	ComponentField* m_fieldInfo;
	std::vector<InputFieldGUI> m_inputFields;
	ToggleGUI m_checkbox;
	ColorPickerGUI m_colorPicker;

	const ComponentGUI* m_componentGUI;
	TextGUI m_fieldNameText;
public:

private:
	const Input::InputManager& GetInputManager() const;
	ComponentField& GetFieldInfo();

public:
	ComponentFieldGUI(const Input::InputManager& inputManager, PopupGUIManager& popupManager,
		const ComponentGUI& componentGUI, ComponentField& field);
	ComponentFieldGUI(const ComponentFieldGUI&) = default;
	~ComponentFieldGUI();

	//void Init();
	void Update();
	void SetFieldToInternal();
	void SetInternalWithInput();
	//ScreenPosition Render(const RenderInfo& renderInfo) override;
	//ScreenPosition SetupRender(const RenderInfo& renderInfo, Event<void>& renderActions) override;
	GUIElement* GetTreeGUI() override;

	const ComponentField& GetFieldInfo() const;

	const ComponentGUI& GetComponentGUISafe() const;
};

