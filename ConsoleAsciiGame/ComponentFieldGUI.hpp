#pragma once
#include <functional>
#include <string>
#include "ScreenPosition.hpp"
#include "ComponentField.hpp"
#include "IRenderable.hpp"
#include "InputField.hpp"
#include "InputManager.hpp"
#include "CheckboxGUI.hpp"

class GUISelectorManager;

class ComponentGUI;

class ComponentFieldGUI : IRenderable
{
private:
	const Input::InputManager& m_inputManager;
	ComponentField& m_fieldInfo;
	std::vector<InputField> m_inputFields;
	CheckboxGUI m_checkbox;

	const ComponentGUI* m_componentGUI;

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
	ScreenPosition Render(const RenderInfo& renderInfo) override;

	const ComponentField& GetFieldInfo() const;

	const ComponentGUI& GetComponentGUISafe() const;
};

