#pragma once
#include <cstdint>

enum class ExecutionState : std::uint8_t
{
	Init			= 0,
	Validation		= 1,
	Update			= 2,
};

enum class UpdateStatusCode : std::uint8_t
{
	Success		= 0,
	Exit		= 1,
	Error		= 2,
};

class Window;
class EngineState
{
private:
	ExecutionState m_executionState;

public:
	UpdateStatusCode m_LastUpdateStatus;
	Window* m_Window;

private:
public:
	EngineState();

	void SetExecutionState(ExecutionState newState);
	ExecutionState GetExecutionState() const;
};

