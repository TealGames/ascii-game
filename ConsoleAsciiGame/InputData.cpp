//NOT USED
#include "pch.hpp"
#include "InputData.hpp"	

InputData::InputData() : ComponentData(), m_LastFrameDirectionalInput() {}

const Utils::Point2DInt& InputData::GetLastFrameInput() const
{
	return m_LastFrameDirectionalInput;
}

void InputData::SetLastFrameInput(const Utils::Point2DInt& lastFrameInput)
{
	m_LastFrameDirectionalInput = lastFrameInput;
}