#pragma once
#include "CommandConsole.hpp"
#include "EntityEditorUI.hpp"
#include "SpriteEditorUI.hpp"
#include "PopupUIManager.hpp"
#include "IBasicRenderable.hpp"
#include "DebugInfo.hpp"
#include "UIToggleComponent.hpp"
#include "UIButton.hpp"
#include "raylib.h"
#include "UIHierarchy.hpp"
#include "UIPanel.hpp"

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
class UIInteractionManager;
class PopupUIManager;

struct EditModeInfo
{
	EntityData* m_Selected;

	EditModeInfo();
};

class EngineEditor //: public IBasicRenderable
{
private:
	UITransformData* m_editorRoot;

	TimeKeeper& m_timeKeeper;
	const Input::InputManager& m_inputManager;
	SceneManagement::SceneManager& m_sceneManager;
	Physics::PhysicsManager& m_physicsManager;
	const CameraController& m_cameraController;
	UIInteractionManager& m_guiSelector;
	ECS::CollisionBoxSystem& m_collisionBoxSystem;
	UIHierarchy& m_guiTree;
	PopupUIManager& m_popupManager;

	CommandConsole m_commandConsole;
	DebugInfo m_debugInfo;

	EntityEditorUI m_entityEditor;
	SpriteEditorUI m_spriteEditor;

	UITextComponent* m_mousePosText;
	UIPanel* m_overheadBarContainer;
	UILayout* m_toggleLayout;
	UIToggleComponent* m_pauseGameToggle;
	UIToggleComponent* m_editModeToggle;
	UIButton* m_assetEditorButton;
	EditModeInfo m_editModeInfo;

	bool m_displayingGameView;

private:
	void InitConsoleCommands(ECS::PlayerSystem& playerSystem);
	void SelectEntityEditor(EntityData& entity);

public:
	EngineEditor(TimeKeeper& time, const Input::InputManager& input, Physics::PhysicsManager& physics, AssetManagement::AssetManager& assetManager,
		SceneManagement::SceneManager& scene, const CameraController& camera, UIInteractionManager& selector, UIHierarchy& guiTree, 
		PopupUIManager& popupManager, ECS::CollisionBoxSystem& collisionSystem);
	~EngineEditor();

	void Init(ECS::PlayerSystem& playerSystem);
	void Update(const float unscaledDeltaTime, const float scaledDeltaTime, const float timeStep);

	bool IsInGameView() const;

	//bool TryRender();
};

