#pragma once
#include <string>
#include <vector>
#include "IRenderable.hpp"
#include "ComponentData.hpp"
#include "ComponentFieldGUI.hpp"
#include "InputManager.hpp"

class EntityGUI;
class GUISelectorManager;

class ComponentGUI : IRenderable
{
private:
	const Input::InputManager* m_inputManager;
	ComponentData* m_component;
	std::vector<ComponentFieldGUI> m_fieldGUIs;

	const EntityGUI* m_entityGUI;
public:

private:
	const Input::InputManager& GetInputManager() const;

public:
	ComponentGUI(const Input::InputManager& inputManager, GUISelectorManager& selector,
		const EntityGUI& entityGUI, ComponentData* component);
	~ComponentGUI();
	
	void Update();
	void SetFieldsToStored();
	ScreenPosition Render(const RenderInfo& renderInfo) override;

	std::string GetComponentName() const;
	std::vector<std::string> GetFieldNames() const;
	const std::vector<ComponentFieldGUI>& GetFields() const;

	const EntityGUI& GetEntityGUISafe() const;
};

