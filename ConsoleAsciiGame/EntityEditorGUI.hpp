#pragma once
#include <optional>
#include "EntityGUI.hpp"
#include "InputManager.hpp"

class GUIHierarchy;
class CameraData;
class CameraController;
class PopupGUIManager;

//using EntityGUICollection = std::unordered_map<std::string, EntityGUI>;
class EntityEditorGUI
{
private:
	const Input::InputManager* m_inputManager;
	PopupGUIManager* m_popupManager;
	GUIHierarchy* m_guiTree;

	RenderInfo m_defaultRenderInfo;
	/*EntityGUICollection m_entityGUIs;
	EntityGUICollection::iterator m_selectedEntity;*/
	EntityGUI* m_selectedEntity;

public:

private:
	const Input::InputManager& GetInputManagerSafe() const;

public:
	EntityEditorGUI(const Input::InputManager& input, const CameraController& cameraController, 
		GUIHierarchy& hierarchy, PopupGUIManager& popupManager);
	~EntityEditorGUI();

	void SetEntityGUI(ECS::Entity& entity);

	void Update();
	//void TryRender();
	//RenderInfo Render(const RenderInfo& renderInfo) override;
};

