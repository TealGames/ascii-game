#pragma once
#include <optional>
#include "EntityGUI.hpp"
#include "InputManager.hpp"

class GUIHierarchy;
class CameraData;
class CameraController;
class PopupGUIManager;
namespace AssetManagement { class AssetManager; };

//using EntityGUICollection = std::unordered_map<std::string, EntityGUI>;
class EntityEditorGUI
{
private:
	const Input::InputManager* m_inputManager;
	const AssetManagement::AssetManager* m_assetManager;
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
		GUIHierarchy& hierarchy, PopupGUIManager& popupManager, const AssetManagement::AssetManager& assetManager);
	~EntityEditorGUI();

	void SetEntityGUI(ECS::Entity& entity);
	bool HasEntitySelected() const;
	bool TryCloseCurrentEntityGUI();

	void Update();
	//void TryRender();
	//RenderInfo Render(const RenderInfo& renderInfo) override;
};

