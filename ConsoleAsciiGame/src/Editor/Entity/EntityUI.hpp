#pragma once
#include <vector>	
#include "ECS/Component/Types/World/EntityData.hpp"
#include "Editor/Entity/ComponentUI.hpp"
#include "ECS/Component/Types/UI/UIToggleComponent.hpp"
#include "ECS/Component/Types/UI/UILayout.hpp"
#include "ECS/Component/Types/UI/UIPanel.hpp"

class UIHierarchy;
class PopupUIManager;
namespace AssetManagement { class AssetManager; };
namespace Input { class InputManager; }

class EntityUI //: public ITreeGUIConstructible
{
private:
	const Input::InputManager* m_inputManager;
	PopupUIManager* m_popupManager;
	AssetManagement::AssetManager* m_assetManager;

	EntityData* m_layoutParent;
	UILayout* m_guiLayout;
	UIPanel* m_entityHeader;
	EntityData* m_entity;

	std::vector<ComponentUI> m_componentUIs;
	UITextComponent* m_entityNameText;
	UIToggleComponent* m_activeToggle;
public: 

private:
	void CreateLayout();
public:
	EntityUI(const Input::InputManager& manager, PopupUIManager& popupManager, 
		AssetManagement::AssetManager& assetManager);
	~EntityUI();

	void Init(EntityData& parent);

	void Update();
	void SetEntity(EntityData& entity);
	bool HasEntity() const;
	void ClearEntity();
	//ScreenPosition Render(const RenderInfo& renderInfo) override;

	//GUIElement* GetTreeGUI() override;

	const EntityData& GetEntity() const;
};

