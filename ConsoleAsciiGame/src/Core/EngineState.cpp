#include "pch.hpp"
#include "EngineState.hpp"

EngineState::EngineState() : m_executionState(ExecutionState::Init) {}

void EngineState::SetExecutionState(ExecutionState newState) { m_executionState = newState; }
ExecutionState EngineState::GetExecutionState() const { return m_executionState; }

