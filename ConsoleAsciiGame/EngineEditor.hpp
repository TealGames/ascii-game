#pragma once
#include "CommandConsole.hpp"
#include "EntityEditorGUI.hpp"
#include "SpriteEditorGUI.hpp"
#include "PopupGUIManager.hpp"
#include "IBasicRenderable.hpp"
#include "DebugInfo.hpp"
#include "ToggleGUI.hpp"
#include "ButtonGUI.hpp"
#include "raylib.h"

namespace Core { class Engine; }
namespace Input { class InputManager; }
namespace SceneManagement { class SceneManager; }
namespace Physics { class PhysicsManager; }
namespace AssetManagement { class AssetManager; }
namespace ECS
{
	class PlayerSystem;
	class CollisionBoxSystem;
	class Entity;
}
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
	GUISelectorManager& m_guiSelector;
	ECS::CollisionBoxSystem& m_collisionBoxSystem;

	CommandConsole m_commandConsole;
	DebugInfo m_debugInfo;
	PopupGUIManager m_popupManager;

	EntityEditorGUI m_entityEditor;
	SpriteEditorGUI m_spriteEditor;

	ToggleGUI m_pauseGameToggle;
	ToggleGUI m_editModeToggle;
	ButtonGUI m_assetEditorButton;
	EditModeInfo m_editModeInfo;

	bool m_displayingGameView;

private:
	void InitConsoleCommands(ECS::PlayerSystem& playerSystem);
	void SelectEntityEditor(ECS::Entity& entity);

public:
	EngineEditor(TimeKeeper& time, const Input::InputManager& input, Physics::PhysicsManager& physics, AssetManagement::AssetManager& assetManager,
		SceneManagement::SceneManager& scene, const CameraController& camera, GUISelectorManager& selector, ECS::CollisionBoxSystem& collisionSystem);
	~EngineEditor();

	void Init(ECS::PlayerSystem& playerSystem);
	void Update(const float deltaTime, const float timeStep);

	bool IsInGameView() const;

	bool TryRender() override;
};

