#include "pch.hpp"
#include "EngineState.hpp"

namespace Engine::Core
{
	EngineState::EngineState(Rendering::GraphicsManager& graphics, Assets::AssetManager& assets, Camera::CameraController& camera,
		Input::InputManager& input, Scenes::SceneManager& scenes, const TimeKeeper& time) : 
		m_executionState(ExecutionState::Init), m_GraphicsContext{ nullptr, &graphics }, m_LastUpdateStatus(UpdateStatusCode::Success),
		m_CameraController(&camera), m_TimeKeeper(&time), m_AssetManager(&assets), m_InputManager(&input), m_SceneManager(&scenes) {}

	void EngineState::SetExecutionState(ExecutionState newState) { m_executionState = newState; }
	ExecutionState EngineState::GetExecutionState() const { return m_executionState; }
}
