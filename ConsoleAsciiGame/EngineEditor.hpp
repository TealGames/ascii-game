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
}
class GUISelectorManager;
class Scene;
class CameraData;
class TimeKeeper;

class EngineEditor : public IBasicRenderable
{
private:
	TimeKeeper& m_timeKeeper;
	const Input::InputManager& m_inputManager;
	SceneManagement::SceneManager& m_sceneManager;

	CommandConsole m_commandConsole;
	EntityEditorGUI m_entityEditor;
	DebugInfo m_debugInfo;

	ToggleGUI m_pauseGameToggle;
public:
	static constexpr KeyboardKey PAUSE_TOGGLE_KEY = KEY_P;

private:
	void InitConsoleCommands(ECS::PlayerSystem& playerSystem);
	void TogglePause();

public:
	EngineEditor(TimeKeeper& time, const Input::InputManager& input, Physics::PhysicsManager& physics,
		SceneManagement::SceneManager& scene, GUISelectorManager& selector);
	~EngineEditor();

	void Init(ECS::PlayerSystem& playerSystem);
	void Update(const float& deltaTime, const float& timeStep, 
		const Scene& activeScene, CameraData& mainCamera);

	bool TryRender() override;
};

