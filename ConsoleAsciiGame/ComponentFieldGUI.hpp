#pragma once
#include <functional>
#include <string>
#include "ScreenPosition.hpp"
#include "ComponentField.hpp"
#include "IDelayedRenderable.hpp"
#include "InputField.hpp"
#include "InputManager.hpp"
#include "ToggleGUI.hpp"

class GUISelectorManager;

class ComponentGUI;

class ComponentFieldGUI : IDelayedRenderable
{
private:
	const Input::InputManager& m_inputManager;
	ComponentField& m_fieldInfo;
	std::vector<InputField> m_inputFields;
	ToggleGUI m_checkbox;

	const ComponentGUI* m_componentGUI;
	TextGUI m_fieldNameText;
public:

private:
public:
	ComponentFieldGUI(const Input::InputManager& inputManager, GUISelectorManager& selector, 
		const ComponentGUI& componentGUI, ComponentField& field);
	ComponentFieldGUI(const ComponentFieldGUI&) = default;
	~ComponentFieldGUI();

	void Update();
	void SetFieldToInternal();
	void SetInternalWithInput();
	//ScreenPosition Render(const RenderInfo& renderInfo) override;
	ScreenPosition SetupRender(const RenderInfo& renderInfo, Event<void>& renderActions) override;

	const ComponentField& GetFieldInfo() const;

	const ComponentGUI& GetComponentGUISafe() const;
};

