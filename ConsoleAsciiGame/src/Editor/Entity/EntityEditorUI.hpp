#pragma once
#include <optional>
#include "EntityUI.hpp"
#include "InputManager.hpp"

class UIHierarchy;
class CameraData;
class CameraController;
class PopupUIManager;
namespace AssetManagement { class AssetManager; };

//using EntityGUICollection = std::unordered_map<std::string, EntityGUI>;
class EntityEditorUI
{
private:
	const Input::InputManager* m_inputManager;
	const AssetManagement::AssetManager* m_assetManager;
	PopupUIManager* m_popupManager;
	UIHierarchy* m_guiTree;

	/*EntityGUICollection m_entityGUIs;
	EntityGUICollection::iterator m_selectedEntity;*/
	EntityUI m_selectedEntityUI;

public:

private:
	const Input::InputManager& GetInputManagerSafe() const;

public:
	EntityEditorUI(const Input::InputManager& input, const CameraController& cameraController, 
		UIHierarchy& hierarchy, PopupUIManager& popupManager, const AssetManagement::AssetManager& assetManager, EntityData& parent);
	~EntityEditorUI();

	void SetEntityUI(EntityData& entity);
	bool HasEntitySelected() const;
	void CloseCurrentEntityGUI();

	void Update();
	//void TryRender();
	//RenderInfo Render(const RenderInfo& renderInfo) override;
};

