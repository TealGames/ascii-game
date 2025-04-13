#pragma once
#include "CommandConsole.hpp"
#include "EntityEditorGUI.hpp"
#include "IBasicRenderable.hpp"
#include "DebugInfo.hpp"
#include "ToggleGUI.hpp"
#include "raylib.h"

namespace Core
{
	class Engine;
}
namespace Input
{
	class InputManager;
}
namespace SceneManagement
{
	class SceneManager;
}
namespace Physics
{
	class PhysicsManager;
}
namespace ECS
{
	class PlayerSystem;
	class CollisionBoxSystem;
	class Entity;
}
class GUISelectorManager;
class Scene;
class CameraData;
class TimeKeeper;
class CameraController;
class GUISelectorManager;

struct EditModeInfo
{
	ECS::Entity* m_Selected;

	EditModeInfo();
};

class EngineEditor : public IBasicRenderable
{
private:
	TimeKeeper& m_timeKeeper;
	const Input::InputManager& m_inputManager;
	SceneManagement::SceneManager& m_sceneManager;
	Physics::PhysicsManager& m_physicsManager;
	const CameraController& m_cameraController;
	const GUISelectorManager& m_guiSelector;
	ECS::CollisionBoxSystem& m_collisionBoxSystem;

	CommandConsole m_commandConsole;
	EntityEditorGUI m_entityEditor;
	DebugInfo m_debugInfo;

	ToggleGUI m_pauseGameToggle;
	ToggleGUI m_editModeToggle;
	EditModeInfo m_editModeInfo;

private:
	void InitConsoleCommands(ECS::PlayerSystem& playerSystem);

public:
	EngineEditor(TimeKeeper& time, const Input::InputManager& input, Physics::PhysicsManager& physics,
		SceneManagement::SceneManager& scene, const CameraController& camera, GUISelectorManager& selector, ECS::CollisionBoxSystem& collisionSystem);
	~EngineEditor();

	void Init(ECS::PlayerSystem& playerSystem);
	void Update(const float& deltaTime, const float& timeStep, 
		Scene& activeScene, CameraData& mainCamera);

	bool TryRender() override;
};

