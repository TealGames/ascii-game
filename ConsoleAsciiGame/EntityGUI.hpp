#pragma once
#include <vector>	
#include "IRenderable.hpp"
#include "InputManager.hpp"
#include "Entity.hpp"
#include "ComponentGUI.hpp"
#include "ToggleGUI.hpp"
#include "ITreeGUIConstructible.hpp"
#include "LayoutGUI.hpp"
#include "PanelGUI.hpp"

class GUIHierarchy;
class PopupGUIManager;

class EntityGUI : public ITreeGUIConstructible
{
private:
	const Input::InputManager* m_inputManager;
	LayoutGUI m_guiLayout;
	PanelGUI m_entityHeader;
	ECS::Entity* m_entity;

	std::vector<ComponentGUI> m_componentGUIs;
	TextGUI m_entityNameText;
	ToggleGUI m_activeToggle;
public: 

private:
public:
	EntityGUI(const Input::InputManager& manager, PopupGUIManager& popupManager, ECS::Entity& entity);
	~EntityGUI();

	void Update();
	void SetComponentsToStored();
	//ScreenPosition Render(const RenderInfo& renderInfo) override;

	GUIElement* GetTreeGUI() override;

	const ECS::Entity& GetEntity() const;
};

