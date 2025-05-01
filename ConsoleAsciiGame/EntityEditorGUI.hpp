#pragma once
#include <optional>
#include "EntityGUI.hpp"
#include "InputManager.hpp"

class GUISelectorManager;
class CameraData;
class CameraController;
class PopupGUIManager;

//using EntityGUICollection = std::unordered_map<std::string, EntityGUI>;
class EntityEditorGUI : IRenderable
{
private:
	const Input::InputManager* m_inputManager;
	PopupGUIManager* m_popupManager;
	GUISelectorManager* m_selectorManager;

	RenderInfo m_defaultRenderInfo;
	/*EntityGUICollection m_entityGUIs;
	EntityGUICollection::iterator m_selectedEntity;*/
	EntityGUI* m_selectedEntity;

public:
	static const Color EDITOR_TEXT_COLOR;
	static const Color EDITOR_BACKGROUND_COLOR;
	static const Color EDITOR_SECONDARY_BACKGROUND_COLOR;
	static const Color EDITOR_SECONDARY_COLOR;
	static const Color EDITOR_PRIMARY_COLOR;

	static const Vec2 EDITOR_CHAR_SPACING;

private:
	const Input::InputManager& GetInputManagerSafe() const;
	GUISelectorManager& GetGUISelector();

public:
	EntityEditorGUI(const Input::InputManager& input, const CameraController& cameraController, 
		GUISelectorManager& selector, PopupGUIManager& popupManager);
	~EntityEditorGUI();

	void SetEntityGUI(ECS::Entity& entity);

	void Update();
	void TryRender();
	ScreenPosition Render(const RenderInfo& renderInfo) override;
};

