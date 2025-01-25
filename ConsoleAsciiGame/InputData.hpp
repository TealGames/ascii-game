#pragma once
#include "ComponentData.hpp"
#include "Point2DInt.hpp"

class InputData : public ComponentData
{
private:
	//TODO; input should get abstracted into more of input actions
	Utils::Point2DInt m_LastFrameDirectionalInput;

public:
	InputData();

	const Utils::Point2DInt& GetLastFrameInput() const;
	void SetLastFrameInput(const Utils::Point2DInt& lastFrameInput);
};