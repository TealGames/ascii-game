#pragma once
#include <vector>	
#include "IRenderable.hpp"
#include "EntityData.hpp"
#include "ComponentUI.hpp"
#include "UIToggleComponent.hpp"
#include "UILayout.hpp"
#include "UIPanel.hpp"

class UIHierarchy;
class PopupUIManager;
namespace AssetManagement { class AssetManager; };
namespace Input { class InputManager; }

class EntityUI //: public ITreeGUIConstructible
{
private:
	const Input::InputManager* m_inputManager;
	PopupUIManager* m_popupManager;
	const AssetManagement::AssetManager* m_assetManager;

	UILayout* m_guiLayout;
	UIPanel* m_entityHeader;
	EntityData* m_entity;

	std::vector<ComponentUI> m_componentUIs;
	UITextComponent* m_entityNameText;
	UIToggleComponent* m_activeToggle;
public: 

private:
public:
	EntityUI(const Input::InputManager& manager, PopupUIManager& popupManager, 
		const AssetManagement::AssetManager& assetManager, EntityData& parent);
	~EntityUI();

	void Update();
	void SetEntity(EntityData& entity);
	bool HasEntity() const;
	void ClearEntity();
	//ScreenPosition Render(const RenderInfo& renderInfo) override;

	//GUIElement* GetTreeGUI() override;

	const EntityData& GetEntity() const;
};

