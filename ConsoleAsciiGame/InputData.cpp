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
	m_lastFrameDirectionalInput = m_currentFrameDirectionalInput;
	m_currentFrameDirectionalInput = input;
}

Utils::Point2DInt InputData::GetInputDelta() const
{
	return m_currentFrameDirectionalInput - m_lastFrameDirectionalInput;
}