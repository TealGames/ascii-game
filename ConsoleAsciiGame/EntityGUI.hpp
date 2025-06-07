#pragma once
#include <vector>	
#include "IRenderable.hpp"
#include "InputManager.hpp"
#include "EntityData.hpp"
#include "ComponentGUI.hpp"
#include "UIToggleComponent.hpp"
#include "ITreeGUIConstructible.hpp"
#include "UILayout.hpp"
#include "PanelGUI.hpp"

class GUIHierarchy;
class PopupGUIManager;
namespace AssetManagement { class AssetManager; };

class EntityGUI : public ITreeGUIConstructible
{
private:
	const Input::InputManager* m_inputManager;
	UILayout m_guiLayout;
	UIPanel m_entityHeader;
	EntityData* m_entity;

	std::vector<ComponentGUI> m_componentGUIs;
	UITextComponent m_entityNameText;
	UIToggleComponent m_activeToggle;
public: 

private:
public:
	EntityGUI(const Input::InputManager& manager, PopupGUIManager& popupManager, 
		const AssetManagement::AssetManager& assetManager, EntityData& entity);
	~EntityGUI();

	void Update();
	void SetComponentsToStored();
	//ScreenPosition Render(const RenderInfo& renderInfo) override;

	GUIElement* GetTreeGUI() override;

	const EntityData& GetEntity() const;
};

