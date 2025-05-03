#pragma once
#include <string>
#include <vector>
#include "IRenderable.hpp"
#include "ComponentData.hpp"
#include "ComponentFieldGUI.hpp"
#include "InputManager.hpp"
#include "ToggleGUI.hpp"

class EntityGUI;
class GUISelectorManager;
class PopupGUIManager;

class ComponentGUI : public IRenderable
{
private:
	const Input::InputManager* m_inputManager;
	ComponentData* m_component;
	std::vector<ComponentFieldGUI> m_fieldGUIs;

	ToggleGUI m_dropdownCheckbox;
	TextGUI m_componentNameText;

	const EntityGUI* m_entityGUI;
public:

private:
	const Input::InputManager& GetInputManager() const;

public:
	ComponentGUI(const Input::InputManager& inputManager, GUISelectorManager& selector,
		PopupGUIManager& popupManager, const EntityGUI& entityGUI, ComponentData& component);
	~ComponentGUI();
	
	void Init();
	void Update();
	void SetFieldsToStored();
	ScreenPosition Render(const RenderInfo& renderInfo) override;
	//ScreenPosition SetupRender(const RenderInfo& renderInfo, Event<void>& renderActions) override;

	std::string GetComponentName() const;
	std::vector<std::string> GetFieldNames() const;
	const std::vector<ComponentFieldGUI>& GetFields() const;

	const EntityGUI& GetEntityGUISafe() const;
};

