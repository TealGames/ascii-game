#pragma once
#include <unordered_map>
#include <string>
#include "EntityGUI.hpp"
#include "InputManager.hpp"
#include "SceneManager.hpp"
#include "PhysicsManager.hpp"

class GUISelectorManager;
class CameraData;

using EntityGUICollection = std::unordered_map<std::string, EntityGUI>;
class EntityEditorGUI : IRenderable
{
private:
	const Input::InputManager& m_inputManager;
	Physics::PhysicsManager& m_physicsManager;
	const SceneManagement::SceneManager& m_sceneManager;
	GUISelectorManager& m_selectorManager;

	RenderInfo m_defaultRenderInfo;
	EntityGUICollection m_entityGUIs;
	EntityGUICollection::iterator m_selectedEntity;

public:
	static const Color EDITOR_TEXT_COLOR;
	static const Color EDITOR_BACKGROUND_COLOR;
	static const Color EDITOR_SECONDARY_COLOR;

	static const Vec2 EDITOR_CHAR_SPACING;

private:
public:
	EntityEditorGUI(const Input::InputManager& input, const SceneManagement::SceneManager& scene, 
		Physics::PhysicsManager& manager, GUISelectorManager& selector);

	void SetEntityGUI(ECS::Entity& entity);

	void Update(CameraData& mainCamera);
	void TryRender();
	ScreenPosition Render(const RenderInfo& renderInfo) override;
};

