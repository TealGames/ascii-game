#include "pch.hpp"
#include "EngineState.hpp"

EngineState::EngineState() : m_executionState(ExecutionState::Init), m_GraphicsContext{}, 
m_LastUpdateStatus(UpdateStatusCode::Success), m_CameraController() {}

void EngineState::SetExecutionState(ExecutionState newState) { m_executionState = newState; }
ExecutionState EngineState::GetExecutionState() const { return m_executionState; }