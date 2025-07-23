#include "pch.hpp"
#include "EngineState.hpp"

EngineState::EngineState() : m_executionState(ExecutionState::Init), m_Window(nullptr), m_LastUpdateStatus(UpdateStatusCode::Success) {}

void EngineState::SetExecutionState(ExecutionState newState) { m_executionState = newState; }
ExecutionState EngineState::GetExecutionState() const { return m_executionState; }

