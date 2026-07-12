#include <cstdint>

namespace Engine::Assets { class AssetManager; }
namespace Engine::Input { class InputManager; }
namespace Engine::Core { class Window; }
namespace Engine::Scenes { class SceneManager; }
namespace Engine::Rendering
{
	class GraphicsManager;
	class GraphicsContext
	{
	private:
	public:
		Core::Window* m_Window = nullptr;
		GraphicsManager* m_GraphicsManager = nullptr;
	};
}
namespace Engine::Camera { class CameraController; }
namespace Engine::Core
{
	enum class ExecutionState : std::uint8_t
	{
		Init = 0,
		Validation = 1,
		Update = 2,
	};

	enum class UpdateStatusCode : std::uint8_t
	{
		Success = 0,
		Exit = 1,
		Error = 2,
	};

	class TimeKeeper;
	class EngineState
	{
	private:
		ExecutionState m_executionState;
	public:
		UpdateStatusCode m_LastUpdateStatus;
		Rendering::GraphicsContext m_GraphicsContext;
		Assets::AssetManager* m_AssetManager;
		Camera::CameraController* m_CameraController;
		Input::InputManager* m_InputManager;
		Scenes::SceneManager* m_SceneManager;
		const TimeKeeper* m_TimeKeeper;

	private:
	public:
		EngineState(Rendering::GraphicsManager& graphics, Assets::AssetManager& assets, Camera::CameraController& camera, 
			Input::InputManager& input, Scenes::SceneManager& scenes, const TimeKeeper& time);

		void SetExecutionState(ExecutionState newState);
		ExecutionState GetExecutionState() const;
	};
}

