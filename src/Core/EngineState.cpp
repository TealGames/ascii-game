#include "pch.hpp"
#include "EngineState.hpp"

EngineState::EngineState() : m_executionState(ExecutionState::Init), m_GraphicsContext{}, m_LastUpdateStatus(UpdateStatusCode::Success) {}

void EngineState::SetExecutionState(ExecutionState newState) { m_executionState = newState; }
ExecutionState EngineState::GetExecutionState() const { return m_executionState; }

void EngineState::SetRenderingContext(const Rendering::GraphicsContext& context)
{
	m_GraphicsContext = context;
}