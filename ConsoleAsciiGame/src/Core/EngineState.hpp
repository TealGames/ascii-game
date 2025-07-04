#pragma once
#include <cstdint>

enum class ExecutionState : std::uint8_t
{
	Init			= 0,
	Validation		= 1,
	Update			= 2,
};

class EngineState
{
private:
	ExecutionState m_executionState;
public:

private:
public:
	EngineState();

	void SetExecutionState(ExecutionState newState);
	ExecutionState GetExecutionState() const;
};

