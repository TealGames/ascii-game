#pragma once
#include <string>
#include <vector>
#include "ECS/Component/Component.hpp"
#include "Editor/Entity/ComponentFieldUI.hpp"
#include "Core/Input/InputManager.hpp"


class EntityUI;
class UIInteractionManager;
class PopupUIManager;
class UIToggleComponent;
class UITextComponent;
class UILayoutComponent;
class UIPanelComponent;
namespace AssetManagement { class AssetManager; };
class ComponentUI //: public ITreeGUIConstructible
{
private:
	const Input::InputManager* m_inputManager;
	PopupUIManager* m_popupManager;

	UITransformData* m_container;
	UILayoutComponent* m_fieldLayout;
	UIPanelComponent* m_nameHeader;
	Component* m_component;
	std::vector<ComponentFieldUI> m_fieldGUIs;

	UIToggleComponent* m_dropdownCheckbox;
	UITextComponent* m_componentNameText;

	const EntityUI* m_entityGUI;
public:

private:
	const Input::InputManager& GetInputManager() const;

public:
	ComponentUI(const Input::InputManager& inputManager, PopupUIManager& popupManager, 
		AssetManagement::AssetManager& m_assetManager, const EntityUI& entityGUI, UILayoutComponent& parent);
	~ComponentUI();
	
	//void Init();
	void SetComponent(Component& component);
	void Update();
	//ScreenPosition Render(const RenderInfo& renderInfo) override;
	//ScreenPosition SetupRender(const RenderInfo& renderInfo, Event<void>& renderActions) override; 

	std::string GetComponentName() const;
	std::vector<std::string> GetFieldNames() const;
	const std::vector<ComponentFieldUI>& GetFields() const;

	const EntityUI& GetEntityGUISafe() const;
};

