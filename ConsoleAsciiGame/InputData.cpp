//NOT USED
#include "pch.hpp"
#include "InputData.hpp"	

InputData::InputData() : ComponentData(), 
	m_currentFrameDirectionalInput(), m_lastFrameDirectionalInput() {}

const Utils::Point2DInt& InputData::GetFrameInput() const
{
	return m_currentFrameDirectionalInput;
}

const Utils::Point2DInt& InputData::GetLastFrameInput() const
{
	return m_lastFrameDirectionalInput;
}

bool InputData::HasInputChanged() const
{
	return m_currentFrameDirectionalInput != m_lastFrameDirectionalInput;
}

void InputData::SetFrameInput(const Utils::Point2DInt& input)
{
	static bool updatingFrame = false;
	if (updatingFrame) return;

	updatingFrame = true;
	m_lastFrameDirectionalInput = m_currentFrameDirectionalInput;
	m_currentFrameDirectionalInput = input;
	updatingFrame = false;
}

Utils::Point2DInt InputData::GetInputDelta() const
{
	return m_currentFrameDirectionalInput - m_lastFrameDirectionalInput;
}