#pragma once
#include "ComponentData.hpp"
#include "Point2DInt.hpp"

class InputData : public ComponentData
{
private:
	//TODO; input should get abstracted into more of input actions
	Utils::Point2DInt m_currentFrameDirectionalInput;
	Utils::Point2DInt m_lastFrameDirectionalInput;

public:
	InputData();

	const Utils::Point2DInt& GetFrameInput() const;
	const Utils::Point2DInt& GetLastFrameInput() const;
	Utils::Point2DInt GetInputDelta() const;
	bool HasInputChanged() const;

	void SetFrameInput(const Utils::Point2DInt& lastFrameInput);
};