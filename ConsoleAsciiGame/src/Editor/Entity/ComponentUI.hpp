#pragma once
#include <string>
#include <vector>
#include "ECS/Component/Types/UI/UILayout.hpp"
#include "ECS/Component/Component.hpp"
#include "Editor/Entity/ComponentFieldUI.hpp"
#include "Core/Input/InputManager.hpp"
#include "ECS/Component/Types/UI/UIToggleComponent.hpp"
#include "ECS/Component/Types/UI/UIPanel.hpp"

class EntityUI;
class UIInteractionManager;
class PopupUIManager;
namespace AssetManagement { class AssetManager; };
class TextureAsset;

class ComponentUI //: public ITreeGUIConstructible
{
private:
	const Input::InputManager* m_inputManager;
	PopupUIManager* m_popupManager;

	UITransformData* m_container;
	UILayout* m_fieldLayout;
	UIPanel* m_nameHeader;
	Component* m_component;
	std::vector<ComponentFieldUI> m_fieldGUIs;

	const TextureAsset* m_dropdownToggledTexture;
	const TextureAsset* m_dropdownDefaultTexture;

	UIToggleComponent* m_dropdownCheckbox;
	UITextComponent* m_componentNameText;

	const EntityUI* m_entityGUI;
public:

private:
	const Input::InputManager& GetInputManager() const;

public:
	ComponentUI(const Input::InputManager& inputManager, PopupUIManager& popupManager, 
		const AssetManagement::AssetManager& m_assetManager, const EntityUI& entityGUI, UILayout& parent);
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

