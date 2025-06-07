#pragma once
#include <string>
#include <vector>
#include "ITreeGUIConstructible.hpp"
#include "UILayout.hpp"
#include "UIContainer.hpp"
#include "ComponentData.hpp"
#include "ComponentFieldGUI.hpp"
#include "InputManager.hpp"
#include "UIToggleComponent.hpp"
#include "PanelGUI.hpp"

class EntityGUI;
class GUISelectorManager;
class PopupGUIManager;
namespace AssetManagement { class AssetManager; };
class TextureAsset;

class ComponentGUI : public ITreeGUIConstructible
{
private:
	const Input::InputManager* m_inputManager;
	UIContainer m_guiContainer;
	UILayout m_fieldLayout;
	UIPanel m_nameHeader;
	Component* m_component;
	std::vector<ComponentFieldGUI> m_fieldGUIs;

	const TextureAsset* m_dropdownToggledTexture;
	const TextureAsset* m_dropdownDefaultTexture;

	UIToggleComponent m_dropdownCheckbox;
	UITextComponent m_componentNameText;

	const EntityGUI* m_entityGUI;
public:

private:
	const Input::InputManager& GetInputManager() const;

public:
	ComponentGUI(const Input::InputManager& inputManager, PopupGUIManager& popupManager, 
		const AssetManagement::AssetManager& m_assetManager, const EntityGUI& entityGUI, Component& component);
	~ComponentGUI();
	
	//void Init();
	void Update();
	void SetFieldsToStored();
	//ScreenPosition Render(const RenderInfo& renderInfo) override;
	//ScreenPosition SetupRender(const RenderInfo& renderInfo, Event<void>& renderActions) override;
	GUIElement* GetTreeGUI() override;

	std::string GetComponentName() const;
	std::vector<std::string> GetFieldNames() const;
	const std::vector<ComponentFieldGUI>& GetFields() const;

	const EntityGUI& GetEntityGUISafe() const;
};

