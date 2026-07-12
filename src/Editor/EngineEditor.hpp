#pragma once
#include "Editor/Console/CommandConsoleUI.hpp"
#include "Editor/Entity/EntityEditorUI.hpp"
#include "Editor/Sprite/SpriteEditorUI.hpp"
#include "Core/Analyzation/DebugInfo.hpp"
#include "Core/UI/UIHierarchy.hpp"

namespace Engine::Core { class Engine; class TimeKeeper; }
namespace Engine::Input { class InputManager; }
namespace Engine::Scenes { class Scene;  class SceneManager; }
namespace Engine::Physics { class PhysicsManager; class CollisionBoxSystem; }
namespace Engine::Camera { class CameraComponent; class CameraController; }
namespace Engine::Assets { class AssetManager; }
namespace Engine::ECS { class EntityData; }
namespace Engine::Player { class PlayerSystem; }
namespace Engine::UI
{
	class UIPanelComponent;
	class UIToggleComponent;
	class UITextComponent;
	class UIButtonComponent;
	class UILayoutComponent;
	class UIInteractionManager;
	class PopupUIManager;
}

namespace Engine::Editor
{
	namespace MainUI = Engine::UI;
	namespace MainDebug = Engine::Debug;

	struct EditModeInfo
	{
		ECS::EntityData* m_Selected;
		EditModeInfo();
	};

	namespace Debug { class GizmoOverlay; }
	class EngineEditor //: public IBasicRenderable
	{
	private:
		MainUI::UITransformComponent* m_editorRoot;

		Core::TimeKeeper& m_timeKeeper;
		const Input::InputManager& m_inputManager;
		Scenes::SceneManager& m_sceneManager;
		Physics::PhysicsManager& m_physicsManager;
		Camera::CameraController& m_cameraController;
		MainUI::UIInteractionManager& m_guiSelector;
		Physics::CollisionBoxSystem& m_collisionBoxSystem;
		MainUI::UIHierarchy& m_guiTree;
		MainUI::PopupUIManager& m_popupManager;
		Debug::GizmoOverlay& m_gizmos;

		MainDebug::CommandController& m_commandController;
		UI::CommandConsoleUI m_commandConsole;
		Debug::DebugInfo m_debugInfo;
		const Input::InputProfile* m_inputProfile;

		UI::EntityEditorUI m_entityEditor;
		UI::SpriteEditorUI m_spriteEditor;

		MainUI::UITextComponent* m_mousePosText;
		MainUI::UIPanelComponent* m_overheadBarContainer;
		MainUI::UILayoutComponent* m_toggleLayout;
		MainUI::UIToggleComponent* m_pauseGameToggle;
		MainUI::UIToggleComponent* m_editModeToggle;
		MainUI::UIButtonComponent* m_assetEditorButton;
		EditModeInfo m_editModeInfo;

		bool m_displayingGameView;
		bool m_inCameraFreemode;

		float m_freelookYaw;
		float m_freelookPitch;

	private:
		void InitConsoleCommands(Player::PlayerSystem& playerSystem);
		void SelectEntityEditor(ECS::EntityData& entity);

	public:
		EngineEditor(Core::TimeKeeper& time, const Input::InputManager& input, Physics::PhysicsManager& physics, Assets::AssetManager& assetManager,
			Scenes::SceneManager& scene, Camera::CameraController& camera, MainUI::UIInteractionManager& selector, MainUI::UIHierarchy& guiTree,
			MainUI::PopupUIManager& popupManager, Physics::CollisionBoxSystem& collisionSystem, MainDebug::CommandController& commands, Debug::GizmoOverlay& gizmos);
		~EngineEditor();

		void Init(Player::PlayerSystem& playerSystem);
		void Update(const float unscaledDeltaTime, const float scaledDeltaTime, const float timeStep);

		bool IsInGameView() const;
	};
}
